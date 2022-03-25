#pragma once
#include <string>
#include <vector>
#include "allocators.h"
#include "commands.h"
#include "event.h"
#include "image.h"
namespace ohm {

struct WindowInfo {
  std::string title;
  size_t width;
  size_t height;
  bool borderless;
  bool fullscreen;
  bool resizable;
  bool shown;
  bool capture_mouse;

  WindowInfo() {
    this->title = "Ohm";
    this->width = 1280;
    this->height = 1024;
    this->borderless = false;
    this->fullscreen = false;
    this->resizable = false;
    this->shown = true;
    this->capture_mouse = false;
  }
};

template <typename API, typename Allocator = DefaultAllocator<API>>
class Window {
 public:
  Window();
  explicit Window(int gpu, WindowInfo info);
  explicit Window(Window&& mv);
  ~Window();
  auto operator=(Window&& mv) -> Window&;
  auto update(WindowInfo info) -> void;
  auto gpu() const -> int;
  auto info() const -> const WindowInfo&;
  auto image(size_t index) -> Image<API, Allocator>&;
  auto image(size_t index) const -> const Image<API, Allocator>&;
  auto images() const -> const std::vector<Image<API, Allocator>>&;
  auto current() const -> size_t;
  auto wait(const Commands<API>& cmds) -> void;
  auto present() -> void;
  auto subscribe(EventCallback callback, std::string_view key) -> void;
  auto subscribe(EventCallback callback, Event::Type type, std::string_view key)
      -> void;
  auto subscribe(EventCallback callback, Key keysym, std::string_view key)
      -> void;
  template <class Object>
  auto subscribe(Object* obj, void (Object::*callback)(const Event&),
                 std::string_view key) -> void;
  template <class Object>
  auto subscribe(Object* obj, void (Object::*callback)(const Event&),
                 Event::Type type, std::string_view key) -> void;
  template <class Object>
  auto subscribe(Object* obj, void (Object::*callback)(const Event&),
                 Key keysym, std::string_view key) -> void;

 private:
  int m_gpu;
  WindowInfo m_info;
  std::vector<Image<API, Allocator>> m_images;
  int32_t m_hande;
};

template <typename API, typename Allocator>
Window<API, Allocator>::Window(int gpu, WindowInfo info) {}

template <typename API, typename Allocator>
Window<API, Allocator>::Window(Window&& mv) {}

template <typename API, typename Allocator>
Window<API, Allocator>::~Window() {}

template <typename API, typename Allocator>
auto Window<API, Allocator>::operator=(Window&& mv) -> Window& {
  return *this;
}

template <typename API, typename Allocator>
auto Window<API, Allocator>::update(WindowInfo info) -> void {}

template <typename API, typename Allocator>
auto Window<API, Allocator>::gpu() const -> int {
  return this->m_gpu;
}

template <typename API, typename Allocator>
auto Window<API, Allocator>::info() const -> const WindowInfo& {
  return this->m_info;
}

template <typename API, typename Allocator>
auto Window<API, Allocator>::image(size_t index) -> Image<API, Allocator>& {
  return this->m_images[0];
}

template <typename API, typename Allocator>
auto Window<API, Allocator>::image(size_t index) const
    -> const Image<API, Allocator>& {
  return this->m_images[0];
}

template <typename API, typename Allocator>
auto Window<API, Allocator>::images() const
    -> const std::vector<Image<API, Allocator>>& {
  return this->m_images;
}

template <typename API, typename Allocator>
auto Window<API, Allocator>::current() const -> size_t {
  return 0;
}

template <typename API, typename Allocator>
auto Window<API, Allocator>::wait(const Commands<API>& cmds) -> void {}

template <typename API, typename Allocator>
auto Window<API, Allocator>::present() -> void {}

template <typename API, typename Allocator>
auto Window<API, Allocator>::subscribe(EventCallback callback,
                                       std::string_view key) -> void {}

template <typename API, typename Allocator>
auto Window<API, Allocator>::subscribe(EventCallback callback, Event::Type type,
                                       std::string_view key) -> void {}

template <typename API, typename Allocator>
auto Window<API, Allocator>::subscribe(EventCallback callback, Key keysym,
                                       std::string_view key) -> void {}

template <typename API, typename Allocator>
template <class Object>
auto Window<API, Allocator>::subscribe(Object* obj,
                                       void (Object::*callback)(const Event&),
                                       std::string_view key) -> void {}

template <typename API, typename Allocator>
template <class Object>
auto Window<API, Allocator>::subscribe(Object* obj,
                                       void (Object::*callback)(const Event&),
                                       Event::Type type, std::string_view key)
    -> void {}

template <typename API, typename Allocator>
template <class Object>
auto Window<API, Allocator>::subscribe(Object* obj,
                                       void (Object::*callback)(const Event&),
                                       Key keysym, std::string_view key)
    -> void {}
}  // namespace ohm