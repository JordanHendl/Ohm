#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>
#include "ohm/vulkan/impl/device.h"
#include "ohm/vulkan/impl/image.h"
#include "ohm/vulkan/impl/swapchain.h"

namespace ohm {
namespace ovk {
class Device;
class CommandBuffer;
class Image;

class RenderPass {
 public:
  RenderPass();
  RenderPass(Device& device, const RenderPassInfo& info);
  RenderPass(RenderPass&& mv);
  ~RenderPass();
  auto operator=(RenderPass&& mv) -> RenderPass&;
  auto bind() -> void;
  auto offset(size_t subpass) const -> size_t;
  inline auto initialized() const -> bool { return this->m_pass; }
  inline auto count() const -> size_t { return this->m_subpasses.size(); }
  inline auto device() -> Device* { return this->m_device; }
  inline auto area() const -> const vk::Rect2D& { return this->m_area; }
  inline auto subpasses() const -> const std::vector<vk::SubpassDescription>& {
    return this->m_subpasses;
  }
  inline auto framebuffers() const -> const std::vector<int32_t>& {
    return this->m_images;
  }
  inline auto framebuffers() -> std::vector<int32_t>& { return this->m_images; }
  inline auto vk_framebuffers() const -> const std::vector<vk::Framebuffer>& {
    return this->m_framebuffers;
  }
  inline auto clear_values() const -> const std::vector<vk::ClearValue>& {
    return this->m_clear_colors;
  }
  inline auto pass() const -> const vk::RenderPass& { return this->m_pass; }
  inline auto current() const -> size_t { return this->m_current_framebuffer; }
  inline auto setCurrent(size_t val) -> void {
    this->m_current_framebuffer = val;
  }

 private:
  using Framebuffers = std::vector<vk::Framebuffer>;
  using Images = std::vector<int32_t>;
  using Attachments = std::vector<vk::AttachmentDescription>;
  using References = std::vector<vk::AttachmentReference>;
  using Dependencies = std::vector<vk::SubpassDependency>;
  using Subpasses = std::vector<vk::SubpassDescription>;
  using ClearColors = std::vector<vk::ClearValue>;

  ClearColors m_clear_colors;
  Framebuffers m_framebuffers;
  Dependencies m_dependencies;
  References m_references;
  References m_color_references;
  References m_depth_references;
  Subpasses m_subpasses;
  Images m_images;
  Attachments m_attachments;
  Device* m_device;
  vk::RenderPass m_pass;
  vk::Rect2D m_area;
  vk::SurfaceKHR m_surface;
  size_t m_current_framebuffer;
  size_t m_num_binded_subpasses;

  auto add_subpass(const Subpass& subpass) -> void;
  auto parse_info(const RenderPassInfo& info) -> void;
  auto make_render_pass() -> void;
  auto make_images() -> void;
};
}  // namespace ovk
}  // namespace ohm
