#pragma once
#include <SDL.h>
#include <string>
#include <vulkan/vulkan.hpp>
#include "ohm/api/window.h"
namespace ohm {
namespace ovk {
class Window {
 public:
  Window();
  Window(const WindowInfo& info);
  Window(Window&& mv);
  ~Window();
  auto operator=(Window&& mv) -> Window&;
  auto update(const WindowInfo& info) -> void;
  auto setTitle(std::string_view title) -> void;
  auto setBorderless(bool value) -> void;
  auto setFullscreen(bool value) -> void;
  auto setWidth(size_t value) -> void;
  auto setHeight(size_t value) -> void;
  auto setXPos(size_t value) -> void;
  auto setYPos(size_t value) -> void;
  auto setResizable(bool value) -> void;
  auto setMonitor(size_t value) -> void;
  auto setMinimized(bool value) -> void;
  auto setMaximized(bool value) -> void;
  auto setShown(bool value) -> void;
  auto setCaptureMouse(bool value) -> void;
  auto active() const -> bool;
  auto title() -> std::string_view;
  auto borderless() -> bool;
  auto fullscreen() -> bool;
  auto width() -> size_t;
  auto height() -> size_t;
  auto xPos() -> size_t;
  auto yPos() -> size_t;
  auto resizable() -> bool;
  auto monitor() -> size_t;
  auto minimized() -> bool;
  auto maximized() -> bool;
  auto initialized() -> bool { return this->m_window; }
  auto window() -> SDL_Window* { return this->m_window; }
  auto surface() { return this->m_surface; }

 private:
  SDL_Window* m_window;
  vk::SurfaceKHR m_surface;
};
}  // namespace ovk
}  // namespace ohm