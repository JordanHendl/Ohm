#pragma once
#include <vulkan/vulkan.hpp>
#include "api/memory.h"
#include "api/array.h"

namespace ohm
{
  namespace ovk
  {
    class Device ;
    class Memory ;
    
    struct Buffer
    {
      public:
        Buffer();
        Buffer( Device& device, size_t count, size_t element_size );
        Buffer(Buffer&& mv);
        ~Buffer() ;
        auto operator=(Buffer&& mv) -> Buffer&;
        auto bind( Memory& memory ) -> void;
        inline auto initialized() const -> bool {return this->buffer();}
        inline auto count() const -> size_t {return this->m_count;}
        inline auto size() const -> size_t {return this->m_requirements.size;}
        inline auto elementSize() const -> size_t {return this->m_element_size;}
        inline auto memory() -> Memory& {return *this->m_memory;}
        inline auto memory() const -> const Memory& {return *this->m_memory;}
        inline auto buffer() -> vk::Buffer& {return this->m_buffer;}
        inline auto buffer() const -> const vk::Buffer& {return this->m_buffer;}
      private:
        Device*                m_device       ;
        Memory*                m_memory       ;
        vk::Buffer             m_buffer       ;
        size_t                 m_count        ;
        size_t                 m_element_size ;
        vk::BufferUsageFlags   m_flags        ;
        vk::MemoryRequirements m_requirements ;
        void createBuffer( unsigned size ) ;
    };
  }
}

