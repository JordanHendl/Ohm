#pragma once
#include "array.h"
namespace ohm {
  
  template<typename API>
  class Descriptor;
  
  template<typename API, typename Allocator>
  class RenderPass;
  
  enum class QueueType {
    Graphics,
    Compute,
    Transfer,
  };
  
  enum class Filter : int;
  template<typename API, QueueType Queue = QueueType::Graphics>
  class Commands {
    public:
      explicit Commands();
      explicit Commands(int device);
      explicit Commands(Commands<API, Queue>& parent);
      explicit Commands(Commands<API, Queue>&& mv);
      ~Commands();
      template<typename Allocator>
      auto attach(const RenderPass<API, Allocator>& pass) -> void;
      
      template<typename Type1, typename Type2>
      auto blit(const Type1& src, Type2& dst, Filter filter) -> void;
      
      template<typename Type1, typename Type2>
      auto copy(const Type1& src, Type2& dst, size_t count = 0) -> void;
      
      template<typename Type1, typename Type2>
      auto copy(const Type1* src, Type2& dst, size_t count = 0) -> void;
      
      template<typename Type1, typename Type2>
      auto copy(const Type1& src, Type2* dst, size_t count = 0) -> void;
      
      template<typename Type, typename Type2, typename Allocator>
      auto draw(const Array<API, Type, Allocator>& indices, const Array<API, Type2, Allocator>& vertices, size_t instance_count = 1) -> void;
      
      template<typename Type, typename Allocator>
      auto draw(const Array<API, Type, Allocator>& vertices, size_t instance_count = 1) -> void;
      
      auto dispatch(size_t x, size_t y, size_t z = 1) -> void;
      auto detach() -> void;
      auto combine(const Commands& child) -> void;
      auto operator=(Commands<API, Queue>& cpy) = delete;
      auto operator=(Commands<API, Queue>&& mv);
      auto wait(const Commands<API, Queue>& cmds);
      auto begin() -> void;
      auto end() -> void;
      auto bind(const Descriptor<API>& desc);
      auto handle() const -> int32_t;
      auto synchronize() -> void;
      auto submit() -> void;
    private:
      int32_t m_handle;
  };
  
  template<typename API, QueueType Queue>
  Commands<API, Queue>::Commands() {
    this->m_handle = -1;
  }
  
  template<typename API, QueueType Queue>
  Commands<API, Queue>::Commands(int gpu) {
    this->m_handle = API::Commands::create(gpu, Queue);
  }
  
  template<typename API, QueueType Queue>
  Commands<API, Queue>::Commands(Commands<API, Queue>& parent) {
    this->m_handle = API::Commands::create(parent.handle());
  }
  
  template<typename API, QueueType Queue>
  Commands<API, Queue>::Commands(Commands<API, Queue>&& mv) {
    this->m_handle = mv.m_handle;
    mv.m_handle = -1;
  }
  
  template<typename API, QueueType Queue>
  Commands<API, Queue>::~Commands() {
    if(this->m_handle >= 0) {
      API::Commands::destroy(this->m_handle);
      this->m_handle = -1;
    }
  }
  
  template<typename API, QueueType Queue>
  template<typename Allocator>
  auto Commands<API, Queue>::attach(const RenderPass<API, Allocator>& pass) -> void {
    
  }
  
  template<typename API, QueueType Queue>
  template<typename Type1, typename Type2>
  auto Commands<API, Queue>::blit(const Type1& src, Type2& dst, Filter filter) -> void {
    
  }
  
  template<typename API, QueueType Queue>
  template<typename Type1, typename Type2>
  auto Commands<API, Queue>::copy(const Type1& src, Type2& dst, size_t count) -> void {
    
  }
  
  template<typename API, QueueType Queue>
  template<typename Type1, typename Type2>
  auto Commands<API, Queue>::copy(const Type1* src, Type2& dst, size_t count) -> void {
    
  }
      
  template<typename API, QueueType Queue>
  template<typename Type1, typename Type2>
  auto Commands<API, Queue>::copy(const Type1& src, Type2* dst, size_t count) -> void {
    
  }
      
  template<typename API, QueueType Queue>
  template<typename Type, typename Type2, typename Allocator>
  auto Commands<API, Queue>::draw(const Array<API, Type, Allocator>& indices, const Array<API, Type2, Allocator>& vertices, size_t instance_count) -> void {
    
  }
  
  template<typename API, QueueType Queue>
  template<typename Type, typename Allocator>
  auto Commands<API, Queue>::draw(const Array<API, Type, Allocator>& vertices, size_t instance_count) -> void {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::dispatch(size_t x, size_t y, size_t z) -> void {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::detach() -> void {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::combine(const Commands& child) -> void {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::operator=(Commands<API, Queue>&& mv) {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::wait(const Commands<API, Queue>& cmds) {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::begin() -> void {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::end() -> void {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::bind(const Descriptor<API>& desc) {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::handle() const -> int32_t {
    return this->m_handle;
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::synchronize() -> void {
    
  }
  
  template<typename API, QueueType Queue>
  auto Commands<API, Queue>::submit() -> void {
    
  }
}

