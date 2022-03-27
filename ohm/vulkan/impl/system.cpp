#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "ohm/vulkan/impl/system.h"

namespace ohm {
namespace ovk {
auto system() -> System& {
  static System sys;
  return sys;
}
}  // namespace ovk
}  // namespace ohm
