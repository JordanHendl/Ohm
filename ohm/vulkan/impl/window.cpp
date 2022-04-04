#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS
#include "ohm/vulkan/impl/window.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <string>
#include "ohm/vulkan/impl/error.h"
#include "ohm/vulkan/impl/system.h"
namespace ohm {
namespace ovk {
Window::Window() { this->m_window = nullptr; }

Window::Window(const WindowInfo& info) {
  auto instance = ovk::system().instance.instance();
  this->m_window =
      (SDL_CreateWindow(info.title.c_str(), 0, 0, info.width, info.height,
                        SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN));

  auto raw_surface = VkSurfaceKHR{};
  auto error = SDL_Vulkan_CreateSurface(this->m_window, instance, &raw_surface);
  OhmAssert(!error, "Failed to create surface for window!");
  (void)error;
  this->m_surface = raw_surface;
  this->update(info);
}

Window::Window(Window&& mv) { *this = std::move(mv); }

Window::~Window() {
  if (this->m_window) SDL_DestroyWindow(this->m_window);
  auto instance = ovk::system().instance.instance();
  auto& dispatch = ovk::system().instance.dispatch();
  if (this->m_surface) instance.destroy(this->m_surface, nullptr, dispatch);
  this->m_surface = nullptr;
  this->m_window = nullptr;
}

auto Window::operator=(Window&& mv) -> Window& {
  this->m_window = mv.m_window;
  this->m_surface = mv.m_surface;
  mv.m_window = nullptr;
  mv.m_surface = nullptr;
  return *this;
}

auto Window::update(const WindowInfo& info) -> void {
  this->setCaptureMouse(info.capture_mouse);

  if (info.borderless != this->borderless())
    this->setBorderless(info.borderless);
  if (info.width != this->width()) this->setWidth(info.width);
  if (info.height != this->height()) this->setHeight(info.height);
  if (info.fullscreen != this->fullscreen())
    this->setFullscreen(info.fullscreen);
  if (info.resizable != this->resizable()) this->setResizable(info.resizable);
  if (info.title != this->title()) this->setTitle(info.title);
}

auto Window::setTitle(std::string_view title) -> void {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");

  SDL_SetWindowTitle(this->m_window, title.begin());
}

auto Window::setBorderless(bool value) -> void {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_SetWindowBordered(this->m_window, static_cast<SDL_bool>(value));
}

auto Window::setFullscreen(bool value) -> void {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_SetWindowFullscreen(this->m_window, value);
}

auto Window::setWidth(size_t value) -> void {
  int x;
  int y;

  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetWindowSize(this->m_window, &x, &y);
  x = value;
  SDL_SetWindowSize(this->m_window, x, y);
}

auto Window::setHeight(size_t value) -> void {
  int x;
  int y;

  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetWindowSize(this->m_window, &x, &y);
  y = value;
  SDL_SetWindowSize(this->m_window, x, y);
}

auto Window::setXPos(size_t value) -> void {
  int x;
  int y;

  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_window, &x, &y);
  x = value;
  SDL_SetWindowPosition(this->m_window, x, y);
}

auto Window::setYPos(size_t value) -> void {
  int x;
  int y;

  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_window, &x, &y);
  y = value;
  SDL_SetWindowPosition(this->m_window, x, y);
}

auto Window::setResizable(bool value) -> void {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_SetWindowResizable(this->m_window, static_cast<SDL_bool>(value));
}

auto Window::setMonitor(size_t value) -> void {
  SDL_DisplayMode mode;
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetDesktopDisplayMode(value, &mode);
  SDL_SetWindowDisplayMode(this->m_window, &mode);
}

auto Window::setMinimized(bool value) -> void {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  if (value)
    SDL_MinimizeWindow(this->m_window);
  else
    SDL_RestoreWindow(this->m_window);
}

auto Window::setMaximized(bool value) -> void {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  if (value)
    SDL_MaximizeWindow(this->m_window);
  else
    SDL_RestoreWindow(this->m_window);
}

auto Window::setShown(bool value) -> void {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  if (value)
    SDL_ShowWindow(this->m_window);
  else
    SDL_HideWindow(this->m_window);
}

auto Window::setCaptureMouse(bool value) -> void {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_CaptureMouse(static_cast<SDL_bool>(value));
  SDL_SetWindowGrab(this->m_window, static_cast<SDL_bool>(value));

  // If we capture the mouse, we want to disable the cursor at the same time.
  auto enabled = value ? SDL_DISABLE : SDL_ENABLE;
  SDL_SetRelativeMouseMode(static_cast<SDL_bool>(value));
  SDL_ShowCursor(enabled);
}

auto Window::title() -> std::string_view {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  return SDL_GetWindowTitle(this->m_window);
}

auto Window::borderless() -> bool {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_window);

  return flags & SDL_WINDOW_BORDERLESS;
}

auto Window::fullscreen() -> bool {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_window);

  return flags & SDL_WINDOW_FULLSCREEN;
}

auto Window::width() -> size_t {
  int w = 0;
  int h = 0;

  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetWindowSize(this->m_window, &w, &h);

  return w;
}

auto Window::height() -> size_t {
  int w = 0;
  int h = 0;

  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetWindowSize(this->m_window, &w, &h);

  return h;
}

auto Window::xPos() -> size_t {
  int x = 0;
  int y = 0;

  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_window, &x, &y);

  return x;
}

auto Window::yPos() -> size_t {
  int x = 0;
  int y = 0;

  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_window, &x, &y);

  return y;
}

auto Window::monitor() -> size_t {
  int x = 0;
  int y = 0;

  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  SDL_GetWindowPosition(this->m_window, &x, &y);

  return y;
}

auto Window::resizable() -> bool {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_window);

  return flags & SDL_WINDOW_RESIZABLE;
}

auto Window::minimized() -> bool {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_window);

  return flags & SDL_WINDOW_MINIMIZED;
}

auto Window::maximized() -> bool {
  OhmAssert(
      !this->m_window,
      "Attempting to access athis->m_window that has not been initialized.");
  auto flags = SDL_GetWindowFlags(this->m_window);

  return flags & SDL_WINDOW_MAXIMIZED;
}
}  // namespace ovk
}  // namespace ohm
