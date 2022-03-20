#pragma once
#include <vector>

namespace ohm {
enum class HeapType : int {
  GpuOnly = 1 << 1,
  HostVisible = 1 << 2,
  LazyAllocation = 1 << 3,
};

struct GpuMemoryHeap {
  HeapType type = HeapType::GpuOnly;
  size_t size = 0;
};

/** Default memory heap selector.
 */
template <typename API>
struct DefaultAllocator {
  /** Function to choose which memory heap the allocation should pull from.
   * Default implementation is very naiive, but can be overloaded to be better
   * if desired.
   */
  inline static auto chooseHeap(int gpu, const std::vector<GpuMemoryHeap>& heap,
                                HeapType requested, size_t size) -> int;

  /** Function to actually allocate memory. Can be overloaded to do memory
   * pooling using offsets, but default is just to actually request memory from
   * the GPU on every allocation.
   */
  inline static auto allocate(int gpu, HeapType type, int heap_index,
                              size_t size) -> int32_t;

  /** Function to handle deleting memory. Default behaviour is to just release
   * it back to the GPU, however can be overloaded when implementing custom
   * allocators.
   */
  inline static auto destroy(int32_t handle) -> void;
};

template <typename API, typename Allocator = DefaultAllocator<API>>
class Memory {
 public:
  explicit Memory();
  explicit Memory(int gpu, size_t size);
  explicit Memory(int gpu, HeapType type, size_t size);
  explicit Memory(const Memory<API>& parent, size_t offset);
  explicit Memory(Memory&& mv);
  Memory(const Memory& cpy) = delete;
  ~Memory();
  auto operator=(Memory&& mv) -> Memory&;
  auto operator=(const Memory& mv) -> Memory& = delete;
  auto gpu() -> int;
  auto size() -> size_t;
  auto type() -> HeapType;

 private:
  int m_gpu;
  int32_t handle;
};

inline auto operator|(const HeapType& a, const HeapType& b) -> HeapType {
  return static_cast<HeapType>(static_cast<int>(a) | static_cast<int>(b));
}

inline auto operator|=(const HeapType& a, const HeapType& b) -> HeapType {
  return static_cast<HeapType>(static_cast<int>(a) | static_cast<int>(b));
}

inline auto operator&(const HeapType& a, const HeapType& b) -> bool {
  return static_cast<HeapType>(static_cast<int>(a) & static_cast<int>(b)) == b;
}

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory() {
  this->m_gpu = 0;
  this->handle = -1;
}

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory(int gpu, size_t size) {
  auto& heaps = API::Memory::heaps(gpu);
  auto heap_index = Allocator::chooseHeap(gpu, heaps, HeapType::GpuOnly, size);

  this->m_gpu = gpu;
  this->handle = Allocator::allocate(gpu, HeapType::GpuOnly, heap_index, size);
}

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory(int gpu, HeapType type, size_t size) {
  auto& heaps = API::Memory::heaps(gpu);
  auto heap_index = Allocator::chooseHeap(gpu, heaps, type, size);

  this->m_gpu = gpu;
  this->handle = Allocator::allocate(gpu, type, heap_index, size);
}

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory(const Memory<API>& parent, size_t offset) {
  this->m_gpu = parent.m_gpu;
  this->handle = API::Memory::offset(parent.handle, offset);
}

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory(Memory<API, Allocator>&& mv) {
  *this = mv;
}

template <typename API, typename Allocator>
Memory<API, Allocator>::~Memory() {
  if (this->handle >= 0) Allocator::destroy(this->handle);
}

template <typename API, typename Allocator>
auto Memory<API, Allocator>::operator=(Memory<API, Allocator>&& mv)
    -> Memory<API, Allocator>& {
  this->m_gpu = mv.m_gpu;
  this->handle = mv.handle;

  mv.handle = -1;
  mv.m_gpu = 0;
}

template <typename API, typename Allocator>
auto Memory<API, Allocator>::size() -> size_t {
  return API::Memory::size(this->handle);
}

template <typename API, typename Allocator>
auto Memory<API, Allocator>::type() -> HeapType {
  return API::Memory::type(this->handle);
}

template <typename API, typename Allocator>
auto Memory<API, Allocator>::gpu() -> int {
  return this->m_gpu;
}

template <typename API>
auto DefaultAllocator<API>::chooseHeap(int gpu,
                                       const std::vector<GpuMemoryHeap>& heap,
                                       HeapType requested, size_t size) -> int {
  auto index = 0;
  for (auto& heap : heap) {
    auto type_match = heap.type & requested;
    auto size_ok = size <= heap.size;
    if (type_match && size_ok) {
      return index;
    }
    index++;
  }

  return 0;
}

template <typename API>
auto DefaultAllocator<API>::allocate(int gpu, HeapType type, int heap_index,
                                     size_t size) -> int32_t {
  return API::Memory::allocate(gpu, type, heap_index, size);
}

template <typename API>
auto DefaultAllocator<API>::destroy(int32_t handle) -> void {
  API::Memory::destroy(handle);
}
}  // namespace ohm

/** Required functions of API
 * Memory::heaps(gpu) -> vector<GpuMemoryHeap>
 * Memory::allocate(gpu, heap_index, size) -> handle
 * Memory::destroy(handle) -> void.
 * Memory::type(handle) -> HeapType
 * Memory::offset(handle, offset) -> handle
 * Memory::size(handle) -> size_t
 */