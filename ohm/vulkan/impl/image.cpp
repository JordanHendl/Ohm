#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "image.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <vulkan/vulkan.hpp>
#include "error.h"
#include "ohm/api/exception.h"

namespace ohm {
namespace ovk {
auto convert(ImageFormat format) -> vk::Format {
  switch (format) {
    case ImageFormat::RGBA32F:
      return vk::Format::eR32G32B32A32Sfloat;
    case ImageFormat::RGB32F:
      return vk::Format::eR32G32B32Sfloat;
    case ImageFormat::RG32F:
      return vk::Format::eR32G32Sfloat;
    case ImageFormat::R32F:
      return vk::Format::eR32Sfloat;
    case ImageFormat::BGRA8:
      return vk::Format::eB8G8R8A8Srgb;
    case ImageFormat::BGR8:
      return vk::Format::eB8G8R8Srgb;
    case ImageFormat::R8:
      return vk::Format::eR8Srgb;
    case ImageFormat::Depth:
      return vk::Format::eD24UnormS8Uint;
    default:
      return vk::Format::eR8G8B8A8Srgb;
  }
}

auto convert(vk::Format format) -> ImageFormat {
  switch (format) {
    case vk::Format::eR32G32B32A32Sfloat:
      return ImageFormat::RGBA32F;
    case vk::Format::eR32G32B32Sfloat:
      return ImageFormat::RGB32F;
    case vk::Format::eR32G32Sfloat:
      return ImageFormat::RG32F;
    case vk::Format::eR32Sfloat:
      return ImageFormat::R32F;
    case vk::Format::eD24UnormS8Uint:
      return ImageFormat::Depth;
    case vk::Format::eB8G8R8A8Srgb:
      return ImageFormat::BGRA8;
    case vk::Format::eR8Srgb:
      return ImageFormat::R8;
    case vk::Format::eB8G8R8Srgb:
      return ImageFormat::BGR8;
    default:
      return ImageFormat::RGBA8;
  }
}

auto Image::createView() -> vk::ImageView {
  vk::ImageViewCreateInfo info;
  vk::ImageSubresourceRange range;

  if (this->m_info.format == ImageFormat::Depth) {
    range.setAspectMask(vk::ImageAspectFlagBits::eDepth |
                        vk::ImageAspectFlagBits::eStencil);
  } else {
    range.setAspectMask(vk::ImageAspectFlagBits::eColor);
  }

  range.setBaseArrayLayer(0);
  range.setBaseMipLevel(0);
  range.setLayerCount(this->m_info.layers);
  range.setLevelCount(1);

  info.setImage(this->m_image);
  info.setViewType(this->m_view_type);  //@JH TODO Make configurable.
  info.setFormat(this->format());
  info.setSubresourceRange(range);

  return error(this->m_device->device().createImageView(
      info, nullptr, this->m_device->dispatch()));
}

auto Image::createSampler() -> vk::Sampler {
  const auto max_anisotropy = 16.0f;

  vk::SamplerCreateInfo info;

  info.setMagFilter(vk::Filter::eNearest);
  info.setMinFilter(vk::Filter::eNearest);
  info.setAddressModeU(vk::SamplerAddressMode::eRepeat);
  info.setAddressModeV(vk::SamplerAddressMode::eRepeat);
  info.setAddressModeW(vk::SamplerAddressMode::eRepeat);
  info.setBorderColor(vk::BorderColor::eIntTransparentBlack);
  info.setCompareOp(vk::CompareOp::eNever);
  info.setMipmapMode(vk::SamplerMipmapMode::eLinear);
  info.setAnisotropyEnable(vk::Bool32(false));
  info.setUnnormalizedCoordinates(vk::Bool32(false));
  info.setCompareEnable(vk::Bool32(false));
  info.setMaxAnisotropy(max_anisotropy);
  info.setMipLodBias(0.0f);
  info.setMinLod(0.0f);
  info.setMaxLod(0.0f);

  return error(this->m_device->device().createSampler(
      info, nullptr, this->m_device->dispatch()));
}

auto Image::createImage() -> vk::Image {
  vk::ImageCreateInfo info;
  vk::Extent3D extent;

  extent.setWidth(this->m_info.width);
  extent.setHeight(this->m_info.height);
  extent.setDepth(1);  //@JH TODO Don't do this maybe?

  info.setExtent(extent);
  info.setUsage(this->m_usage_flags);
  info.setFormat(this->format());
  info.setImageType(this->m_type);
  info.setSamples(this->m_num_samples);
  info.setMipLevels(this->m_info.mip_maps);
  info.setArrayLayers(this->layers());
  info.setInitialLayout(vk::ImageLayout::eUndefined);
  info.setSharingMode(vk::SharingMode::eExclusive);
  info.setTiling(this->m_tiling);
  info.setFlags(this->m_flags);

  // This next section is to ask for usage flag bits.
  auto flags = std::vector<vk::ImageUsageFlags>();

  // Append all the flags we WANT....
  flags.push_back(vk::ImageUsageFlagBits::eTransferSrc);
  flags.push_back(vk::ImageUsageFlagBits::eTransferDst);
  flags.push_back(vk::ImageUsageFlagBits::eStorage);
  if (this->m_info.format == ImageFormat::Depth) {
    flags.push_back(vk::ImageUsageFlagBits::eDepthStencilAttachment);
  }
  if (this->m_start_layout == vk::ImageLayout::eColorAttachmentOptimal)
    flags.push_back(vk::ImageUsageFlagBits::eColorAttachment);

  // For each flag we want, check and see if it's allowed for this type. If not,
  // don't include it.
  for (auto flag : flags) {
    auto attempt = this->m_usage_flags | flag;
    auto result = this->m_device->p_device().getImageFormatProperties(
        info.format, info.imageType, info.tiling, attempt, info.flags,
        this->m_device->dispatch());

    if (result.result == vk::Result::eSuccess) this->m_usage_flags = attempt;
  }

  // Finally, set usage and create image.
  info.setUsage(this->m_usage_flags);
  auto result = this->m_device->device().createImage(
      info, nullptr, this->m_device->dispatch());
  return result;
}

Image::Image() {
  this->m_layout = vk::ImageLayout::eUndefined;
  this->m_old_layout = vk::ImageLayout::eUndefined;
  this->m_type = vk::ImageType::e2D;
  this->m_num_samples = vk::SampleCountFlagBits::e1;
  this->m_preallocated = false;
  this->m_tiling = vk::ImageTiling::eOptimal;
  this->m_memory = nullptr;
  this->m_device = nullptr;
  this->m_usage_flags = vk::ImageUsageFlagBits::eSampled;
  this->m_view_type = vk::ImageViewType::e2D;
  this->m_layer = 0;
  this->m_should_delete = true;
  this->m_subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
  this->m_subresource.setBaseArrayLayer(0);
  this->m_subresource.setLayerCount(this->layers());
  this->m_subresource.setMipLevel(0);
}

Image::Image(const Image& orig, unsigned layer) {
  this->m_layout = orig.m_layout;
  this->m_old_layout = orig.m_old_layout;
  this->m_type = orig.m_type;
  this->m_num_samples = orig.m_num_samples;
  this->m_preallocated = orig.m_preallocated;
  this->m_tiling = orig.m_tiling;
  this->m_memory = orig.m_memory;
  this->m_device = orig.m_device;
  this->m_usage_flags = orig.m_usage_flags;
  this->m_view_type = orig.m_view_type;
  this->m_subresource = orig.m_subresource;

  this->m_should_delete = false;
  this->m_layer = layer;
  this->m_subresource.setBaseArrayLayer(layer);
  this->m_subresource.setLayerCount(1);
}

Image::Image(Image&& mv) { *this = std::move(mv); }

Image::~Image() {
  if (this->m_should_delete) {
    if (this->m_image && this->m_device) {
      this->m_device->device().destroy(this->m_sampler, nullptr,
                                       this->m_device->dispatch());
      this->m_device->device().destroy(this->m_view, nullptr,
                                       this->m_device->dispatch());

      if (!this->m_preallocated)
        this->m_device->device().destroy(this->m_image, nullptr,
                                         this->m_device->dispatch());
    }
    this->m_layout = vk::ImageLayout::eUndefined;
    this->m_old_layout = vk::ImageLayout::eUndefined;
    this->m_type = vk::ImageType::e2D;
    this->m_num_samples = vk::SampleCountFlagBits::e1;
    this->m_preallocated = false;
    this->m_tiling = vk::ImageTiling::eOptimal;
    this->m_memory = nullptr;
    this->m_device = nullptr;
    this->m_usage_flags = vk::ImageUsageFlagBits::eSampled;
    this->m_view_type = vk::ImageViewType::e2D;
    this->m_layer = 0;
    this->m_should_delete = true;

    this->m_subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
    this->m_subresource.setBaseArrayLayer(0);
    this->m_subresource.setLayerCount(this->layers());
    this->m_subresource.setMipLevel(0);
  }

  this->m_layout = vk::ImageLayout::eUndefined;
}

auto Image::operator=(Image&& mv) -> Image& {
  this->m_layout = mv.m_layout;
  this->m_old_layout = mv.m_old_layout;
  this->m_type = mv.m_type;
  this->m_num_samples = mv.m_num_samples;
  this->m_preallocated = mv.m_preallocated;
  this->m_tiling = mv.m_tiling;
  this->m_memory = mv.m_memory;
  this->m_device = mv.m_device;
  this->m_usage_flags = mv.m_usage_flags;
  this->m_view_type = mv.m_view_type;
  this->m_subresource = mv.m_subresource;
  this->m_should_delete = mv.m_should_delete;
  this->m_layer = mv.m_layer;

  mv.m_layout = vk::ImageLayout::eUndefined;
  mv.m_old_layout = vk::ImageLayout::eUndefined;
  mv.m_type = vk::ImageType::e2D;
  mv.m_num_samples = vk::SampleCountFlagBits::e1;
  mv.m_preallocated = false;
  mv.m_tiling = vk::ImageTiling::eOptimal;
  mv.m_memory = nullptr;
  mv.m_device = nullptr;
  mv.m_usage_flags = vk::ImageUsageFlagBits::eSampled;
  mv.m_view_type = vk::ImageViewType::e2D;
  mv.m_layer = 0;
  mv.m_should_delete = true;
  mv.m_subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
  mv.m_subresource.setBaseArrayLayer(0);
  mv.m_subresource.setLayerCount(this->layers());
  mv.m_subresource.setMipLevel(0);

  return *this;
}

auto Image::initialize(Device& device, const ImageInfo& info,
                       vk::ImageLayout start) -> size_t {
  this->m_device = &device;
  this->m_info = info;
  this->m_start_layout = start;

  this->m_subresource.setLayerCount(info.layers);
  this->m_subresource.setAspectMask(
      info.format != ImageFormat::Depth
          ? vk::ImageAspectFlagBits::eColor
          : vk::ImageAspectFlagBits::eDepth |
                vk::ImageAspectFlagBits::eStencil);

  if (info.is_cubemap) {
    this->m_view_type = vk::ImageViewType::eCube;
    this->m_type = vk::ImageType::e2D;
    this->m_info.layers = 6;

    this->m_flags |= vk::ImageCreateFlagBits::eCubeCompatible;
  } else {
    switch (info.layers) {
      case 0:
        this->m_view_type = vk::ImageViewType::e1D;
        this->m_type = vk::ImageType::e1D;
        break;
      case 1:
        this->m_view_type = vk::ImageViewType::e2D;
        break;
      default:
        this->m_view_type = vk::ImageViewType::e2DArray;
        this->m_type = vk::ImageType::e2D;
        break;
    }
  }

  this->m_image = this->createImage();

  this->m_requirements = this->m_device->device().getImageMemoryRequirements(
      this->m_image, this->m_device->dispatch());

  return this->m_requirements.size;
}

auto Image::initialize(Device& device, const ImageInfo& info,
                       vk::Image prealloc, vk::ImageLayout start) -> size_t {
  this->m_device = &device;
  this->m_info = info;
  this->m_start_layout = start;

  this->m_subresource.setLayerCount(info.layers);

  if (info.is_cubemap) {
    this->m_view_type = vk::ImageViewType::eCube;
  } else {
    switch (info.layers) {
      case 0:
        this->m_view_type = vk::ImageViewType::e1D;
        this->m_type = vk::ImageType::e1D;
        break;
      case 1:
        this->m_view_type = vk::ImageViewType::e2D;
        break;
      default:
        this->m_view_type = vk::ImageViewType::e2DArray;
        this->m_type = vk::ImageType::e2D;
        break;
    }
  }

  this->m_image = prealloc;
  this->m_preallocated = true;
  this->m_requirements = this->m_device->device().getImageMemoryRequirements(
      this->m_image, this->m_device->dispatch());
  this->m_view = this->createView();
  this->m_sampler = this->createSampler();
  return this->m_requirements.size;
}

auto Image::bind(Memory& memory) -> void {
  OhmException(
      !this->initialized(), Error::APIError,
      "Attempting to bind memory to a Image that has not been initialized.");
  OhmException(memory.size < this->m_requirements.size, Error::APIError,
               "Attempting to bind memory to texture without enough memory "
               "allocated.");

  this->m_memory = &memory;
  error(this->m_device->device().bindImageMemory(
      this->m_image, memory.memory, memory.offset, this->m_device->dispatch()));
  this->m_view = this->createView();
  this->m_sampler = this->createSampler();

  auto oneshot = CommandBuffer(*this->m_device, QueueType::Graphics);
  oneshot.transition(*this, this->m_start_layout);
  oneshot.submit();
  oneshot.synchronize();
}

auto Image::setUsage(vk::ImageUsageFlags usage) -> void {
  this->m_usage_flags = usage;
}

auto Image::setLayout(vk::ImageLayout layout) -> void {
  this->m_layout = layout;
}
}  // namespace ovk
}  // namespace ohm
