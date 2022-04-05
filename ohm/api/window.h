#pragma once
#include <string>
#include <vector>
#include "allocators.h"
#include "commands.h"
#include "event.h"
#include "image.h"
namespace ohm {
template <typename API>
class Window;

struct WindowInfo {
  std::string title;
  size_t width;
  size_t height;
  bool borderless;
  bool fullscreen;
  bool resizable;
  bool shown;
  bool capture_mouse;
  bool vsync;

  WindowInfo(std::string_view name, size_t width = 1280, size_t height = 1024,
             bool resizable = false) {
    this->title = name;
    this->width = width;
    this->height = height;
    this->resizable = resizable;
    this->borderless = false;
    this->fullscreen = false;
    this->shown = true;
    this->capture_mouse = false;
    this->vsync = false;
  }

  WindowInfo(size_t width, size_t height) {
    this->title = "Window";
    this->width = width;
    this->height = height;
    this->borderless = false;
    this->fullscreen = false;
    this->resizable = false;
    this->shown = true;
    this->capture_mouse = false;
    this->vsync = false;
  }

  WindowInfo() {
    this->title = "Window";
    this->width = 1280;
    this->height = 1024;
    this->borderless = false;
    this->fullscreen = false;
    this->resizable = false;
    this->shown = true;
    this->capture_mouse = false;
    this->vsync = false;
  }
};

template <typename API>
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
  auto image(size_t index) -> Image<API>&;
  auto image(size_t index) const -> const Image<API>&;
  auto images() const -> const std::vector<Image<API>>&;
  auto current() -> Image<API>&;
  auto wait(const Commands<API>& cmds) -> void;
  auto handle() const -> int32_t;
  auto present() -> bool;

 private:
  int m_gpu;
  WindowInfo m_info;
  std::vector<Image<API>> m_images;
  int32_t m_handle;
};

template <typename API>
Window<API>::Window(int gpu, WindowInfo info) {
  this->m_gpu = gpu;
  this->m_info = info;
  this->m_handle = API::Window::create(gpu, info);
  this->m_images.resize(API::Window::count(this->m_handle));

  auto index = 0u;
  for (auto& img : this->m_images) {
    img.m_handle = API::Window::image(this->m_handle, index++);
    img.m_info.width = info.width;
    img.m_info.height = info.height;
  }
}

template <typename API>
Window<API>::Window(Window&& mv) {
  *this = std::move(mv);
}

template <typename API>
Window<API>::~Window() {
  if (this->m_handle >= 0) {
    API::Window::destroy(this->m_handle);
    for (auto& img : this->m_images) {
      img.m_handle = -1;
    }
    this->m_gpu = -1;
    this->m_handle = -1;
  }
}

template <typename API>
auto Window<API>::operator=(Window&& mv) -> Window& {
  this->m_info = mv.m_info;
  this->m_gpu = mv.m_gpu;
  this->m_handle = mv.m_handle;
  this->m_channels = std::move(mv.m_channels);
  this->m_images = std::move(mv.m_images);

  mv.m_gpu = -1;
  mv.m_handle = -1;
  mv.m_info = {};
  return *this;
}

template <typename API>
auto Window<API>::update(WindowInfo info) -> void {
  API::Window::update(this->m_handle, info);
}

template <typename API>
auto Window<API>::gpu() const -> int {
  return this->m_gpu;
}

template <typename API>
auto Window<API>::info() const -> const WindowInfo& {
  return this->m_info;
}

template <typename API>
auto Window<API>::image(size_t index) -> Image<API>& {
  return this->m_images[index];
}

template <typename API>
auto Window<API>::image(size_t index) const -> const Image<API>& {
  return this->m_images[index];
}

template <typename API>
auto Window<API>::images() const -> const std::vector<Image<API>>& {
  return this->m_images;
}

template <typename API>
auto Window<API>::current() -> Image<API>& {
  auto curr = API::Window::current(this->m_handle);
  return this->m_images[curr];
}

template <typename API>
auto Window<API>::wait(const Commands<API>& cmds) -> void {
  API::Window::wait(this->m_handle, cmds.handle());
}

template <typename API>
auto Window<API>::present() -> bool {
  if (!API::Window::present(this->m_handle)) {
    auto index = 0u;
    for (auto& img : this->m_images) {
      img.m_handle = API::Window::image(this->m_handle, index++);
    }

    return false;
  }
  return true;
}

template <typename API>
auto Window<API>::handle() const -> int32_t {
  return this->m_handle;
}
}  // namespace ohm