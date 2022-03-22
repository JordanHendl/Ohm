#pragma once
#include <vulkan/vulkan.hpp>
#include "api/memory.h"

namespace ohm {
namespace ovk {
class Device;

/** Class to represent Vulkan Memory
 */
struct Memory {
  Memory();

  /** Constructor.
   * @param device The device to allocate memory on.
   * @param size The requested size.
   * @param coherent Whether the memory should be host-coherent/mappable.
   */
  Memory(Device& device, unsigned size, size_t heap, HeapType type);

  /** Constructor.
   * @param memory The object to base this one off of.
   * @param offset The offset of the input memory to start this object at.
   */
  Memory(const Memory& memory, unsigned offset);

  /** Move constructor.
   * @param cpy The object to copy from.
   */
  Memory(Memory&& cpy);

  /** Deconstructor.
   */
  ~Memory();

  auto operator=(Memory&& mem) -> Memory&;

  /** Method to check if this object is initialized.
   * @return Whether this object is initialized.
   */
  inline auto initialized() const -> bool { return this->memory; }

  /** Method to map the input pointer to this object's device memory, if
   * applicable.
   * @param ptr The pointer reference to map.
   */
  auto map(void** ptr) const -> void;

  /** Method to unmap any mapped memory.
   */
  auto unmap() const -> void;

  unsigned size;
  size_t heap;
  vk::DeviceSize offset;
  bool coherent;
  mutable void** host_ptr;
  vk::DeviceMemory memory;
  Device* device;
  HeapType type;
};
}  // namespace ovk
}  // namespace ohm
