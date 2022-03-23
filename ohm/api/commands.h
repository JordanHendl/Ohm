#pragma once
#include "array.h"
namespace ohm {

template <typename API>
class Descriptor;

template <typename API, typename Allocator>
class RenderPass;

enum class QueueType : int {
  Graphics,
  Compute,
  Transfer,
};

enum class Filter : int {
  Linear,
  Nearest,
  Cubic,
};

template <typename API, QueueType Queue = QueueType::Graphics>
class Commands {
 public:
  explicit Commands();
  explicit Commands(int device);
  explicit Commands(Commands<API, Queue>& parent);
  explicit Commands(Commands<API, Queue>&& mv);
  ~Commands();
  template <typename Allocator>
  auto attach(const RenderPass<API, Allocator>& pass) -> void;

  template <typename Type1, typename Type2>
  auto blit(const Type1& src, Type2& dst, Filter filter) -> void;

  template <typename Type, typename Allocator>
  auto copy(const Array<API, Type, Allocator>& src,
            Array<API, Type, Allocator>& dst, size_t count = 0) -> void;

  template <typename Type, typename Allocator>
  auto copy(const Type* src, Array<API, Type, Allocator>& dst, size_t count = 0)
      -> void;

  template <typename Type, typename Allocator>
  auto copy(const Array<API, Type, Allocator>& src, Type* dst, size_t count = 0)
      -> void;

  template <typename Type, typename Type2, typename Allocator>
  auto draw(const Array<API, Type, Allocator>& indices,
            const Array<API, Type2, Allocator>& vertices,
            size_t instance_count = 1) -> void;

  template <typename Type, typename Allocator>
  auto draw(const Array<API, Type, Allocator>& vertices,
            size_t instance_count = 1) -> void;

  auto dispatch(size_t x, size_t y, size_t z = 1) -> void;
  auto detach() -> void;
  auto combine(const Commands& child) -> void;
  auto operator=(Commands<API, Queue>& cpy) = delete;
  auto operator=(Commands<API, Queue>&& mv);
  auto wait(const Commands<API, Queue>& cmds);
  auto begin() -> void;
  auto end() -> void;
  auto bind(const Descriptor<API>& desc);
  auto handle() const -> int32_t;
  auto gpu() const -> int;
  auto synchronize() -> void;
  auto submit() -> void;

 private:
  int m_gpu;
  int32_t m_handle;
};

template <typename API, QueueType Queue>
Commands<API, Queue>::Commands() {
  this->m_handle = -1;
  this->m_gpu = -1;
}

template <typename API, QueueType Queue>
Commands<API, Queue>::Commands(int gpu) {
  this->m_handle = API::Commands::create(gpu, Queue);
  this->m_gpu = gpu;
}

template <typename API, QueueType Queue>
Commands<API, Queue>::Commands(Commands<API, Queue>& parent) {
  this->m_handle = API::Commands::create(parent.handle());
  this->m_gpu = parent.m_gpu;
}

template <typename API, QueueType Queue>
Commands<API, Queue>::Commands(Commands<API, Queue>&& mv) {
  this->m_handle = mv.m_handle;
  this->m_gpu = mv.m_gpu;
  mv.m_handle = -1;
  mv.m_gpu = -1;
}

template <typename API, QueueType Queue>
Commands<API, Queue>::~Commands() {
  if (this->m_handle >= 0) {
    API::Commands::destroy(this->m_handle);
    this->m_handle = -1;
    this->m_gpu = -1;
  }
}

template <typename API, QueueType Queue>
template <typename Allocator>
auto Commands<API, Queue>::attach(const RenderPass<API, Allocator>& pass)
    -> void {}

template <typename API, QueueType Queue>
template <typename Type1, typename Type2>
auto Commands<API, Queue>::blit(const Type1& src, Type2& dst, Filter filter)
    -> void {
  API::Commands::blit(this->m_handle, src, dst, filter);
}
template <typename API, QueueType Queue>
template <typename Type, typename Allocator>
auto Commands<API, Queue>::copy(const Array<API, Type, Allocator>& src,
                                Array<API, Type, Allocator>& dst, size_t count)
    -> void {
  API::Commands::copyArray(this->m_handle, src.handle(), dst.handle(), count);
}

template <typename API, QueueType Queue>
template <typename Type, typename Allocator>
auto Commands<API, Queue>::copy(const Array<API, Type, Allocator>& src,
                                Type* dst, size_t count) -> void {
  API::Commands::copyArray(this->m_handle, src.handle(),
                           static_cast<void*>(dst), count);
}

template <typename API, QueueType Queue>
template <typename Type, typename Allocator>
auto Commands<API, Queue>::copy(const Type* src,
                                Array<API, Type, Allocator>& dst, size_t count)
    -> void {
  API::Commands::copyArray(this->m_handle, static_cast<const void*>(src),
                           dst.handle(), count);
}

template <typename API, QueueType Queue>
template <typename Type, typename Type2, typename Allocator>
auto Commands<API, Queue>::draw(const Array<API, Type, Allocator>& indices,
                                const Array<API, Type2, Allocator>& vertices,
                                size_t instance_count) -> void {}

template <typename API, QueueType Queue>
template <typename Type, typename Allocator>
auto Commands<API, Queue>::draw(const Array<API, Type, Allocator>& vertices,
                                size_t instance_count) -> void {}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::dispatch(size_t x, size_t y, size_t z) -> void {
  API::Commands::dispatch(this->m_handle, x, y, z);
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::detach() -> void {
  API::Commands::detatch(this->m_handle);
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::combine(const Commands& child) -> void {
  API::Commands::combine(this->m_handle, child.m_handle);
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::operator=(Commands<API, Queue>&& mv) {
  this->m_handle = mv.m_handle;
  mv.m_handle = -1;
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::wait(const Commands<API, Queue>& cmds) {
  API::Commands::wait(this->m_handle, cmds.m_handle);
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::begin() -> void {
  API::Commands::begin(this->m_handle);
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::end() -> void {
  API::Commands::end(this->m_handle);
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::gpu() const -> int {
  return this->m_gpu;
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::bind(const Descriptor<API>& desc) {}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::handle() const -> int32_t {
  return this->m_handle;
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::synchronize() -> void {
  API::Commands::synchronize(this->m_handle);
}

template <typename API, QueueType Queue>
auto Commands<API, Queue>::submit() -> void {
  API::Commands::submit(this->m_handle);
}
}  // namespace ohm
