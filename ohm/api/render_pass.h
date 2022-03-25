#pragma once
#include <vector>
#include "allocators.h"
#include "image.h"

namespace ohm {
struct Subpass {
  std::vector<ImageInfo> attachments;
  std::vector<size_t> dependancies;
  float depth_clear;
  bool enable_depth;

  Subpass() {
    this->depth_clear = 0.0f;
    this->enable_depth = false;
  }

  Subpass(ImageInfo info, bool depth = false) {
    this->attachments.push_back(info);
    this->depth_clear = 0.0f;
    this->enable_depth = depth;
  }
};
struct RenderPassInfo {
  std::vector<Subpass> subpasses;
};

template <typename API, typename Allocator = DefaultAllocator<API>>
class RenderPass {
 public:
  RenderPass();
  explicit RenderPass(int gpu, RenderPassInfo info);
  explicit RenderPass(RenderPass&& mv);
  ~RenderPass();
  auto operator=(RenderPass&& mv) -> RenderPass&;
  auto gpu() const -> int;
  auto handle() const -> int32_t;
  auto image(size_t framebuffer = 0, size_t subpass = 0) -> Image<API>&;
  auto image(size_t framebuffer = 0, size_t subpass = 0) const
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
auto RenderPass<API, Allocator>::image(size_t framebuffer, size_t subpass)
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

  const auto final_index = fb_count + framebuffer;
  OhmException(this->m_info.subpasses[subpass].attachments.size() > framebuffer,
               Error::LogicError,
               "Asking for an image from a subpass that is out of bounds.");
  return this->m_framebuffers[fb_count + framebuffer];
}

template <typename API, typename Allocator>
auto RenderPass<API, Allocator>::image(size_t framebuffer, size_t subpass) const
    -> const Image<API>& {
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

  const auto final_index = fb_count + framebuffer;
  OhmException(this->m_info.subpasses[subpass].attachments.size() > framebuffer,
               Error::LogicError,
               "Asking for an image from a subpass that is out of bounds.");
  return this->m_framebuffers[fb_count + framebuffer];
}

template <typename API, typename Allocator>
auto RenderPass<API, Allocator>::images() const
    -> const std::vector<Image<API>>& {
  return this->m_framebuffers;
}
}  // namespace ohm