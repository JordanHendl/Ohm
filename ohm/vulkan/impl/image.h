#pragma once
#include <vulkan/vulkan.hpp>
#include "api/image.h"
#include "command_buffer.h"
#include "device.h"
#include "memory.h"

namespace ohm {
namespace ovk {
auto convert(ImageFormat format) -> vk::Format;
auto convert(vk::Format format) -> ImageFormat;

class Image {
 public:
  /** Constructor
   */
  Image() = default;

  /** Copy constructor. Does not do a deep copy, performs a surface-level copy
   * only. ( Copies pointers to GPU data, does not allocate and copy ).
   * @param orig The image to grab the image data from.
   */
  Image(const Image& orig);

  /** Copy constructor. Does not do a deep copy, performs a surface-level copy
   * only. ( Copies pointers to GPU data, does not allocate and copy ).
   * @param orig The image to grab the image data from.
   */
  Image(const Image& orig, unsigned layer);

  Image(Image&& mv);

  /** Default deconstructor.
   */
  ~Image();

  /** Equals operator. Does not do a deep copy,  performs a surface-level copy
   * only. ( Copies pointers to GPU data, does not allocate and copy ).
   * @param src The image to grab image data from.
   * @return A reference to this image after the copy.
   */
  auto operator=(Image&& mv) -> Image&;

  auto operator=(const Image& cpy) -> Image&;

  /** Method to determine if this object was initialized or not.
   * @return Whether or not this object was initialized.
   */

  /** Method to initialize this object with the input parameters.
   */
  auto initialize(Device& gpu, const ImageInfo& info,
                  vk::ImageLayout start = vk::ImageLayout::eGeneral) -> size_t;

  /** Method to initialize this object with the input parameters.
   */
  auto initialize(Device& gpu, const ImageInfo& info, vk::Image prealloc,
                  vk::ImageLayout start = vk::ImageLayout::eGeneral) -> size_t;

  inline auto initialized() const -> bool { return this->m_image; }
  inline auto bind(Memory& memory) -> void;
  inline auto layer() const { return this->m_layer; }
  inline auto memory() -> Memory& { return *this->m_memory; }
  inline auto size() const -> size_t { return this->m_requirements.size; };
  inline auto layout() const { return this->m_layout; }
  inline auto format() const { return convert(this->m_info.format); }
  inline auto ohm_format() const { return this->m_info.format; }
  inline auto offset() const {
    return this->m_memory != nullptr ? this->m_memory->offset : 0;
  }
  inline auto view() const { return this->m_view; }
  inline auto sampler() const { return this->m_sampler; }
  inline auto image() const { return this->m_image; }
  inline auto count() const { return this->m_info.count(); }
  inline auto width() const { return this->m_info.width; }
  inline auto height() const { return this->m_info.height; }
  inline auto layers() const { return this->m_info.layers; }
  inline auto subresource() const { return this->m_subresource; }

  auto setUsage(vk::ImageUsageFlags usage) -> void;
  auto setLayout(vk::ImageLayout layout) -> void;

  /** Method to retrieve the image barrier used for this object.
   * @note This is so the lifetime of this barrier exists when any barrier
   * commands on this object are executed. If, for example, a local barrier is
   * used, then it will not exist by the time the command buffer is executed,
   * causing at best a crash, and at worse, undefined behavior. To circumvent
   * this, barriers are stored in each object, to guarantee lifetime.
   * @return The barrier used for all barrier operation on this object.
   */
  inline auto barrier() { return this->m_barrier; }

 private:
  Device* m_device;
  Memory* m_memory;
  vk::MemoryRequirements m_requirements;
  vk::ImageSubresourceLayers m_subresource;
  bool m_preallocated;
  ImageInfo m_info;
  vk::ImageTiling m_tiling;
  vk::Image m_image;
  vk::ImageView m_view;
  vk::Sampler m_sampler;
  mutable vk::ImageLayout m_start_layout;
  mutable vk::ImageLayout m_layout;
  mutable vk::ImageLayout m_old_layout;
  vk::ImageType m_type;
  vk::SampleCountFlagBits m_num_samples;
  vk::ImageUsageFlags m_usage_flags;
  vk::ImageMemoryBarrier m_barrier;
  vk::ImageViewType m_view_type;
  vk::ImageCreateFlags m_flags;
  unsigned m_layer;
  bool m_should_delete;

  /** Helper method to create a vulkan image view.
   * @return A Valid vulkan image view.
   */
  inline auto createView() -> vk::ImageView;

  /** Method to create a vulkan sampler.
   * @return A Valid vulkan sampler.
   */
  inline auto createSampler() -> vk::Sampler;

  /** Method to create a vulkan image.
   * @return A Made vulkan image.
   */
  inline auto createImage() -> vk::Image;
};
}  // namespace ovk
}  // namespace ohm
