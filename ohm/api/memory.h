#pragma once
#include <vector>
#include <utility>
#include "allocators.h"
namespace ohm {
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
  auto gpu() const -> int;
  auto size() const -> size_t;
  auto type() const -> HeapType;
  auto handle() const -> int32_t;

 private:
  int m_gpu;
  int32_t m_handle;
};

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory() {
  this->m_gpu = 0;
  this->m_handle = -1;
}

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory(int gpu, size_t size) {
  auto& heaps = API::Memory::heaps(gpu);
  auto heap_index = Allocator::chooseHeap(gpu, heaps, HeapType::GpuOnly, size);

  this->m_gpu = gpu;
  this->m_handle =
      Allocator::allocate(gpu, HeapType::GpuOnly, heap_index, size);
}

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory(int gpu, HeapType type, size_t size) {
  auto& heaps = API::Memory::heaps(gpu);
  auto heap_index = Allocator::chooseHeap(gpu, heaps, type, size);

  this->m_gpu = gpu;
  this->m_handle = Allocator::allocate(gpu, type, heap_index, size);
}

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory(const Memory<API>& parent, size_t offset) {
  this->m_gpu = parent.m_gpu;
  this->m_handle = API::Memory::offset(parent.m_handle, offset);
}

template <typename API, typename Allocator>
Memory<API, Allocator>::Memory(Memory<API, Allocator>&& mv) {
  *this = std::move(mv);
}

template <typename API, typename Allocator>
Memory<API, Allocator>::~Memory() {
  if (this->m_handle >= 0) Allocator::destroy(this->m_handle);
}

template <typename API, typename Allocator>
auto Memory<API, Allocator>::operator=(Memory<API, Allocator>&& mv)
    -> Memory<API, Allocator>& {
  this->m_gpu = mv.m_gpu;
  this->m_handle = mv.m_handle;

  mv.m_handle = -1;
  mv.m_gpu = 0;

  return *this;
}

template <typename API, typename Allocator>
auto Memory<API, Allocator>::size() const -> size_t {
  return API::Memory::size(this->m_handle);
}

template <typename API, typename Allocator>
auto Memory<API, Allocator>::type() const -> HeapType {
  return API::Memory::type(this->m_handle);
}

template <typename API, typename Allocator>
auto Memory<API, Allocator>::gpu() const -> int {
  return this->m_gpu;
}

template <typename API, typename Allocator>
auto Memory<API, Allocator>::handle() const -> int32_t {
  return this->m_handle;
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