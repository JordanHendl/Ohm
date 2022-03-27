#pragma once
#include <string>
#include <utility>
#include "descriptor.h"
#include "ohm/io/shader.h"
#include "render_pass.h"
namespace ohm {
enum class Topology : int {
  Point,
  Line,
  LineStrip,
  Triangle,
  TriangleStrip,
};

struct Viewport {
  size_t width;
  size_t height;
  size_t x_pos;
  size_t y_pos;
  size_t max_depth;

  Viewport() {
    this->width = 1280;
    this->height = 1024;
    this->x_pos = 0;
    this->y_pos = 0;
    this->max_depth = 1.f;
  }
};

struct PipelineInfo {
  std::string file_name;
  std::vector<Viewport> viewports;
  bool stencil_test;
  bool depth_test;
  Topology topology;

  // name : shader contents.
  std::vector<std::pair<std::string, std::string>> inline_files;

  PipelineInfo() {
    this->topology = Topology::Triangle;
    this->depth_test = false;
    this->stencil_test = false;
  }

  PipelineInfo(std::string_view file) {
    this->file_name = file;
    this->topology = Topology::Triangle;
    this->depth_test = false;
    this->stencil_test = false;
  }

  PipelineInfo(std::vector<std::pair<std::string, std::string>> files) {
    this->inline_files = files;
    this->topology = Topology::Triangle;
    this->depth_test = false;
    this->stencil_test = false;
  }

  PipelineInfo(std::string_view file, Viewport viewport) {
    this->file_name = file;
    this->viewports.push_back(viewport);
    this->topology = Topology::Triangle;
    this->depth_test = false;
    this->stencil_test = false;
  }
};

template <typename API, typename Allocator>
class RenderPass;

template <typename API>
class Pipeline {
 public:
  Pipeline();
  explicit Pipeline(int gpu, PipelineInfo info);
  template <typename Allocator>
  explicit Pipeline(const RenderPass<API, Allocator>& rp,
                    const PipelineInfo& info);
  explicit Pipeline(Pipeline&& mv);
  ~Pipeline();
  auto operator=(Pipeline&& mv) -> Pipeline&;
  auto gpu() const -> int;
  auto descriptor() const -> Descriptor<API>;
  auto handle() const -> int32_t;

 private:
  PipelineInfo m_info;
  int m_gpu;
  int32_t m_rp_handle;
  int32_t m_handle;
};

template <typename API>
Pipeline<API>::Pipeline() {
  this->m_rp_handle = -1;
  this->m_handle = -1;
  this->m_gpu = -1;
}

template <typename API>
Pipeline<API>::Pipeline(int gpu, PipelineInfo info) {
  this->m_rp_handle = -1;
  this->m_gpu = gpu;
  this->m_info = info;
  this->m_handle = API::Pipeline::create(gpu, info);
}

template <typename API>
template <typename Allocator>
Pipeline<API>::Pipeline(const RenderPass<API, Allocator>& rp,
                        const PipelineInfo& info) {
  this->m_rp_handle = rp.handle();
  this->m_gpu = rp.gpu();
  this->m_info = info;
  this->m_handle = API::Pipeline::create_from_rp(rp.handle(), info);
}

template <typename API>
Pipeline<API>::Pipeline(Pipeline&& mv) {
  *this = std::move(mv);
}

template <typename API>
Pipeline<API>::~Pipeline() {
  if (this->m_handle >= 0) {
    API::Pipeline::destroy(this->m_handle);
    this->m_rp_handle = -1;
    this->m_handle = -1;
    this->m_gpu = -1;
  }
}

template <typename API>
auto Pipeline<API>::operator=(Pipeline&& mv) -> Pipeline& {
  this->m_gpu = mv.m_gpu;
  this->m_handle = mv.m_handle;
  this->m_info = mv.m_info;
  this->m_rp_handle = mv.m_rp_handle;

  mv.m_rp_handle = -1;
  mv.m_handle = -1;
  mv.m_gpu = -1;
  mv.m_info = {};
  return *this;
}

template <typename API>
auto Pipeline<API>::gpu() const -> int {
  return this->m_gpu;
}

template <typename API>
auto Pipeline<API>::descriptor() const -> Descriptor<API> {
  auto handle = API::Pipeline::descriptor(this->m_handle);
  return Descriptor<API>(handle, this);
}

template <typename API>
auto Pipeline<API>::handle() const -> int32_t {
  return this->m_handle;
}
}  // namespace ohm