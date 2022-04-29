#pragma once
#include <string>
#include <vector>
#include "ohm/api/exception.h"
#include "ohm/api/ohm.h"

namespace ohm {
struct Gpu;
struct GpuMemoryHeap;
enum class HeapType : int;
enum class QueueType : int;
inline namespace v1 {
struct Vulkan {
  /** System-related function API
   */
  struct System {
    static auto initialize() Ohm_NOEXCEPT -> void;
    static auto shutdown() Ohm_NOEXCEPT -> void;
    static auto name() Ohm_NOEXCEPT -> std::string;
    static auto set_parameter(std::string_view str) Ohm_NOEXCEPT -> void;
    static auto set_debug_parameter(std::string_view str) Ohm_NOEXCEPT -> void;
    static auto devices() Ohm_NOEXCEPT -> std::vector<Gpu>;
  };

  /** Memory-related function API
   */
  struct Memory {
    static auto heaps(int gpu) Ohm_NOEXCEPT
        -> const std::vector<GpuMemoryHeap>&;
    static auto allocate(int gpu, HeapType type, size_t heap_index,
                         size_t size) Ohm_NOEXCEPT -> int32_t;
    static auto destroy(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto type(int32_t handle) Ohm_NOEXCEPT -> HeapType;
    static auto size(int32_t handle) Ohm_NOEXCEPT -> size_t;
    static auto offset(int32_t handle, size_t offset) Ohm_NOEXCEPT -> size_t;
  };

  /** Array-related function API
   */
  struct Array {
    static auto create(int gpu, size_t num_elmts, size_t elm_size) Ohm_NOEXCEPT
        -> int32_t;
    static auto destroy(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto required(int32_t handle) Ohm_NOEXCEPT -> size_t;
    static auto bind(int32_t array_handle, int32_t memory_handle) Ohm_NOEXCEPT
        -> void;
  };

  /** Image-related function API
   */
  struct Image {
    static auto create(int gpu, const ImageInfo& info) Ohm_NOEXCEPT -> int32_t;
    static auto destroy(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto layer(int32_t handle, size_t layer) Ohm_NOEXCEPT -> int32_t;
    static auto required(int32_t handle) Ohm_NOEXCEPT -> size_t;
    static auto bind(int32_t image_handle, int32_t mem_handle) Ohm_NOEXCEPT
        -> void;
  };

  /** Commands-related function API
   */
  struct Commands {
    static auto create(int gpu, QueueType type) Ohm_NOEXCEPT -> int32_t;
    static auto destroy(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto begin(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto bind(int32_t handle, int32_t desc) Ohm_NOEXCEPT -> void;
    static auto copy_to_image(int32_t handle, int32_t src, int32_t dst,
                              size_t count) Ohm_NOEXCEPT -> void;
    static auto copy_image(int32_t handle, int32_t src, int32_t dst,
                           size_t count) Ohm_NOEXCEPT -> void;
    static auto copy_array(int32_t handle, int32_t src, int32_t dst,
                           size_t count) Ohm_NOEXCEPT -> void;
    static auto copy_array(int32_t handle, int32_t src, void* dst,
                           size_t count) Ohm_NOEXCEPT -> void;
    static auto copy_array(int32_t handle, const void* src, int32_t dst,
                           size_t count) Ohm_NOEXCEPT -> void;
    static auto dispatch(int32_t handle, size_t x, size_t y,
                         size_t z) Ohm_NOEXCEPT -> void;
    static auto submit(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto blit_to_window(int32_t handle, int32_t src, int32_t dst,
                               Filter filter) Ohm_NOEXCEPT -> void;
    static auto blit_to_image(int32_t handle, int32_t src, int32_t dst,
                              Filter filter) Ohm_NOEXCEPT -> void;
    static auto blit_from_renderpass(int32_t handle, int32_t src, int32_t dst,
                                     Filter filter) Ohm_NOEXCEPT -> void;
    static auto synchronize(int32_t handle) Ohm_NOEXCEPT -> void;
  };

  struct RenderPass {
    static auto create(int gpu, const RenderPassInfo& info) Ohm_NOEXCEPT
        -> int32_t;
    static auto destroy(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto count(int32_t handle) Ohm_NOEXCEPT -> size_t;
    static auto image(int32_t handle, size_t index) -> int32_t;
  };

  /** Pipeline-related function API
   */
  struct Pipeline {
    static auto create(int gpu, const PipelineInfo& info) Ohm_NOEXCEPT
        -> int32_t;
    static auto create_from_rp(int32_t rp_handle,
                               const PipelineInfo& info) Ohm_NOEXCEPT
        -> int32_t;
    static auto destroy(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto descriptor(int32_t handle) Ohm_NOEXCEPT -> int32_t;
  };

  /** Descriptor-related function API
   */
  struct Descriptor {
    static auto destroy(int32_t handle) -> void;
    static auto bind_array(int32_t handle, std::string_view name, int32_t array)
        -> void;
    static auto bind_image(int32_t handle, std::string_view name, int32_t image)
        -> void;
    static auto bind_images(int32_t handle, std::string_view name,
                            const std::vector<int32_t>& images) -> void;
  };

  /** Event-related function API
   */
  struct Event {
    static auto create() Ohm_NOEXCEPT -> int32_t;
    static auto destroy(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto add(int32_t handle, std::function<void(const ohm::Event&)> cb)
        -> void;
    static auto poll() -> void;
  };

  /** Window-related function API
   */
  struct Window {
    static auto create(int gpu, const WindowInfo& info) Ohm_NOEXCEPT -> int32_t;
    static auto count(int32_t handle) Ohm_NOEXCEPT -> size_t;
    static auto image(int32_t handle, size_t index) Ohm_NOEXCEPT -> int32_t;
    static auto update(int32_t handle, const WindowInfo& info) Ohm_NOEXCEPT
        -> void;
    static auto wait(int32_t handle, int32_t cmd) Ohm_NOEXCEPT -> void;
    static auto present(int32_t handle) Ohm_NOEXCEPT -> bool;
    static auto has_focus(int32_t handle) Ohm_NOEXCEPT -> bool;
    static auto destroy(int32_t handle) Ohm_NOEXCEPT -> void;
  };
};
}  // namespace v1
}  // namespace ohm