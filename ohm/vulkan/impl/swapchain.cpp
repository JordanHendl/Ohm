
#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "swapchain.h"
#include "ohm/vulkan/impl/error.h"
#include "ohm/vulkan/impl/system.h"
namespace ohm {
namespace ovk {
Swapchain::Swapchain(Device& device, vk::SurfaceKHR surface, bool vsync) {
  auto fence_info = vk::FenceCreateInfo();
  fence_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

  this->m_dependency = nullptr;
  this->m_current_frame = 0;
  this->m_skip_frame = false;
  this->m_surface = surface;
  this->m_device = &device;
  this->m_vsync = vsync;

  this->m_fences.clear();
  this->find_properties();
  this->choose_extent();
  this->make_swapchain();
  this->gen_images();

  this->m_fences.resize(this->textures().size());
  this->m_image_available.resize(this->textures().size());
  this->m_present_done.resize(this->textures().size());
  this->m_fences_in_flight.resize(this->textures().size());

  auto gpu = device.device();
  auto& dispatch = device.dispatch();
  auto alloc_cb = device.allocationCB();
  for (auto& sem : this->m_image_available) {
    sem = error(gpu.createSemaphore({}, nullptr, dispatch));
  }

  for (auto& fence : this->m_fences)
    fence = error(gpu.createFence(fence_info, alloc_cb, dispatch));
  for (auto& fence : this->m_fences_in_flight) fence = nullptr;

  this->acquire();
}

Swapchain::Swapchain(Swapchain&& mv) { *this = std::move(mv); }

Swapchain::~Swapchain() {
  if (this->m_swapchain) {
    auto gpu = this->m_device->device();
    auto& dispatch = this->m_device->dispatch();
    auto alloc_cb = this->m_device->allocationCB();

    gpu.destroy(this->m_swapchain, alloc_cb, dispatch);
    this->m_swapchain = nullptr;

    for (auto& sem : this->m_image_available)
      gpu.destroy(sem, alloc_cb, dispatch);
    for (auto& sem : this->m_present_done) gpu.destroy(sem, alloc_cb, dispatch);

    this->m_images.clear();
  }
}

auto Swapchain::operator=(Swapchain&& mv) -> Swapchain& {
  this->m_fences = mv.m_fences;
  this->m_fences_in_flight = mv.m_fences_in_flight;
  this->m_formats = mv.m_formats;
  this->m_modes = mv.m_modes;
  this->m_images = mv.m_images;
  this->m_image_available = mv.m_image_available;
  this->m_present_done = mv.m_present_done;
  this->m_queue = mv.m_queue;
  this->m_device = mv.m_device;
  this->m_dependency = mv.m_dependency;
  this->m_swapchain = mv.m_swapchain;
  this->m_capabilities = mv.m_capabilities;
  this->m_surface_format = mv.m_surface_format;
  this->m_surface = mv.m_surface;
  this->m_extent = mv.m_extent;
  this->m_acquired = mv.m_acquired;
  this->m_current_frame = mv.m_current_frame;
  this->m_skip_frame = mv.m_skip_frame;
  this->m_vsync = mv.m_vsync;

  mv.m_fences.clear();
  mv.m_fences_in_flight.clear();
  mv.m_formats.clear();
  mv.m_modes.clear();
  mv.m_images.clear();
  mv.m_image_available.clear();
  mv.m_present_done.clear();
  mv.m_current_frame = 0;
  mv.m_queue = nullptr;
  mv.m_device = nullptr;
  mv.m_dependency = nullptr;
  mv.m_swapchain = nullptr;
  mv.m_surface = nullptr;
  mv.m_skip_frame = false;
  mv.m_vsync = false;

  while (!mv.m_acquired.empty()) mv.m_acquired.pop();
  return *this;
}

auto Swapchain::make_swapchain() -> void {
  auto info = vk::SwapchainCreateInfoKHR();

  auto usage = vk::ImageUsageFlagBits::eTransferDst |
               vk::ImageUsageFlagBits::eTransferSrc |
               vk::ImageUsageFlagBits::eColorAttachment;
  choose_format(vk::Format::eB8G8R8A8Srgb,
                vk::ColorSpaceKHR::eSrgbNonlinear);  // TODO make config

  auto present_mode = this->m_vsync ? vk::PresentModeKHR::eFifo
                                    : vk::PresentModeKHR::eImmediate;

  info.setSurface(this->m_surface);
  info.setMinImageCount(this->m_capabilities.minImageCount + 1);
  info.setImageFormat(this->m_surface_format.format);
  info.setImageColorSpace(this->m_surface_format.colorSpace);
  info.setImageExtent(this->m_extent);
  info.setImageArrayLayers(1);  // TODO make config.
  info.setImageUsage(usage);    // TODO make config.
  info.setPreTransform(this->m_capabilities.currentTransform);
  info.setCompositeAlpha(
      vk::CompositeAlphaFlagBitsKHR::eOpaque);  // TODO make config.
  info.setPresentMode(this->select_mode(present_mode));

  info.setImageSharingMode(vk::SharingMode::eExclusive);
  info.setQueueFamilyIndexCount(0);
  info.setQueueFamilyIndices(nullptr);

  this->m_swapchain = error(this->m_device->device().createSwapchainKHR(
      info, this->m_device->allocationCB(), this->m_device->dispatch()));
}

auto Swapchain::gen_images() -> void {
  auto info = ImageInfo();
  auto gpu = this->m_device->device();
  auto& dispatch = this->m_device->dispatch();

  info.width = this->width();
  info.height = this->height();
  info.format = convert(this->m_surface_format.format);
  info.mip_maps = 1;
  info.layers = 1;

  auto images = error(gpu.getSwapchainImagesKHR(this->m_swapchain, dispatch));

  this->m_images.reserve(images.size());
  auto index = 0;
  for (auto& img : ovk::system().image) {
    if (!img.initialized()) {
      img = std::move(ovk::Image(this->device(), info, images[index]));
      this->m_images.push_back(index);
    }
    index++;
  }
}

auto Swapchain::choose_format(vk::Format value, vk::ColorSpaceKHR color)
    -> void {
  for (const auto& format : this->m_formats) {
    if (format.format == value && format.colorSpace == color) {
      this->m_surface_format = format;
    }
  }
}

void Swapchain::find_properties() {
  const auto device = this->m_device->p_device();
  auto& dispatch = this->m_device->dispatch();
  this->m_formats = error(
      device.getSurfaceFormatsKHR(this->m_surface, this->m_device->dispatch()));
  this->m_capabilities =
      error(device.getSurfaceCapabilitiesKHR(this->m_surface, dispatch));
  this->m_modes = error(device.getSurfacePresentModesKHR(
      this->m_surface, this->m_device->dispatch()));
}

void Swapchain::choose_extent() {
  if (this->m_capabilities.currentExtent.width != UINT32_MAX) {
    this->m_extent = this->m_capabilities.currentExtent;
  } else {
    this->m_extent.width =
        std::max(this->m_capabilities.minImageExtent.width,
                 std::min(this->m_capabilities.maxImageExtent.width,
                          this->m_extent.width));
    this->m_extent.height =
        std::max(this->m_capabilities.minImageExtent.height,
                 std::min(this->m_capabilities.maxImageExtent.height,
                          this->m_extent.height));
  }
}

vk::PresentModeKHR Swapchain::select_mode(vk::PresentModeKHR value) {
  for (auto& mode : this->m_modes) {
    if (mode == value) return mode;
  }

  return vk::PresentModeKHR::eFifo;
}

auto Swapchain::acquire() -> bool {
  auto gpu = this->m_device->device();
  auto img = 0u;
  auto& dispatch = this->m_device->dispatch();
  auto& sem = this->m_image_available[this->m_current_frame];

  auto result = gpu.acquireNextImageKHR(this->m_swapchain, UINT64_MAX, sem,
                                        nullptr, &img, dispatch);
  if (result != vk::Result::eSuccess) {
    this->m_skip_frame = true;
    return false;
  }

  this->m_acquired.push(static_cast<unsigned>(img));
  this->m_skip_frame = false;
  return true;
}

auto Swapchain::present() -> bool {
  if (!this->m_skip_frame) {
    if (this->m_dependency) {
      auto dep = this->m_dependency;
      dep->clearDependancies();
      dep->addDependancy(this->m_image_available[this->current()]);
      this->m_fences_in_flight[this->current()] = dep->fence();
      dep->submit();
      if (!dep->present(*this)) {
        this->m_current_frame =
            (this->m_current_frame + 1) % this->textures().size();
        return false;
      }

      this->m_current_frame =
          (this->m_current_frame + 1) % this->textures().size();
      this->m_acquired.pop();
      return this->acquire();
    }
  } else {
    if (this->m_dependency) this->m_dependency->end();
    this->m_current_frame =
        (this->m_current_frame + 1) % this->textures().size();
    this->m_acquired.pop();
  }

  return true;
}

void Swapchain::wait(CommandBuffer& cmd) {
  OhmAssert(cmd.depended(),
            "Attempting to set a dependency on a command buffer that already "
            "is being depended upon.");
  this->m_dependency = &cmd;
  this->m_dependency->setDepended(true);
}
}  // namespace ovk
}  // namespace ohm
