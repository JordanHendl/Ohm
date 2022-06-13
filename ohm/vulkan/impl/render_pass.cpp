#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "render_pass.h"
#include <array>
#include <iostream>
#include <vector>
#include <utility>
#include <vulkan/vulkan.hpp>
#include "ohm/api/render_pass.h"
#include "ohm/vulkan/impl/device.h"
#include "ohm/vulkan/impl/error.h"
#include "ohm/vulkan/impl/image.h"
#include "ohm/vulkan/impl/swapchain.h"
#include "ohm/vulkan/impl/system.h"

namespace ohm {
namespace ovk {
inline auto convert(const Attachment& attachment, bool depth)
    -> vk::AttachmentDescription {
  using StoreOps = vk::AttachmentStoreOp;
  using LoadOps = vk::AttachmentLoadOp;

  const auto format = convert(attachment.info.format);
  const auto layout = vk::ImageLayout::eColorAttachmentOptimal;
  const auto stencil_store = depth ? StoreOps::eStore : StoreOps::eDontCare;
  const auto stencil_load = depth ? LoadOps::eLoad : LoadOps::eDontCare;
  const auto load_op = LoadOps::eClear;  /// TODO make configurable
  const auto store_op = StoreOps::eStore;

  auto desc = vk::AttachmentDescription();
  desc.setSamples(vk::SampleCountFlagBits::e1);
  desc.setLoadOp(load_op);
  desc.setStoreOp(store_op);
  desc.setFormat(format);
  desc.setInitialLayout(vk::ImageLayout::eUndefined);
  desc.setStencilLoadOp(stencil_load);
  desc.setStencilStoreOp(stencil_store);
  desc.setFinalLayout(layout);
  return desc;
}

RenderPass::RenderPass() {
  this->m_current_framebuffer = 0;
  this->m_num_binded_subpasses = 0;
  this->m_area.extent.setWidth(1280);
  this->m_area.extent.setHeight(1024);
}

RenderPass::RenderPass(Device& device, const RenderPassInfo& info) {
  this->m_device = &device;
  this->m_area.extent.width = info.width;
  this->m_area.extent.height = info.height;

  this->parse_info(info);
  this->make_render_pass();
  this->make_images();
}

RenderPass::RenderPass(RenderPass&& mv) { *this = std::move(mv); }

RenderPass::~RenderPass() {
  for (auto& framebuffer : this->m_framebuffers) {
    this->m_device->device().destroy(framebuffer,
                                     this->m_device->allocationCB(),
                                     this->m_device->dispatch());
  }

  if (this->m_pass) {
    this->m_device->device().destroy(this->m_pass,
                                     this->m_device->allocationCB(),
                                     this->m_device->dispatch());
    this->m_pass = nullptr;
  }

  this->m_images.clear();
  this->m_framebuffers.clear();
  this->m_attachments.clear();
  this->m_subpasses.clear();
  this->m_references.clear();
  this->m_dependencies.clear();
}

auto RenderPass::operator=(RenderPass&& mv) -> RenderPass& {
  this->m_clear_colors = std::move(mv.m_clear_colors);
  this->m_framebuffers = std::move(mv.m_framebuffers);
  this->m_dependencies = std::move(mv.m_dependencies);
  this->m_references = std::move(mv.m_references);
  this->m_color_references = std::move(mv.m_color_references);
  this->m_depth_references = std::move(mv.m_depth_references);
  this->m_subpasses = std::move(mv.m_subpasses);
  this->m_images = std::move(mv.m_images);
  this->m_attachments = std::move(mv.m_attachments);
  this->m_device = mv.m_device;
  this->m_pass = mv.m_pass;
  this->m_area = mv.m_area;
  this->m_surface = mv.m_surface;
  this->m_current_framebuffer = mv.m_current_framebuffer;
  this->m_num_binded_subpasses = mv.m_num_binded_subpasses;

  mv.m_clear_colors.clear();
  mv.m_framebuffers.clear();
  mv.m_dependencies.clear();
  mv.m_references.clear();
  mv.m_color_references.clear();
  mv.m_depth_references.clear();
  mv.m_subpasses.clear();
  mv.m_images.clear();
  mv.m_attachments.clear();
  mv.m_device = nullptr;
  mv.m_pass = nullptr;
  mv.m_area = vk::Rect2D();
  mv.m_surface = nullptr;
  mv.m_current_framebuffer = 0;
  mv.m_num_binded_subpasses = 0;
  return *this;
}
auto RenderPass::make_images() -> void {
  static constexpr auto NUM_BUFFERS = 3u;

  auto info = ImageInfo();
  auto format = vk::Format();
  auto layout = vk::ImageLayout();

  this->m_images.reserve(NUM_BUFFERS * this->m_attachments.size());
  this->m_framebuffers.resize(NUM_BUFFERS);
  for (auto attach = 0u; attach < NUM_BUFFERS; attach++) {
    for (auto index = 0u; index < this->m_attachments.size(); index++) {
      auto usage_flag = vk::ImageUsageFlags();
      format = this->m_attachments[index].format;
      info.width = this->m_area.extent.width;
      info.height = this->m_area.extent.height;
      info.format = convert(format);
      layout = vk::ImageLayout::eColorAttachmentOptimal;
      if (this->m_attachments[index].format == vk::Format::eD24UnormS8Uint) {
        usage_flag = vk::ImageUsageFlagBits::eDepthStencilAttachment |
                     vk::ImageUsageFlagBits::eSampled;
        info.format = ImageFormat::Depth;
        layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
      }

      auto global_index = 0u;
      for (auto& img : ovk::system().image) {
        if (!img.initialized()) {
          img.setUsage(usage_flag);
          img = std::move(ovk::Image(*this->device(), info, layout));
          this->m_images.push_back(global_index);
          break;
        }
        global_index++;
      }
    }
  }
}

auto RenderPass::add_subpass(const Subpass& subpass) -> void {
  const auto src_subpass =
      this->m_subpasses.empty() ? 0 : this->m_subpasses.size();

  auto attach_desc = vk::AttachmentDescription();
  auto attach_ref = vk::AttachmentReference();
  auto subpass_desc = vk::SubpassDescription();
  auto subpass_dep = vk::SubpassDependency();
  auto clear = vk::ClearValue();
  auto color = vk::ClearColorValue();

  subpass_desc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

  for (auto index = 0u; index < subpass.attachments.size(); index++) {
    auto& attachment = subpass.attachments[index];

    attach_desc = convert(attachment, subpass.enable_depth);

    color.float32[0] = attachment.clear_color[0];
    color.float32[1] = attachment.clear_color[1];
    color.float32[2] = attachment.clear_color[2];
    color.float32[3] = attachment.clear_color[3];
    clear.setColor(color);

    attach_ref.setAttachment(this->m_attachments.size());
    attach_ref.setLayout(attach_desc.finalLayout);

    this->m_clear_colors.push_back(clear);
    this->m_attachments.push_back(attach_desc);
    this->m_references.push_back(attach_ref);
    this->m_color_references.push_back(attach_ref);
    subpass_desc.colorAttachmentCount++;
  }

  // There can be only one depth per subpass, so append one if its enabled.
  if (subpass.enable_depth) {
    clear.depthStencil.depth = subpass.depth_clear;
    clear.depthStencil.stencil = 0;
    attach_desc.setStoreOp(vk::AttachmentStoreOp::eStore);
    attach_desc.setLoadOp(vk::AttachmentLoadOp::eClear);
    attach_desc.setSamples(vk::SampleCountFlagBits::e1);
    attach_desc.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    attach_desc.setFormat(vk::Format::eD24UnormS8Uint);
    attach_desc.setInitialLayout(vk::ImageLayout::eUndefined);
    attach_desc.setStencilLoadOp(vk::AttachmentLoadOp::eClear);
    attach_desc.setStencilStoreOp(vk::AttachmentStoreOp::eStore);

    attach_ref.setAttachment(this->m_attachments.size());
    attach_ref.setLayout(attach_desc.finalLayout);

    this->m_clear_colors.push_back(clear);
    this->m_attachments.push_back(attach_desc);
    this->m_references.push_back(attach_ref);
    this->m_depth_references.push_back(attach_ref);
  }

  if (subpass_desc.pColorAttachments == nullptr) {
    subpass_desc.setPColorAttachments(this->m_color_references.data() +
                                      this->m_color_references.size() -
                                      subpass_desc.colorAttachmentCount);
  }

  if (subpass_desc.pDepthStencilAttachment == nullptr) {
    subpass_desc.setPDepthStencilAttachment(this->m_depth_references.data());
  }

  // TODO Add attachment references here for referencing other attachments.
  for (size_t index = 0; index < subpass.dependancies.size(); index++) {
    subpass_dep.setSrcSubpass(src_subpass);
    if (subpass.dependancies[index] < this->m_dependencies.size()) {
      subpass_dep.setDstSubpass(subpass.dependancies[index]);
    }

    subpass_dep.setSrcStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput |
        vk::PipelineStageFlagBits::eEarlyFragmentTests);
    subpass_dep.setDstStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput |
        vk::PipelineStageFlagBits::eEarlyFragmentTests);
    subpass_dep.setDstAccessMask(
        vk::AccessFlagBits::eColorAttachmentWrite |
        vk::AccessFlagBits::eDepthStencilAttachmentWrite);
  }

  if (!subpass.attachments.empty()) this->m_subpasses.push_back(subpass_desc);
  if (!subpass.dependancies.empty())
    this->m_dependencies.push_back(subpass_dep);
}

auto RenderPass::parse_info(const RenderPassInfo& info) -> void {
  for (auto& subpass : info.subpasses) {
    this->add_subpass(subpass);
  }
}

auto RenderPass::make_render_pass() -> void {
  auto info = vk::RenderPassCreateInfo();

  auto device = this->m_device->device();
  auto& dispatch = this->m_device->dispatch();
  auto* alloc_cb = this->m_device->allocationCB();

  info.setAttachments(this->m_attachments);
  info.setDependencies(this->m_dependencies);
  info.setSubpasses(this->m_subpasses);

  this->m_pass = error(device.createRenderPass(info, alloc_cb, dispatch));
}

auto RenderPass::bind() -> void {
  static constexpr auto NUM_BUFFERS = 3u;
  auto total_framebuffers = 0u;
  auto info = vk::FramebufferCreateInfo();
  auto views = std::vector<vk::ImageView>();

  auto device = this->m_device->device();
  auto& dispatch = this->m_device->dispatch();
  auto* alloc_cb = this->m_device->allocationCB();

  info.setWidth(this->m_area.extent.width);
  info.setHeight(this->m_area.extent.height);

  for (auto attach = 0u; attach < NUM_BUFFERS; attach++) {
    views.clear();

    for (auto& subpass : this->m_subpasses) {
      auto num_framebuffers = subpass.inputAttachmentCount +
                              subpass.colorAttachmentCount +
                              subpass.preserveAttachmentCount;
      if (subpass.pDepthStencilAttachment) num_framebuffers++;
      auto tmp = total_framebuffers;
      for (unsigned index = total_framebuffers; index < tmp + num_framebuffers;
           index++) {
        auto img_index = this->m_images[total_framebuffers++];
        auto& img = ovk::system().image[img_index];
        views.push_back(img.view());
      }

      info.setAttachmentCount(views.size());
      info.setPAttachments(views.data());
      info.setLayers(1);
      info.setRenderPass(this->m_pass);

      this->m_framebuffers[attach] =
          error(device.createFramebuffer(info, alloc_cb, dispatch));
    }
  }
}

auto RenderPass::offset(size_t subpass) const -> size_t {
  auto offset = 0u;
  OhmAssert(subpass > this->m_subpasses.size(), "Wrong number of subpasses.");
  for (auto index = 0u; index < subpass; index++) {
    auto& ref = this->m_subpasses[index];
    offset += ref.inputAttachmentCount + ref.colorAttachmentCount +
              ref.preserveAttachmentCount;
  }

  return offset;
}
}  // namespace ovk
}  // namespace ohm
