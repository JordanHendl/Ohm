#pragma once
#include <vulkan/vulkan.hpp>
#include "ohm/api/exception.h"  
namespace ohm {
namespace ovk {

/** Function to take in a vulkan error and handle it.
 * If it is a debug build, checks the error and throws.
 * Otherwise, this function is a simple inline passthrough.
 */
template <typename T>
inline auto error(vk::ResultValue<T> result) {
  OhmAssert(result.result != vk::Result::eSuccess,
            vk::to_string(result.result));
  return result.value;
}

/** Similar to the above function, this one handles vulkan results.
 * In a debug build, checks error and throws if it is a failure
 * Otherwise, this function is a NOP and should get compiled out.
 */
inline auto error(vk::Result result) {
  OhmAssert(result != vk::Result::eSuccess, vk::to_string(result));
}
}  // namespace ovk
}  // namespace ohm
