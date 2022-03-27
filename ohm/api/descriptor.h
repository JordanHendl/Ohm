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
  Descriptor(int32_t handle, const Pipeline<API>* parent);
  const Pipeline<API>* m_parent;
  int32_t m_handle;
};

template <typename API>
Descriptor<API>::Descriptor() {
  this->m_handle = -1;
  this->m_parent = nullptr;
}

template <typename API>
Descriptor<API>::Descriptor(Descriptor&& mv) {
  *this = std::move(mv);
}

template <typename API>
Descriptor<API>::Descriptor(int32_t handle, const Pipeline<API>* parent) {
  this->m_handle = handle;
  this->m_parent = parent;
}

template <typename API>
Descriptor<API>::~Descriptor() {
  API::Descriptor::destroy(this->m_handle);
  this->m_handle = -1;
  this->m_parent = nullptr;
}

template <typename API>
auto Descriptor<API>::operator=(Descriptor&& mv) -> Descriptor& {
  this->m_handle = mv.m_handle;
  this->m_parent = mv.m_parent;

  mv.m_handle = -1;
  mv.m_parent = nullptr;
}

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
                           const Array<API, Type, Allocator>& array) -> void {
  API::Descriptor::bind_array(this->m_handle, name, array.handle());
}

template <typename API>
template <typename Allocator>
auto Descriptor<API>::bind(std::string_view name,
                           const Image<API, Allocator>* images, size_t amt)
    -> void {
  auto tmp = std::vector<int32_t>();
  for (auto index = 0; index < amt; index++)
    tmp.push_back(images[index].handle());

  API::Descriptor::bind_images(this->m_handle, tmp);
}

template <typename API>
template <typename Allocator>
auto Descriptor<API>::bind(std::string_view name,
                           const Image<API, Allocator>& image) -> void {
  API::Descriptor::bind_image(this->m_handle, name, image.handle());
}
}  // namespace ohm