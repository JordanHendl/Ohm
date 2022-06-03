#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "ohm/vulkan/vulkan_impl.h"
#include <SDL.h>
#include <atomic>
#include <algorithm>
#include "impl/swapchain.h"
#include "ohm/api/exception.h"
#include "ohm/api/memory.h"
#include "ohm/api/system.h"
#include "ohm/vulkan/impl/error.h"
#include "ohm/vulkan/impl/system.h"
#ifdef __linux__
#include <stdlib.h>
#include <unistd.h>
#elif _WIN32
//@JH TODO find out how to get system name from windows API.
#endif

namespace ohm {
inline namespace v1 {
auto Vulkan::System::initialize() Ohm_NOEXCEPT -> void {
  auto& loader = ovk::system().loader;
  if (!loader.initialized()) {
#ifdef _WIN32
    loader.load("vulkan-1.dll");
#elif __linux__
    loader.load("libvulkan.so.1");
#endif

    ovk::system().instance.initialize(loader, ovk::system().allocate_cb);
    ovk::system().devices.reserve(ovk::system().instance.devices().size());
    ovk::system().gpus.reserve(ovk::system().instance.devices().size());

    for (auto& p_device : ovk::system().instance.devices()) {
      auto device = ovk::Device();
      for (auto& extension : ovk::system().device_extensions) {
        device.addExtension(extension.c_str());
      }

      for (auto& validation : ovk::system().validation_layers) {
        device.addValidation(validation.c_str());
      }

      device.initialize(loader, ovk::system().allocate_cb,
                        p_device);
      ovk::system().devices.emplace_back(std::move(device));
    }
    auto compare = [](ovk::Device& a, ovk::Device& b) {
      return a.score() > b.score();
    };
    std::sort(ovk::system().devices.begin(), ovk::system().devices.end(), compare);
    for(auto index = 0u; index < ovk::system().devices.size(); index++) {
      auto gpu = ohm::Gpu();
      gpu.name = ovk::system().devices[index].name();
      ovk::system().gpus.emplace_back(std::move(gpu));
    }
  }
}

auto Vulkan::System::name() Ohm_NOEXCEPT -> std::string {
#ifdef __linux__
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);
  return std::string(hostname);
#elif _WIN32
  return "";
#endif
}

auto Vulkan::System::shutdown() Ohm_NOEXCEPT -> void {
  ovk::system().shutdown();
}

auto Vulkan::System::set_parameter(std::string_view) Ohm_NOEXCEPT -> void {
  // nop... for now.
}

auto Vulkan::System::set_debug_parameter(std::string_view str) Ohm_NOEXCEPT
    -> void {
  ovk::system().validation_layers.push_back(std::string(str));
  ovk::system().instance.addValidationLayer(str.cbegin());
}

auto Vulkan::System::devices() Ohm_NOEXCEPT -> std::vector<Gpu> {
  return ovk::system().gpus;
}

auto Vulkan::Memory::heaps(int gpu) Ohm_NOEXCEPT
    -> const std::vector<GpuMemoryHeap>& {
  auto& device = ovk::system().devices.at(gpu);
  return device.heaps();
}

//@JH TODO would like to simplify this as its a bit messy.
auto Vulkan::Memory::allocate(int gpu, HeapType requested, size_t heap_index,
                              size_t size) Ohm_NOEXCEPT -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto mem_type_count = device.memoryProperties().memoryTypeCount;
  auto index = 0;
  for (auto& mem : ovk::system().memory) {
    if (!mem.initialized()) {
      for (auto type_index = 0u; type_index < mem_type_count; type_index++) {
        auto& type = device.memoryProperties().memoryTypes[type_index];
        auto vk_type = vk::MemoryPropertyFlags();
        if (requested & HeapType::HostVisible)
          vk_type = vk_type | vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent;
        else
          vk_type = vk_type | vk::MemoryPropertyFlagBits::eDeviceLocal;

        if (type.heapIndex == heap_index && (type.propertyFlags & vk_type)) {
          mem = std::move(ovk::Memory(device, size, type_index, requested));
          return index;
        }
      }
      OhmAssert(true,
                "Tried allocating to a heap that doesn't match requested "
                "allocation type.");
    }
    index++;
  }

  OhmAssert(true, "Too many memory allocations.");
  return 0;
}

auto Vulkan::Memory::type(int32_t handle) Ohm_NOEXCEPT -> HeapType {
  OhmAssert(handle < 0, "Invalid handle passed to API.");
  auto& mem = ovk::system().memory[handle];
  OhmAssert(!mem.initialized(),
            "Attempting to use memory object that is not initialized.");
  return mem.type;
}

auto Vulkan::Memory::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Invalid handle passed to API.");
  auto& mem = ovk::system().memory[handle];
  auto tmp = ovk::Memory();
  OhmAssert(!mem.initialized(),
            "Attempting to use memory object that is not initialized.");
  tmp = std::move(mem);
}

auto Vulkan::Memory::size(int32_t handle) Ohm_NOEXCEPT -> size_t {
  OhmAssert(handle < 0, "Invalid handle passed to API.");
  auto& mem = ovk::system().memory[handle];
  OhmAssert(!mem.initialized(),
            "Attempting to use memory object that is not initialized.");
  return mem.size - mem.offset;
}

auto Vulkan::Memory::offset(int32_t handle, size_t offset) Ohm_NOEXCEPT
    -> size_t {
  OhmAssert(handle < 0, "Invalid handle passed to API.");
  auto& parent = ovk::system().memory[handle];
  OhmAssert(!parent.initialized(),
            "Attempting to use memory object that is not initialized.");
  auto index = 0;
  for (auto& mem : ovk::system().memory) {
    if (!mem.initialized()) {
      mem = std::move(ovk::Memory(parent, offset));
      return index;
    }
    index++;
  }

  OhmAssert(true, "Too many memory allocations.");
  return 0;
}

auto Vulkan::Array::create(int gpu, size_t num_elmts,
                           size_t elm_size) Ohm_NOEXCEPT -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto index = 0;
  for (auto& buf : ovk::system().buffer) {
    if (!buf.initialized()) {
      buf = std::move(ovk::Buffer(device, num_elmts, elm_size));
      return index;
    }
    index++;
  }

  OhmAssert(true, "Too many buffers. API has run out of allocation.");
  return -1;
}

auto Vulkan::Array::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to delete an invalid array handle.");
  auto& buf = ovk::system().buffer[handle];
  auto tmp = ovk::Buffer();

  OhmAssert(!buf.initialized(),
            "Attempting to use array object that is not initialized.");
  tmp = std::move(buf);
}

auto Vulkan::Array::required(int32_t handle) Ohm_NOEXCEPT -> size_t {
  OhmAssert(handle < 0, "Attempting to query an invalid array handle.");
  auto& buf = ovk::system().buffer[handle];

  OhmAssert(!buf.initialized(),
            "Attempting to use array object that is not initialized.");
  return buf.size();
}

auto Vulkan::Array::bind(int32_t array_handle,
                         int32_t memory_handle) Ohm_NOEXCEPT -> void {
  OhmAssert(array_handle < 0, "Attempting to bind to an invalid array handle.");
  OhmAssert(memory_handle < 0, "Attempting to bind an invalid memory handle.");
  auto& buf = ovk::system().buffer[array_handle];
  auto& mem = ovk::system().memory[memory_handle];

  OhmAssert(!buf.initialized(),
            "Attempting to use array object that is not initialized.");
  buf.bind(mem);
}

auto Vulkan::Image::create(int gpu, const ImageInfo& info) Ohm_NOEXCEPT
    -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto index = 0;
  for (auto& val : ovk::system().image) {
    if (!val.initialized()) {
      val = std::move(ovk::Image(device, info));
      return index;
    }
    index++;
  }

  OhmAssert(true, "Too many images. API has run out of allocation.");
  return -1;
}

auto Vulkan::Image::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to delete an invalid array handle.");
  auto& val = ovk::system().image[handle];
  auto tmp = ovk::Image();

  OhmAssert(!val.initialized(),
            "Attempting to use image object that is not initialized.");
  tmp = std::move(val);
}

auto Vulkan::Image::layer(int32_t handle, size_t layer) Ohm_NOEXCEPT
    -> int32_t {
  OhmAssert(handle < 0, "Attempting to delete an invalid image handle.");
  auto& parent = ovk::system().image[handle];
  auto index = 0;
  for (auto& val : ovk::system().image) {
    if (!val.initialized()) {
      val = std::move(ovk::Image(parent, layer));
      return index;
    }
    index++;
  }

  OhmAssert(true, "Too many buffers. API has run out of allocation.");
  return -1;
}

auto Vulkan::Image::required(int32_t handle) Ohm_NOEXCEPT -> size_t {
  OhmAssert(handle < 0, "Attempting to delete an invalid image handle.");
  auto& val = ovk::system().image[handle];
  auto tmp = ovk::Image();

  OhmAssert(!val.initialized(),
            "Attempting to use image object that is not initialized.");
  return val.size();
}

auto Vulkan::Image::bind(int32_t handle, int32_t mem_handle) Ohm_NOEXCEPT
    -> void {
  OhmAssert(handle < 0, "Attempting to delete an invalid array handle.");
  auto& val = ovk::system().image[handle];
  auto& mem = ovk::system().memory[mem_handle];
  auto tmp = ovk::Image();

  OhmAssert(!val.initialized(),
            "Attempting to use image object that is not initialized.");
  OhmAssert(!mem.initialized(),
            "Attempting to use memory object that is not initialized.");
  val.bind(mem);
}

auto Vulkan::Commands::create(int gpu, QueueType type) Ohm_NOEXCEPT -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto index = 0;
  for (auto& buf : ovk::system().commands) {
    if (!buf.initialized()) {
      buf = std::move(ovk::CommandBuffer(device, type));
      return index;
    }
    index++;
  }

  OhmAssert(true, "Too many commands. API has run out of allocation.");
  return -1;
}

auto Vulkan::Commands::draw(int32_t handle, int32_t vertices, size_t instance_count) Ohm_NOEXCEPT -> void {

}

auto Vulkan::Commands::draw_indexed(int32_t handle, int32_t indices, int32_t vertices, size_t instance_count) Ohm_NOEXCEPT -> void {

}

auto Vulkan::Commands::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to destroy an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];
  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  auto tmp = ovk::CommandBuffer();
  tmp = std::move(cmd);
}

auto Vulkan::Commands::begin(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to destroy an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];

  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  cmd.begin();
}

auto Vulkan::Commands::bind(int32_t handle, int32_t desc) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to destroy an invalid commands handle.");
  OhmAssert(desc < 0, "Attempting to destroy an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];
  auto& descriptor = ovk::system().descriptor[desc];

  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  OhmAssert(!descriptor.initialized(),
            "Attempting to use object that is not initialized.");
  cmd.bind(descriptor);
}

auto Vulkan::Commands::copy_to_image(int32_t handle, int32_t src, int32_t dst,
                                     size_t count) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  OhmAssert(src < 0, "Attempting to use an invalid src array handle.");
  OhmAssert(dst < 0, "Attempting to use an invalid dst image handle.");

  auto& cmd = ovk::system().commands[handle];

  auto& r_src = ovk::system().buffer[src];
  auto& r_dst = ovk::system().image[dst];

  cmd.copy(r_src, r_dst, count);
}

auto Vulkan::Commands::copy_image(int32_t handle, int32_t src, int32_t dst,
                                  size_t count) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  OhmAssert(src < 0, "Attempting to use an invalid src image handle.");
  OhmAssert(dst < 0, "Attempting to use an invalid dst image handle.");

  auto& cmd = ovk::system().commands[handle];

  auto& r_src = ovk::system().image[src];
  auto& r_dst = ovk::system().image[dst];

  cmd.copy(r_src, r_dst, count);
}

auto Vulkan::Commands::copy_array(int32_t handle, int32_t src, int32_t dst,
                                  size_t count) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  OhmAssert(src < 0, "Attempting to use an invalid src array handle.");
  OhmAssert(dst < 0, "Attempting to use an invalid dst array handle.");
  auto& cmd = ovk::system().commands[handle];

  auto& src_buf = ovk::system().buffer[src];
  auto& dst_buf = ovk::system().buffer[dst];

  cmd.copy(src_buf, dst_buf, count);
}

auto Vulkan::Commands::copy_array(int32_t handle, int32_t src, void* dst,
                                  size_t count) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  OhmAssert(src < 0, "Attempting to use an invalid src array handle.");
  OhmAssert(dst == nullptr, "Attempting to use an invalid dst ptr.");

  auto& cmd = ovk::system().commands[handle];
  auto& src_buf = ovk::system().buffer[src];

  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  cmd.copy(src_buf, static_cast<unsigned char*>(dst), count);
}

auto Vulkan::Commands::copy_array(int32_t handle, const void* src, int32_t dst,
                                  size_t count) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  OhmAssert(src == nullptr, "Attempting to use an invalid src pointer.");
  OhmAssert(dst < 0, "Attempting to use an invalid dst array handle.");

  auto& cmd = ovk::system().commands[handle];
  auto& dst_buf = ovk::system().buffer[dst];

  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  cmd.copy(static_cast<const unsigned char*>(src), dst_buf, count);
}

auto Vulkan::Commands::dispatch(int32_t handle, size_t x, size_t y,
                                size_t z) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];

  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  cmd.dispatch(x, y, z);
}

auto Vulkan::Commands::blit_to_window(int32_t handle, int32_t src, int32_t dst,
                                      Filter filter) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  OhmAssert(dst < 0, "Attempting to use an invalid window handle.");
  auto& cmd = ovk::system().commands[handle];
  auto& img = ovk::system().image[src];
  auto& swapchain = ovk::system().swapchain[dst];
  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  OhmAssert(!img.initialized(),
            "Attempting to use object that is not initialized.");
  OhmAssert(!swapchain.initialized(),
            "Attempting to use object that is not initialized.");
  cmd.blit(img, swapchain, filter);
}

auto Vulkan::Commands::blit_to_image(int32_t handle, int32_t src, int32_t dst,
                                     Filter filter) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  OhmAssert(dst < 0, "Attempting to use an invalid window handle.");
  auto& cmd = ovk::system().commands[handle];
  auto& img1 = ovk::system().image[src];
  auto& img2 = ovk::system().image[dst];
  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  OhmAssert(!img1.initialized(),
            "Attempting to use object that is not initialized.");
  OhmAssert(!img2.initialized(),
            "Attempting to use object that is not initialized.");
  cmd.blit(img1, img2, filter);
}

auto Vulkan::Commands::blit_from_renderpass(int32_t handle, int32_t src,
                                            int32_t dst,
                                            Filter filter) Ohm_NOEXCEPT
    -> void {}

auto Vulkan::Commands::submit(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];

  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  cmd.submit();
}

auto Vulkan::Commands::synchronize(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to use an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];

  OhmAssert(!cmd.initialized(),
            "Attempting to use object that is not initialized.");
  cmd.synchronize();
}

auto Vulkan::RenderPass::create(int gpu,
                                const RenderPassInfo& info) Ohm_NOEXCEPT
    -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto index = 0;
  for (auto& val : ovk::system().render_pass) {
    if (!val.initialized()) {
      val = std::move(ovk::RenderPass(device, info));
      return index;
    }
    index++;
  }

  OhmAssert(true, "Too many render passes. API has run out of allocation.");
  return -1;
}

auto Vulkan::RenderPass::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to destroy an invalid render pass handle.");
  auto& pass = ovk::system().render_pass[handle];
  OhmAssert(!pass.initialized(),
            "Attempting to use object that is not initialized.");
  auto tmp = ovk::RenderPass();
  tmp = std::move(pass);
}

auto Vulkan::RenderPass::count(int32_t handle) Ohm_NOEXCEPT -> size_t {
  OhmAssert(handle < 0, "Attempting to destroy an invalid render pass handle.");
  auto& pass = ovk::system().render_pass[handle];
  OhmAssert(!pass.initialized(),
            "Attempting to use object that is not initialized.");
  return pass.framebuffers().size();
}

auto Vulkan::RenderPass::image(int32_t handle, size_t index) -> int32_t {
  OhmAssert(handle < 0, "Attempting to destroy an invalid render pass handle.");
  auto& pass = ovk::system().render_pass[handle];
  OhmAssert(!pass.initialized(),
            "Attempting to use object that is not initialized.");
  OhmAssert(index >= pass.framebuffers().size(),
            "Accessing render pass out of bounds.");
  return pass.framebuffers()[index];
}

auto Vulkan::Pipeline::create(int gpu, const PipelineInfo& info) Ohm_NOEXCEPT
    -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto index = 0;
  for (auto& pipe : ovk::system().pipeline) {
    if (!pipe.initialized()) {
      pipe = std::move(ovk::Pipeline(device, info));
      return index;
    }
    index++;
  }

  OhmAssert(true, "Too many pipelines. API has run out of allocation space.");
  return -1;
}

auto Vulkan::Pipeline::create_from_rp(int32_t rp_handle,
                                      const PipelineInfo& info) Ohm_NOEXCEPT
    -> int32_t {
  auto& rp = ovk::system().render_pass[rp_handle];
  auto index = 0;
  for (auto& val : ovk::system().pipeline) {
    if (!val.initialized()) {
      val = std::move(ovk::Pipeline(rp, info));
      return index;
    }
    index++;
  }

  OhmAssert(true, "Too many pipelines. API has run out of allocation.");
  return -1;
}

auto Vulkan::Pipeline::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to delete an invalid pipeline handle.");
  auto& pipe = ovk::system().pipeline[handle];
  auto tmp = ovk::Pipeline();

  OhmAssert(!pipe.initialized(),
            "Attempting to destroy a pipeline object that is not initialized.");
  tmp = std::move(pipe);
}

auto Vulkan::Pipeline::descriptor(int32_t handle) Ohm_NOEXCEPT -> int32_t {
  auto& pipeline = ovk::system().pipeline[handle];
  auto index = 0;
  for (auto& val : ovk::system().descriptor) {
    if (!val.initialized()) {
      val = std::move(pipeline.descriptor());
      return index;
    }
    index++;
  }

  OhmAssert(true, "Too many descriptors. API has run out of allocation space.");
  return -1;
}

auto Vulkan::Descriptor::destroy(int32_t handle) -> void {
  OhmAssert(handle < 0, "Attempting to delete an invalid descriptor handle.");
  auto& val = ovk::system().descriptor[handle];
  auto tmp = ovk::Descriptor();

  OhmAssert(
      !val.initialized(),
      "Attempting to destroy a descriptor object that is not initialized.");
  tmp = std::move(val);
}

auto Vulkan::Descriptor::bind_array(int32_t handle, std::string_view name,
                                    int32_t array) -> void {
  OhmAssert(handle < 0, "Attempting to delete an invalid descriptor handle.");
  auto& val = ovk::system().descriptor[handle];
  auto& arr = ovk::system().buffer[array];
  OhmAssert(!val.initialized(),
            "Attempting to use a descriptor object that is not initialized.");
  OhmAssert(!arr.initialized(),
            "Attempting to use an array object that is not initialized.");

  val.bind(name, arr);
}

auto Vulkan::Descriptor::bind_image(int32_t handle, std::string_view name,
                                    int32_t image) -> void {
  OhmAssert(handle < 0, "Attempting to delete an invalid descriptor handle.");
  auto& val = ovk::system().descriptor[handle];
  auto& img = ovk::system().image[image];
  OhmAssert(!val.initialized(),
            "Attempting to use a descriptor object that is not initialized.");
  OhmAssert(!img.initialized(),
            "Attempting to use an image object that is not initialized.");

  val.bind(name, img);
}

auto Vulkan::Descriptor::bind_images(int32_t handle, std::string_view name,
                                     const std::vector<int32_t>& images)
    -> void {
  auto images_to_bind = std::vector<const ovk::Image*>();
  images_to_bind.reserve(images.size());

  OhmAssert(handle < 0, "Attempting to delete an invalid descriptor handle.");
  auto& val = ovk::system().descriptor[handle];

  for (auto& image : images) {
    auto& img = ovk::system().image[image];
    OhmAssert(!img.initialized(), "Attempting to bind an invalid image.");
    images_to_bind.push_back(&img);
  }

  val.bind(name, images_to_bind.data(), images.size());
}

auto Vulkan::Event::create() Ohm_NOEXCEPT -> int32_t {
  static auto id = std::atomic<int32_t>{0};

  auto handle = id++;
  ovk::system().event[handle] = std::make_shared<ovk::Event>();
  return handle;
}

auto Vulkan::Event::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  auto iter = ovk::system().event.find(handle);

  if (iter != ovk::system().event.end()) ovk::system().event.erase(handle);
}

auto Vulkan::Event::add(int32_t handle,
                        std::function<void(const ohm::Event&)> cb) -> void {
  auto event = ovk::system().event[handle];
  event->add(cb);
}

auto Vulkan::Event::poll() -> void {
  auto event = SDL_Event();
  while (SDL_PollEvent(&event) != 0) {
    for (auto& ev : ovk::system().event) {
      ev.second->push(event);
    }
  }
}

auto Vulkan::Window::create(int gpu, const WindowInfo& info) Ohm_NOEXCEPT
    -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto index = 0;
  for (auto& window : ovk::system().window) {
    if (!window.initialized()) {
      // This vulkan implementation couples swapchains & images 1:1 so
      // we have to create both here.
      auto& swap = ovk::system().swapchain[index];
      window = std::move(ovk::Window(info));
      device.checkSupport(window.surface());
      swap = std::move(ovk::Swapchain(device, window.surface(), info.vsync));
      return index;
    }
    index++;
  }

  OhmAssert(true,
            "Too many windows. API has run out of allocation cache space.");
  return -1;
}

auto Vulkan::Window::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Attempting to delete an invalid window handle.");
  auto& val = ovk::system().window[handle];
  auto& val2 = ovk::system().swapchain[handle];
  auto tmp = ovk::Window();
  auto tmp2 = ovk::Swapchain();

  OhmAssert(!val.initialized(),
            "Attempting to destroy a window object that is not initialized.");

  tmp = std::move(val);
  tmp2 = std::move(val2);
}

auto Vulkan::Window::count(int32_t handle) Ohm_NOEXCEPT -> size_t {
  OhmAssert(handle < 0, "Accessing invalid handle!");
  auto& swapchain = ovk::system().swapchain[handle];
  OhmAssert(!swapchain.initialized(), "Accessing invalid swapchain!");
  return swapchain.images().size();
}

auto Vulkan::Window::image(int32_t handle, size_t index) Ohm_NOEXCEPT
    -> int32_t {
  OhmAssert(handle < 0, "Accessing invalid handle!");
  auto& swapchain = ovk::system().swapchain[handle];
  OhmAssert(!swapchain.initialized(), "Accessing invalid swapchain!");
  return swapchain.images()[index];
}

auto Vulkan::Window::update(int32_t handle, const WindowInfo& info) Ohm_NOEXCEPT
    -> void {
  OhmAssert(handle < 0, "Accessing invalid handle!");
  auto& window = ovk::system().window[handle];
  OhmAssert(!window.initialized(), "Accessing invalid window!");
  window.update(info);
}

auto Vulkan::Window::has_focus(int32_t handle) Ohm_NOEXCEPT -> bool {
  OhmAssert(handle < 0, "Accessing invalid handle!");
  auto& window = ovk::system().window[handle];
  OhmAssert(!window.initialized(), "Accessing invalid window!");
  return window.has_focus();
}

auto Vulkan::Window::wait(int32_t handle, int32_t cmd) Ohm_NOEXCEPT -> void {
  OhmAssert(handle < 0, "Accessing invalid handle!");
  OhmAssert(cmd < 0, "Accessing invalid handle!");
  auto& swapchain = ovk::system().swapchain[handle];
  auto& buf = ovk::system().commands[cmd];
  OhmAssert(!swapchain.initialized(), "Accessing invalid window!");
  swapchain.wait(buf);
}

auto Vulkan::Window::present(int32_t handle) Ohm_NOEXCEPT -> bool {
  OhmAssert(handle < 0, "Accessing invalid handle!");
  auto& swapchain = ovk::system().swapchain[handle];
  OhmAssert(!swapchain.initialized(), "Accessing invalid swapchain!");
  if (!swapchain.present()) {
    auto device = &swapchain.device();
    auto surface = swapchain.surface();
    auto vsync = swapchain.vsync();
    auto dep = swapchain.dependency();
    dep->setDepended(false);
    ovk::error(device->device().waitIdle(device->dispatch()));
    { auto tmp = std::move(swapchain); }
    swapchain = std::move(ovk::Swapchain(*device, surface, vsync));
    if (dep) swapchain.wait(*dep);
    return false;
  }
  return true;
}
}  // namespace v1
}  // namespace ohm
