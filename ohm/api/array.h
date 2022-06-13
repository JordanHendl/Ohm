#pragma once
#include <memory>
#include <utility>
#include "exception.h"
#include "memory.h"
namespace ohm {

/** Object to encapsulate a typed memory array.
 */
template <typename API, typename Type, class Allocator = DefaultAllocator<API>>
class Array {
 public:
  Array();
  explicit Array(int gpu, size_t count);
  explicit Array(Memory<API, Allocator>&& mv, size_t count);
  explicit Array(int gpu, size_t count, HeapType type);
  explicit Array(Array&& mv);
  ~Array();
  auto operator=(Array&& mv) -> Array&;
  auto memory() const -> const Memory<API, Allocator>&;
  auto size() const -> size_t;
  auto capacity() const -> size_t;
  auto byte_size() const -> size_t;
  auto handle() const -> int32_t;

 private:
  int32_t m_handle;
  size_t m_count;
  Memory<API, Allocator> m_memory;
};

template <typename API, typename Type, class Allocator>
Array<API, Type, Allocator>::Array() {
  this->m_handle = -1;
  this->m_count = 0;
}

template <typename API, typename Type, class Allocator>
Array<API, Type, Allocator>::Array(int gpu, size_t count) {
  this->m_count = count;
  this->m_handle = API::Array::create(gpu, count, sizeof(Type));

  auto buffer_required_size = API::Array::required(this->m_handle);
  this->m_memory = std::move(Memory<API, Allocator>(gpu, buffer_required_size));

  API::Array::bind(this->m_handle, m_memory.handle());
}

template <typename API, typename Type, class Allocator>
Array<API, Type, Allocator>::Array(Memory<API, Allocator>&& mv, size_t count) {
  this->m_count = count;
  this->m_handle = API::Array::create(mv.gpu(), count, sizeof(Type));

  auto buffer_required_size = API::Array::required(this->m_handle);
  auto enough_memory = buffer_required_size <= mv.size();

  OhmException(!enough_memory, Error::LogicError,
               "Attempting to create a buffer from memory that has not "
               "allocated enough.");
  if (enough_memory) {
    this->m_memory = std::move(mv);
    API::Array::bind(this->m_handle, this->m_memory.handle());
  } else {
    API::Array::destroy(this->m_handle);
    this->m_handle = -1;
    this->m_count = 0;
  }
}

template <typename API, typename Type, class Allocator>
Array<API, Type, Allocator>::Array(int gpu, size_t count, HeapType type) {
  this->m_count = count;
  this->m_handle = API::Array::create(gpu, count, sizeof(Type));

  auto buffer_required_size = API::Array::required(this->m_handle);
  this->m_memory = Memory<API, Allocator>(gpu, type, buffer_required_size);

  API::Array::bind(this->m_handle, m_memory.handle());
}

template <typename API, typename Type, class Allocator>
Array<API, Type, Allocator>::Array(Array&& mv) {
  *this = std::move(mv);
}

template <typename API, typename Type, class Allocator>
Array<API, Type, Allocator>::~Array() {
  if (this->m_handle >= 0) {
    API::Array::destroy(this->m_handle);
    this->m_handle = -1;
    this->m_count = 0;
  }
}

template <typename API, typename Type, class Allocator>
auto Array<API, Type, Allocator>::operator=(Array<API, Type, Allocator>&& mv)
    -> Array<API, Type, Allocator>& {
  this->m_handle = mv.m_handle;
  this->m_count = mv.m_count;
  this->m_memory = std::move(mv.m_memory);

  mv.m_handle = -1;
  mv.m_count = 0;
  return *this;
}

template <typename API, typename Type, class Allocator>
auto Array<API, Type, Allocator>::memory() const
    -> const Memory<API, Allocator>& {
  return this->m_memory;
}

template <typename API, typename Type, class Allocator>
auto Array<API, Type, Allocator>::size() const -> size_t {
  return this->m_count;
}

template <typename API, typename Type, class Allocator>
auto Array<API, Type, Allocator>::capacity() const -> size_t {
  return this->byte_size() / sizeof(Type);
}

template <typename API, typename Type, class Allocator>
auto Array<API, Type, Allocator>::byte_size() const -> size_t {
  return this->m_memory.size();
}

template <typename API, typename Type, class Allocator>
auto Array<API, Type, Allocator>::handle() const -> int32_t {
  return this->m_handle;
}
}  // namespace ohm

/** Required functions of API
 * Array::create(gpu, num_elements, size_of_element) -> handle
 * Array::destroy(array_handle) -> void.
 * Array::required(array_handle) -> size_t (of required memory to bind to
 * buffer) Array::bind(array_handle, memory_handle) -> void
 */
