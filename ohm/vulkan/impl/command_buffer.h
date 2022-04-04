#pragma once
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "device.h"
#include "ohm/api/commands.h"

namespace ohm {
namespace ovk {
class Buffer;
class Image;
class Pipeline;
class RenderPass;
class Device;
class Descriptor;
class CommandBuffer;
class Swapchain;

using Family = unsigned;
using PoolMap = std::unordered_map<Family, vk::CommandPool>;
using MutexMap = std::unordered_map<Family, std::mutex>;
using ThreadMap = std::unordered_map<std::thread::id, PoolMap>;
static ThreadMap thread_map;
static MutexMap mutex_map;
auto clearPools(Device& device) -> void;

class CommandBuffer {
 public:
  CommandBuffer();
  CommandBuffer(Device& device, QueueType type);
  CommandBuffer(RenderPass& pass);
  CommandBuffer(CommandBuffer& cmd);
  CommandBuffer(CommandBuffer&& mv);
  ~CommandBuffer();
  auto operator=(CommandBuffer&& mv) -> CommandBuffer&;
  auto attach(RenderPass& pass) -> void;
  auto begin() -> void;
  auto beginDraw() -> void;
  auto copy(const Buffer& src, Buffer& dst, size_t amt = 0) -> void;
  auto copy(const Buffer& src, Image& dst, size_t amt = 0) -> void;
  auto copy(Image& src, Buffer& dst, size_t amt = 0) -> void;
  auto copy(const Buffer& src, unsigned char* dst, size_t amt = 0) -> void;
  auto copy(const unsigned char* src, Buffer& dst, size_t amt = 0) -> void;
  auto copy(Image& src, Image& dst, size_t amt = 0) -> void;
  auto addDependancy(vk::Semaphore semaphore) -> void;
  auto clearDependancies() -> void;
  auto detach() -> void;
  auto bind(Descriptor& desc) -> void;
  auto blit(Image& src, Image& dst, Filter filter) -> void;
  auto blit(RenderPass& src, Swapchain& dst, Filter filter,
            unsigned subpass = 0, unsigned framebuffer = 0) -> void;
  auto blit(RenderPass& src, RenderPass& dst, Filter filter,
            unsigned subpass = 0, unsigned framebuffer = 0) -> void;
  auto blit(Image& src, Swapchain& dst, Filter filter) -> void;
  auto combine(CommandBuffer& child) -> void;
  auto draw(const Buffer& vertices, unsigned instance_count = 1) -> void;
  auto draw(const Buffer& indices, const Buffer& vertices,
            unsigned instance_count = 1) -> void;
  auto dispatch(unsigned x, unsigned y, unsigned z = 1) -> void;
  auto depended() const -> bool;
  auto setDepended(bool flag) -> void;
  auto end() -> void;
  //          auto transition( Image& texture, Layout layout ) -> void ;
  auto transition(Image& texture, vk::ImageLayout layout) -> void;
  //        auto transitionSingle( Image& texture, vk::CommandBuffer cmd,
  //        vk::ImageLayout layout ) -> void ;
  auto synchronize() -> void;
  auto submit() -> void;
  auto present(Swapchain& swapchain) -> bool;
  auto pipelineBarrier(unsigned src, unsigned dst) -> void;
  auto wait(CommandBuffer& buffer) -> void;
  auto cmd(unsigned index) -> vk::CommandBuffer;
  inline auto pool() { return this->m_vk_pool; }
  inline auto queue() -> Queue& { return *this->m_queue; }
  inline auto current() const {
    return this->m_cmd_buffers[this->m_current_id];
  }
  inline auto fence() const -> const vk::Fence& {
    return this->m_sync_info[this->m_current_id].fence;
  }
  inline auto fence(unsigned index) { return this->m_sync_info[index].fence; }
  inline auto initialized() const { return !this->m_cmd_buffers.empty(); }

 private:
  using RecordFunction =
      std::function<void(vk::CommandBuffer& buffer, unsigned index)>;
  struct CmdBuffSync {
    bool signaled = false;
    bool render_pass_started = false;
    vk::Fence fence;
    vk::Semaphore semaphore;
  };

  using CmdBuffers = std::vector<vk::CommandBuffer>;
  using Fences = std::vector<vk::Fence>;

  Device* m_device;
  RenderPass* m_render_pass;
  CommandBuffer* m_dependency;
  Queue* m_queue;
  vk::SubpassContents m_subpass_flags;
  vk::PipelineBindPoint m_bind_point;
  vk::CommandBufferBeginInfo m_begin_info;
  vk::CommandBufferInheritanceInfo m_inheritance;
  vk::CommandPool m_vk_pool;
  CommandBuffer* m_parent;
  CmdBuffers m_cmd_buffers;
  std::vector<CmdBuffSync> m_sync_info;
  std::vector<vk::Semaphore> m_dependancies;
  bool m_recording;
  size_t m_current_id;
  std::mutex m_lock;
  bool m_dirty;
  bool m_depended;
  bool m_first;

  /** Method to retrieve the map of command pools from a queue family.
   * @note creates pool if it is not found.
   * @param queue_family
   * @return A Reference to the created Command Pool.
   */
  auto create_pool(Family queue_family) -> vk::CommandPool;

  /** Method to grab the currently active command buffer.
   * @return The currently active command buffer.
   */

  /** Method to grab the currently active command buffer.
   * @return The currently active command buffer.
   */

  /** Method to advance the current command buffer.
   */
  auto advance() -> void;

  /** Method to end the current render pass and JUST the current render pass.
   */
  auto endRenderPass() -> void;

  /** Method to initiate recording of command buffers.
   */
  auto record() -> void;

  /** Method to append an operation to each command buffer.
   * @param function The function to use for appending to the command buffer.
   */
  auto append(RecordFunction function) -> void;

  /** Method to rerecord this command buffer.
   * Simply rerecords all the current recorded operations.
   */
  auto rerecord() -> void;

  /** Method to retrieve the previous ID of command buffer.
   * @return The previous ID of command buffer.
   */
  auto previousID() -> unsigned;

  /** Method to synchronize this object.
   */
  auto unsafe_synchronize() -> void;

  auto unsafe_end() -> void;
};
}  // namespace ovk
}  // namespace ohm
