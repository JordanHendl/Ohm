#pragma once
#include <memory>
#include "memory.h"

namespace ohm {

enum class ImageFormat : int {
  R8,       ///< Single channel Char.
  R32U,     ///< Single channel Integer.
  R32I,     ///< Single channel Integer.
  R32F,     ///< Single channel Float.
  RG32F,    ///< Two channel Float.
  RGB8,     ///< Three channel Char.
  BGR8,     ///< Three channel Char.
  RGB32U,   ///< Three channel Integer.
  RGB32I,   ///< Three channel Integer.
  RGB32F,   ///< Three channel Float.
  RGBA8,    ///< Four channel Char.
  BGRA8,    ///< Three channel Char.
  RGBA32I,  ///< Four channel Integer.
  RGBA32U,  ///< Four channel Unsigned Integer.
  RGBA32F,  ///< Four channel Float.
  Depth,    ///< Depth.
};

struct ImageInfo {
  size_t width;
  size_t height;
  size_t layers;
  size_t mip_maps;
  ImageFormat format;
  bool is_cubemap;

  ImageInfo() {
    this->width = 512;
    this->height = 512;
    this->layers = 1;
    this->mip_maps = 1;
    this->format = ImageFormat::RGBA8;
    this->is_cubemap = false;
  }

  ImageInfo(size_t width, size_t height, ImageFormat fmt = ImageFormat::RGBA8) {
    this->width = width;
    this->height = height;
    this->layers = 1;
    this->mip_maps = 1;
    this->format = fmt;
    this->is_cubemap = false;
  }

  auto count() const -> size_t {
    return this->width * this->height * this->layers;
  }

  auto operator=(const ImageInfo& cp) -> ImageInfo& {
    this->format = cp.format;
    this->width = cp.width;
    this->height = cp.height;
    this->layers = cp.layers;
    this->is_cubemap = cp.is_cubemap;
    this->mip_maps = cp.mip_maps;

    return *this;
  }
};

template <typename API>
class Window;

template <typename API, typename Allocator>
class RenderPass;

template <typename API, typename Allocator = DefaultAllocator<API>>
class Image {
 public:
  Image();
  Image(int gpu, ImageInfo info);
  Image(Image&& mv);
  ~Image();
  auto operator=(Image&& mv) -> Image&;
  auto info() const -> ImageInfo;
  auto width() const -> size_t;
  auto height() const -> size_t;
  auto layers() const -> size_t;
  auto gpu() const -> int;
  auto handle() const -> int32_t;
  auto layer(size_t index) -> Image<API, Allocator>;
  auto memory() -> const Memory<API, Allocator>&;
  auto format() -> ImageFormat;

 private:
  friend class Window<API>;
  friend class RenderPass<API, Allocator>;

  int32_t m_handle;
  ImageInfo m_info;
  std::shared_ptr<Memory<API, Allocator>> m_memory;
};

template <typename API, typename Allocator>
Image<API, Allocator>::Image() {
  this->m_handle = -1;
  this->m_memory = std::make_shared<Memory<API, Allocator>>();
}

template <typename API, typename Allocator>
Image<API, Allocator>::Image(int gpu, ImageInfo info) {
  this->m_handle = API::Image::create(gpu, info);
  this->m_info = info;
  auto required_size = API::Image::required(this->m_handle);
  this->m_memory = std::make_shared<Memory<API, Allocator>>(gpu, required_size);
  API::Image::bind(this->m_handle, m_memory->handle());
}

template <typename API, typename Allocator>
Image<API, Allocator>::Image(Image&& mv) {
  *this = std::move(mv);
}

template <typename API, typename Allocator>
Image<API, Allocator>::~Image() {
  if (this->m_handle >= 0) {
    API::Image::destroy(this->m_handle);
    this->m_handle = -1;
    this->m_info = ImageInfo();
  }
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::operator=(Image<API, Allocator>&& mv)
    -> Image<API, Allocator>& {
  this->m_memory = std::move(mv.m_memory);
  this->m_info = mv.m_info;
  this->m_handle = mv.m_handle;

  mv.m_handle = -1;
  mv.m_info = ImageInfo();

  return *this;
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::info() const -> ImageInfo {
  return this->m_info;
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::gpu() const -> int {
  return this->m_memory->gpu();
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::width() const -> size_t {
  return this->m_info.width;
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::height() const -> size_t {
  return this->m_info.height;
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::layers() const -> size_t {
  return this->m_info.layers;
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::handle() const -> int32_t {
  return this->m_handle;
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::layer(size_t index) -> Image<API, Allocator> {
  auto layer_handle = API::Image::layer(this->m_handle, index);
  auto tmp = Image<API, Allocator>();
  tmp.m_handle = layer_handle;
  tmp.m_info = this->m_info;
  tmp.m_memory = this->m_memory;
  return tmp;
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::memory() -> const Memory<API, Allocator>& {
  return *this->m_memory;
}

template <typename API, typename Allocator>
auto Image<API, Allocator>::format() -> ImageFormat {
  return this->m_info.format;
}
}  // namespace ohm

/** Required functions of Image API
 * Image::create(gpu, image_info) -> handle
 * Image::destroy(handle) -> void
 * Image::layer(handle) -> handle
 * Image::required(handle) -> size_t
 */