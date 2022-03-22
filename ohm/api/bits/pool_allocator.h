#pragma once
#include <mutex>
#include <vector>
#include "../exception.h"
#include "../memory.h"

namespace ohm {

/** Pool memory allocator.
 */
template <typename API>
struct PoolAllocator {
  inline static auto chooseHeap(int gpu, const std::vector<GpuMemoryHeap>& heap,
                                HeapType requested, size_t size) -> int;

  inline static auto allocate(int gpu, HeapType type, int heap_index,
                              size_t size) -> int32_t;

  inline static auto destroy(int32_t handle) -> void;

 private:
  struct Block {
    bool occupied = false;
    int32_t occupied_handle = -1;
  };

  struct Heap {
    GpuMemoryHeap heap;
    int32_t id = -1;
    std::vector<Block> blocks;
  };

  struct PoolAllocatorData {
    std::vector<Heap> heaps;
    int32_t block_size = 2048;
    int32_t requested_memory = 1 << 26;
    std::mutex mutex;
  };

  static PoolAllocatorData data;
};

template <typename API>
typename PoolAllocator<API>::PoolAllocatorData PoolAllocator<API>::data;

template <typename API>
auto PoolAllocator<API>::chooseHeap(int gpu,
                                    const std::vector<GpuMemoryHeap>& heaps,
                                    HeapType requested, size_t size) -> int {
  using alloc = PoolAllocator<API>;
  const auto num_blocks = alloc::data.requested_memory / alloc::data.block_size;
  auto index = 0;

  std::unique_lock<std::mutex> lock(alloc::data.mutex);

  if (alloc::data.heaps.empty()) {
    alloc::data.heaps.resize(heaps.size());
    for (auto& heap : heaps) {
      auto& tmp = alloc::data.heaps[index];
      tmp.heap = heap;
      tmp.blocks.resize(num_blocks);
      tmp.id = API::Memory::allocate(gpu, heap.type, index,
                                     alloc::data.requested_memory);
      index++;
    }
  }

  index = 0;
  for (auto& heap : heaps) {
    auto type_match = heap.type & requested;
    auto size_ok = size <= heap.size;
    if (type_match && size_ok) {
      return index;
    }
    index++;
  }

  OhmException(true, Error::LogicError,
               "Could not find a valid memory heap to allocate from.");
  return -1;
}

template <typename API>
auto PoolAllocator<API>::allocate(int gpu, HeapType type, int heap_index,
                                  size_t size) -> int32_t {
  using alloc = PoolAllocator<API>;
  auto& heap = alloc::data.heaps[heap_index];

  auto index = 0;
  auto start_block = -1;
  auto found_size = 0u;
  // Search blocks and find a contiguous block chunk that meets memory needs.
  std::unique_lock<std::mutex> lock(alloc::data.mutex);
  for (auto& block : heap.blocks) {
    if (!block.occupied) {
      start_block = start_block < 0 ? index : start_block;
      found_size += alloc::data.block_size;

      if (found_size >= size) {
        const auto num_blocks =
            static_cast<int>(found_size / alloc::data.block_size);
        const auto offset = (start_block + 1) * alloc::data.block_size;
        auto handle = API::Memory::offset(heap.id, offset);

        // Found a chunk and created the offset, but need to claim all the
        // blocks we found.
        for (auto index = start_block; index < num_blocks; ++index) {
          auto& block = heap.blocks[index];

          block.occupied = true;
          block.occupied_handle = handle;
        }

        return handle;
      }
    } else {
      found_size = 0;
      start_block = -1;
    }
  }

  OhmException(true, Error::LogicError,
               "Memory too fragmented, could not allocate.");
  return -1;
}

template <typename API>
auto PoolAllocator<API>::destroy(int32_t handle) -> void {
  using alloc = PoolAllocator<API>;
  auto found_heap = false;
  for (auto& heap : alloc::data.heaps) {
    for (auto& block : heap.blocks) {
      if (block.occupied_handle == handle) {
        block.occupied = false;
        block.occupied_handle = -1;
        found_heap = true;
      }

      if (found_heap && handle >= 0) {
        API::Memory::destroy(handle);
        return;
      }
    }
  }
}
}  // namespace ohm