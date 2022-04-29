#pragma once
#include <array>
#include <vector>
#include "allocators.h"
#include "image.h"
namespace ohm {
struct Attachment {
  ImageInfo info;
  std::array<float, 4> clear_color;

  Attachment() {
    this->info = {};
    this->clear_color = {0, 0, 0, 0};
  }
};

struct Subpass {
  std::vector<Attachment> attachments;
  std::vector<size_t> dependancies;
  float depth_clear;
  bool enable_depth;

  Subpass() {
    this->depth_clear = 0.0f;
    this->enable_depth = false;
  }

  Subpass(Attachment info, bool depth = false) {
    this->attachments.push_back(info);
    this->depth_clear = 0.0f;
    this->enable_depth = depth;
  }
};
struct RenderPassInfo {
  size_t width;
  size_t height;
  std::vector<Subpass> subpasses;

  RenderPassInfo() {
    this->width = 1280;
    this->height = 1024;
  }

  RenderPassInfo(size_t width, size_t height) {
    this->width = width;
    this->height = height;
  }
};

// RenderPasses are triple-buffered 'render descriptions'. Describes a rendering
// operation.
// Note* All render passes are triple buffered & segmented by subpass & layer.
// Example layout:
// [ subpass 0 framebuffer 0 layer 0, subpass 0 framebuffer 1 layer 0, 
//   subpass 1, framebuffer 0, layer 0, subpass 0, framebuffer 0, layer 1 ... ]
template <typename API, typename Allocator = DefaultAllocator<API>>
class RenderPass {
 public:
  RenderPass();
  explicit RenderPass(int gpu, RenderPassInfo info);
  explicit RenderPass(int gpu, const std::vector<Attachment>& attachments);
  explicit RenderPass(RenderPass&& mv);
  ~RenderPass();
  auto operator=(RenderPass&& mv) -> RenderPass&;
  auto gpu() const -> int;
  auto handle() const -> int32_t;
  auto image(size_t layer = 0, size_t framebuffer = 0, size_t subpass = 0) -> Image<API>&;
  auto image(size_t layer = 0, size_t framebuffer = 0, size_t subpass = 0) const
      -> const Image<API>&;
  auto images() const -> const std::vector<Image<API>>&;

 private:
  RenderPassInfo m_info;
  int m_gpu;
  std::vector<Image<API>> m_framebuffers;
  int32_t m_handle;
};

template <typename API, typename Allocator>
RenderPass<API, Allocator>::RenderPass() {
  this->m_handle = -1;
  this->m_gpu = -1;
}

template <typename API, typename Allocator>
RenderPass<API, Allocator>::RenderPass(int gpu, RenderPassInfo info) {
  this->m_gpu = gpu;
  this->m_handle = API::RenderPass::create(gpu, info);
  this->m_framebuffers.resize(API::RenderPass::count(this->m_handle));

  auto index = 0u;
  for (auto& img : this->m_framebuffers) {
    img.m_handle = API::RenderPass::image(this->m_handle, index++);
    auto required_size = API::Image::required(img.handle());
    img.m_memory = std::make_shared<Memory<API, Allocator>>(gpu, required_size);
    API::Image::bind(img.m_handle, img.m_memory->handle());
  }
}

template <typename API, typename Allocator>
RenderPass<API, Allocator>::RenderPass(int gpu, const std::vector<Attachment>& attachments) {
  auto tmp_subpass = Subpass();
  tmp_subpass.attachments = attachments;
  auto info = RenderPassInfo();
  info.subpasses.push_back(tmp_subpass);
  
  this->m_gpu = gpu;
  this->m_handle = API::RenderPass::create(gpu, info);
  this->m_framebuffers.resize(API::RenderPass::count(this->m_handle));

  auto index = 0u;
  for (auto& img : this->m_framebuffers) {
    img.m_handle = API::RenderPass::image(this->m_handle, index++);
    auto required_size = API::Image::required(img.handle());
    img.m_memory = std::make_shared<Memory<API, Allocator>>(gpu, required_size);
    API::Image::bind(img.m_handle, img.m_memory->handle());
  }
}

template <typename API, typename Allocator>
RenderPass<API, Allocator>::RenderPass(RenderPass&& mv) {
  *this = std::move(mv);
}

template <typename API, typename Allocator>
RenderPass<API, Allocator>::~RenderPass() {
  if (this->m_handle >= 0) {
    API::RenderPass::destroy(this->m_handle);
    this->m_handle = -1;
    this->m_gpu = -1;
  }
}

template <typename API, typename Allocator>
auto RenderPass<API, Allocator>::operator=(RenderPass&& mv) -> RenderPass& {
  this->m_handle = mv.m_handle;
  this->m_info = mv.m_info;
  this->m_gpu = mv.m_gpu;
  this->m_framebuffers = std::move(mv.m_framebuffers);

  mv.m_gpu = -1;
  mv.m_handle = -1;
  mv.m_info = {};
}

template <typename API, typename Allocator>
auto RenderPass<API, Allocator>::gpu() const -> int {
  return this->m_gpu;
}

template <typename API, typename Allocator>
auto RenderPass<API, Allocator>::handle() const -> int32_t {
  return this->m_handle;
}

template <typename API, typename Allocator>
auto RenderPass<API, Allocator>::image(size_t layer, size_t framebuffer, size_t subpass)
    -> Image<API>& {
  // Count the amount of framebuffers until the target subpass.
  auto sp_index = 0;
  auto fb_count = 0;
  for (auto& m_subpass : this->m_info.subpasses) {
    sp_index++;
    if (sp_index != subpass) {
      for (auto& attachment : m_subpass.attachments) {
        fb_count++;
      }
    }
  }

  const auto final_index = (fb_count + framebuffer) * layer;
  OhmAssert(this->m_info.subpasses[subpass].attachments.size() > framebuffer,
               "Asking for an image from a subpass that is out of bounds.");
  OhmAssert(final_index >= this->m_framebuffers.size(),
               "Asking for an image from a subpass that is out of bounds.");
  return this->m_framebuffers[final_index];
}

template <typename API, typename Allocator>
auto RenderPass<API, Allocator>::image(size_t layer, size_t framebuffer, size_t subpass) const
    -> const Image<API>& {
  return this->image(layer, framebuffer, subpass);
}

template <typename API, typename Allocator>
auto RenderPass<API, Allocator>::images() const
    -> const std::vector<Image<API>>& {
  return this->m_framebuffers;
}
}  // namespace ohm