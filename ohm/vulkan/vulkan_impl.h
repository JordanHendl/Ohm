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
    static auto copyArray(int32_t handle, int32_t src, int32_t dst,
                          size_t count) Ohm_NOEXCEPT -> void;
    static auto copyArray(int32_t handle, int32_t src, void* dst,
                          size_t count) Ohm_NOEXCEPT -> void;
    static auto copyArray(int32_t handle, const void* src, int32_t dst,
                          size_t count) Ohm_NOEXCEPT -> void;
    static auto submit(int32_t handle) Ohm_NOEXCEPT -> void;
    static auto synchronize(int32_t handle) Ohm_NOEXCEPT -> void;
  };

  struct RenderPass {
    static auto create(int gpu, const RenderPassInfo& info) Ohm_NOEXCEPT
        -> int32_t;
    static auto destroy(int32_t handle) Ohm_NOEXCEPT -> void;
  };

  /** Commands-related function API
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
};
}  // namespace v1
}  // namespace ohm