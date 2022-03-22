#pragma once
#include <string>
#include <vector>
namespace ohm {
struct Gpu {
  std::string name;
  int slot;
};

template <typename API>
class System {
 public:
  static auto initialize() -> void;
  static auto name() -> std::string;
  static auto setParameter(std::string_view param) -> void;
  static auto setDebugParameter(std::string_view param) -> void;
  static auto devices() -> std::vector<Gpu>;
  static auto shutdown() -> void;
};

template <typename API>
auto System<API>::initialize() -> void {
  API::System::initialize();
}

template <typename API>
auto System<API>::shutdown() -> void {
  API::System::shutdown();
}

template <typename API>
auto System<API>::name() -> std::string {
  return API::System::name();
}

template <typename API>
auto System<API>::setParameter(std::string_view param) -> void {
  API::System::setParameter(param);
}

template <typename API>
auto System<API>::setDebugParameter(std::string_view param) -> void {
  API::System::setDebugParameter(param);
}

template <typename API>
auto System<API>::devices() -> std::vector<Gpu> {
  return API::System::devices();
}
}  // namespace ohm

/** Required functions of API
 * System::name -> string
 * System::setParameter -> void
 * System::devices -> vector<Gpu>
 */