#pragma once
#include "array.h"
#include "image.h"
#include "memory.h"
namespace ohm {
template <typename API>
class Pipeline;

template <typename API>
class Descriptor {
 public:
  Descriptor();
  explicit Descriptor(Descriptor&& mv);
  ~Descriptor();
  auto operator=(Descriptor&& mv) -> Descriptor&;
  auto handle() const -> int32_t;
  auto parent() -> const Pipeline<API>*;
  template <typename Type, typename Allocator>
  auto bind(std::string_view name, const Array<API, Type, Allocator>& array)
      -> void;

  template <typename Allocator>
  auto bind(std::string_view name, const Image<API, Allocator>* images,
            size_t amt) -> void;

  template <typename Allocator>
  auto bind(std::string_view name, const Image<API, Allocator>& image) -> void;

 private:
  friend class Pipeline<API>;
  const Pipeline<API>* m_parent;
  int32_t m_handle;
};

template <typename API>
Descriptor<API>::Descriptor() {}

template <typename API>
Descriptor<API>::Descriptor(Descriptor&& mv) {}

template <typename API>
Descriptor<API>::~Descriptor() {}

template <typename API>
auto Descriptor<API>::operator=(Descriptor&& mv) -> Descriptor& {}

template <typename API>
auto Descriptor<API>::handle() const -> int32_t {
  return this->m_handle;
}

template <typename API>
auto Descriptor<API>::parent() -> const Pipeline<API>* {
  return this->m_parent;
}

template <typename API>
template <typename Type, typename Allocator>
auto Descriptor<API>::bind(std::string_view name,
                           const Array<API, Type, Allocator>& array) -> void {}

template <typename API>
template <typename Allocator>
auto Descriptor<API>::bind(std::string_view name,
                           const Image<API, Allocator>* images, size_t amt)
    -> void {}

template <typename API>
template <typename Allocator>
auto Descriptor<API>::bind(std::string_view name,
                           const Image<API, Allocator>& image) -> void {}
}  // namespace ohm