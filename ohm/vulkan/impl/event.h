#pragma once
#include "ohm/api/event.h"
#include <vector>
#include <functional>
#include <SDL.h>
namespace ohm {
  namespace ovk {
  class Event {
  public:
    Event();
    Event(Event&& mv);
    ~Event();
    auto operator=(Event&& mv) -> Event&;
    auto add(std::function<void(const ohm::Event&)> cb) -> void;
    auto push(SDL_Event& e) -> void;
  private:
    std::vector<std::function<void(const ohm::Event&)>> m_callbacks;
  };
}
}