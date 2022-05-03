#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "ohm/vulkan/impl/system.h"

namespace ohm {
namespace ovk {
static System sys;
auto system() -> System& {  
  return sys;
}
}  // namespace ovk
}  // namespace ohm
