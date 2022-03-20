#pragma once
#include "memory.h"

namespace ohm {
  template<typename API, typename Type, class Allocator = DefaultAllocator<API>>
  class Buffer {
    public:
      Buffer();
      explicit Buffer(int gpu, size_t count);
      explicit Buffer(int gpu, size_t count, bool mappable);
      ~Buffer();
      auto memory() const -> const Memory<API, Allocator>&;
      auto size() const -> size_t;
      auto byte_size() const -> size_t;
      
    private:
      int32_t m_handle;
      Memory<API, Allocator> m_memory;
  };

}