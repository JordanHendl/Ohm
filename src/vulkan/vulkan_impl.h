#pragma once
#include <string>
#include <vector>

namespace ohm {
struct Gpu;
struct GpuMemoryHeap;
enum class HeapType : int;

struct Vulkan {
  
  /** System-related function API
   */
  struct System {
    static auto initialize() -> void;
    static auto name() -> std::string;
    static auto setParameter(std::string_view str) -> void;
    static auto setDebugParameter(std::string_view str) -> void;
    static auto devices() -> std::vector<Gpu>;
  };
  
  /** Memory-related function API
   */
  struct Memory {
    static auto heaps(int gpu) -> const std::vector<GpuMemoryHeap>&;
    static auto allocate(int gpu, HeapType type, size_t heap_index, size_t size)
        -> int32_t;
    static auto destroy(int32_t handle) -> void;
    static auto type(int32_t handle) -> HeapType;
    static auto size(int32_t handle) -> size_t;
    static auto offset(int32_t handle, size_t offset) -> size_t;
  };
  
  /** Buffer-related function API
   */
};
}  // namespace ohm