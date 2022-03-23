#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "command_buffer.h"
#include <array>
#include <climits>
#include <iostream>
#include <map>
#include <vector>
#include "api/exception.h"
#include "buffer.h"
#include "device.h"
#include "error.h"
#include "image.h"
#include "memory.h"

namespace ohm {
namespace ovk {
constexpr unsigned BUFFER_COUNT = 4;

static std::map<vk::Device, std::map<Family, vk::CommandPool>> pool_map;
auto CommandBuffer::create_pool(Family queue_family) -> vk::CommandPool {
  const vk::CommandPoolCreateFlags flags =
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer;  // TODO make this
                                                           // configurable.
  vk::CommandPoolCreateInfo info;

  info.setFlags(flags);
  info.setQueueFamilyIndex(queue_family);

  auto& queue_map = pool_map[this->m_device->device()];

  auto iter = queue_map.find(queue_family);
  if (iter == queue_map.end()) {
    iter = queue_map.insert(
        iter, {queue_family, this->m_device->device().createCommandPool(
                                 info, this->m_device->allocationCB(),
                                 this->m_device->dispatch())});
  };

  return iter->second;
}

auto clearPools(Device& device) -> void {
  auto& queue_map = pool_map[device.device()];
  for (auto& pool : queue_map) {
    device.device().destroy(pool.second, nullptr, device.dispatch());
  }
  queue_map.clear();
}

CommandBuffer::CommandBuffer() {
  this->m_subpass_flags = vk::SubpassContents::eInline;
  this->m_recording = false;
  this->m_current_id = 0;
  this->m_render_pass = nullptr;
  this->m_dirty = false;
  this->m_dependency = nullptr;
  this->m_depended = false;
  this->m_first = true;
  this->m_parent = nullptr;
}

CommandBuffer::CommandBuffer(Device& device, QueueType type) {
  vk::CommandBufferAllocateInfo info;
  this->m_device = &device;

  this->m_subpass_flags = vk::SubpassContents::eInline;
  this->m_recording = false;
  this->m_current_id = 0;
  this->m_render_pass = nullptr;
  this->m_dirty = false;
  this->m_dependency = nullptr;
  this->m_depended = false;
  this->m_first = true;
  this->m_parent = nullptr;

  switch (type) {
    case QueueType::Compute:
      this->m_queue = &device.compute();
      break;
    case QueueType::Graphics:
      this->m_queue = &device.graphics();
      break;
      //        case QueueType::Sparse   : this->m_queue = &device.sparse  () ;
      //        break ;
    case QueueType::Transfer:
      this->m_queue = &device.transfer();
      break;
    default:
      this->m_queue = &device.compute();
  }

  this->m_vk_pool = this->create_pool(this->m_queue->id);
  info.setCommandBufferCount(BUFFER_COUNT);
  info.setLevel(vk::CommandBufferLevel::ePrimary);
  info.setCommandPool(this->m_vk_pool);

  this->m_sync_info.resize(BUFFER_COUNT);

  for (auto& sync : this->m_sync_info) {
    vk::FenceCreateInfo fence_info;
    fence_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

    sync.fence = error(this->m_device->device().createFence(
        fence_info, this->m_device->allocationCB(),
        this->m_device->dispatch()));
    sync.semaphore = error(this->m_device->device().createSemaphore(
        {}, this->m_device->allocationCB(), this->m_device->dispatch()));
  }

  this->m_cmd_buffers = error(this->m_device->device().allocateCommandBuffers(
      info, this->m_device->dispatch()));
}

CommandBuffer::CommandBuffer(CommandBuffer& cmd) {
  vk::CommandBufferAllocateInfo info;

  this->m_subpass_flags = vk::SubpassContents::eInline;
  this->m_recording = false;
  this->m_current_id = 0;
  this->m_dirty = false;
  this->m_dependency = nullptr;
  this->m_depended = false;
  this->m_first = true;
  this->m_device = cmd.m_device;
  this->m_queue = cmd.m_queue;
  this->m_vk_pool = cmd.m_vk_pool;
  this->m_render_pass = cmd.m_render_pass;
  this->m_parent = &cmd;

  info.setCommandBufferCount(BUFFER_COUNT);
  info.setLevel(vk::CommandBufferLevel::eSecondary);
  info.setCommandPool(this->m_vk_pool);

  this->m_sync_info.resize(BUFFER_COUNT);

  for (auto& sync : this->m_sync_info) {
    vk::FenceCreateInfo fence_info;
    fence_info.setFlags(vk::FenceCreateFlagBits::eSignaled);

    sync.fence = error(this->m_device->device().createFence(
        fence_info, this->m_device->allocationCB(),
        this->m_device->dispatch()));
    sync.semaphore = error(this->m_device->device().createSemaphore(
        {}, this->m_device->allocationCB(), this->m_device->dispatch()));
  }

  this->m_begin_info.setPInheritanceInfo(&this->m_inheritance);
  this->m_cmd_buffers = error(this->m_device->device().allocateCommandBuffers(
      info, this->m_device->dispatch()));
}

CommandBuffer::CommandBuffer(CommandBuffer&& mv) { *this = std::move(mv); }

CommandBuffer::~CommandBuffer() {
  if (this->initialized()) {
    this->synchronize();
    auto device = this->m_device->device();
    if (this->m_cmd_buffers.size() != 0)
      device.freeCommandBuffers(this->m_vk_pool, this->m_cmd_buffers.size(),
                                this->m_cmd_buffers.data(),
                                this->m_device->dispatch());
    for (auto& fence : this->m_sync_info) {
      device.destroy(fence.fence, this->m_device->allocationCB(),
                     this->m_device->dispatch());
      device.destroy(fence.semaphore, this->m_device->allocationCB(),
                     this->m_device->dispatch());
    }

    this->m_device = nullptr;
    this->m_render_pass = nullptr;
    this->m_dependency = nullptr;
    this->m_queue = nullptr;
    this->m_subpass_flags = vk::SubpassContents();
    this->m_bind_point = vk::PipelineBindPoint();
    this->m_begin_info = vk::CommandBufferBeginInfo();
    this->m_inheritance = vk::CommandBufferInheritanceInfo();
    this->m_vk_pool = vk::CommandPool();
    this->m_parent = nullptr;
    this->m_recording = false;
    this->m_current_id = 0;
    this->m_dirty = false;
    this->m_depended = false;
    this->m_first = false;

    this->m_cmd_buffers.clear();
    this->m_sync_info.clear();
    this->m_dependancies.clear();
  }
}

auto CommandBuffer::operator=(CommandBuffer&& mv) -> CommandBuffer& {
  this->m_device = mv.m_device;
  this->m_render_pass = mv.m_render_pass;
  this->m_dependency = mv.m_dependency;
  this->m_queue = mv.m_queue;
  this->m_subpass_flags = mv.m_subpass_flags;
  this->m_bind_point = mv.m_bind_point;
  this->m_begin_info = mv.m_begin_info;
  this->m_inheritance = mv.m_inheritance;
  this->m_vk_pool = mv.m_vk_pool;
  this->m_parent = mv.m_parent;
  this->m_cmd_buffers = mv.m_cmd_buffers;
  this->m_sync_info = mv.m_sync_info;
  this->m_dependancies = mv.m_dependancies;
  this->m_recording = mv.m_recording;
  this->m_current_id = mv.m_current_id;
  this->m_dirty = mv.m_dirty;
  this->m_depended = mv.m_depended;
  this->m_first = mv.m_first;

  mv.m_device = nullptr;
  mv.m_render_pass = nullptr;
  mv.m_dependency = nullptr;
  mv.m_queue = nullptr;
  mv.m_subpass_flags = vk::SubpassContents();
  mv.m_bind_point = vk::PipelineBindPoint();
  mv.m_begin_info = vk::CommandBufferBeginInfo();
  mv.m_inheritance = vk::CommandBufferInheritanceInfo();
  mv.m_vk_pool = vk::CommandPool();
  mv.m_parent = nullptr;
  mv.m_recording = false;
  mv.m_current_id = 0;
  mv.m_dirty = false;
  mv.m_depended = false;
  mv.m_first = false;

  mv.m_cmd_buffers.clear();
  mv.m_sync_info.clear();
  mv.m_dependancies.clear();

  return *this;
}

auto CommandBuffer::advance() -> void {
  this->m_current_id = (this->m_current_id + 1) % this->m_cmd_buffers.size();
}

auto CommandBuffer::record() -> void {
  if (!this->m_recording) {
    this->unsafe_synchronize();
    if (this->m_parent) {
      OhmException(!this->m_parent->m_recording, Error::APIError,
                   "Attempting to record to a child command buffer without "
                   "beginning the parent first. Children must have their "
                   "begin()/end() combo in the parent's begin()/end() combo.");
    }

    for (auto& cmd : this->m_cmd_buffers) {
      error(cmd.begin(this->m_begin_info, this->m_device->dispatch()));
    }
  }
  this->m_recording = true;
}

auto CommandBuffer::end() -> void {
  std::unique_lock<std::mutex> lock(this->m_lock);
  this->unsafe_end();
}

auto CommandBuffer::unsafe_end() -> void {
  if (this->m_recording)
    for (unsigned index = 0; index < this->m_cmd_buffers.size(); index++) {
      auto& cmd = this->m_cmd_buffers[index];
      if (this->m_recording) error(cmd.end(this->m_device->dispatch()));
    }

  this->m_recording = false;
}

auto CommandBuffer::append(
    std::function<void(vk::CommandBuffer& buffer, unsigned index)> function)
    -> void {
  for (unsigned index = 0; index < this->m_cmd_buffers.size(); index++) {
    function(this->m_cmd_buffers[index], index);
  }
}

auto CommandBuffer::previousID() -> unsigned {
  return this->m_current_id == 0 ? BUFFER_COUNT - 1 : this->m_current_id - 1;
}

auto CommandBuffer::unsafe_synchronize() -> void {
  auto& device = *this->m_device;
  for (auto& sync : this->m_sync_info) {
    error(device.device().waitForFences(1, &sync.fence, true, UINT64_MAX,
                                        device.dispatch()));
  }
}

auto CommandBuffer::begin() -> void {
  std::unique_lock<std::mutex> lock(this->m_lock);
  this->record();
}

auto CommandBuffer::copy(const Buffer& src, Buffer& dst, size_t) -> void {
  vk::BufferCopy region;
  vk::MemoryBarrier barrier;

  barrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
  barrier.setDstAccessMask(vk::AccessFlagBits::eMemoryWrite);

  region.setSize(std::min(src.size(), dst.size()));
  region.setSrcOffset(src.memory().offset);
  region.setDstOffset(dst.memory().offset);

  std::unique_lock<std::mutex> lock(this->m_lock);

  OhmException(!this->m_recording, Error::APIError,
               "Attempting to record to a command buffer without starting a "
               "record operation.");
  auto function = [&](vk::CommandBuffer& cmd, size_t) {
    cmd.copyBuffer(src.buffer(), dst.buffer(), 1, &region,
                   this->m_device->dispatch());
  };

  this->append(function);
  this->m_dirty = true;
}

auto CommandBuffer::copy(const Buffer& src, Image& dst, size_t) -> void {
  vk::BufferImageCopy info;
  vk::Extent3D extent;

  extent.setWidth(dst.width());
  extent.setHeight(dst.height());
  extent.setDepth(1);

  info.setImageExtent(extent);
  info.setBufferImageHeight(0);
  info.setBufferRowLength(0);
  info.setImageOffset(dst.memory().offset);
  info.setImageSubresource(dst.subresource());

  std::unique_lock<std::mutex> lock(this->m_lock);
  OhmException(!this->m_recording, Error::APIError,
               "Attempting to record to a command buffer without starting a "
               "record operation.");

  auto function = [&](vk::CommandBuffer& cmd, size_t) {
    cmd.copyBufferToImage(src.buffer(), dst.image(), vk::ImageLayout::eGeneral,
                          1, &info, this->m_device->dispatch());
  };

  auto dst_old_layout = dst.layout();

  if (dst.layout() != vk::ImageLayout::eGeneral)
    this->transition(dst, vk::ImageLayout::eGeneral);

  this->append(function);

  if (dst_old_layout != vk::ImageLayout::eUndefined)
    this->transition(dst, dst_old_layout);

  this->m_dirty = true;
}

auto CommandBuffer::copy(Image& src, Buffer& dst, size_t) -> void {
  vk::BufferImageCopy info;
  vk::Extent3D extent;

  extent.setWidth(src.width());
  extent.setHeight(src.height());
  extent.setDepth(src.layers());

  info.setImageExtent(extent);
  info.setBufferImageHeight(0);
  info.setBufferRowLength(0);
  info.setImageOffset(dst.memory().offset);
  info.setImageSubresource(src.subresource());

  auto function = [&](vk::CommandBuffer& cmd, size_t) {
    cmd.copyImageToBuffer(src.image(), vk::ImageLayout::eGeneral, dst.buffer(),
                          1, &info, this->m_device->dispatch());
  };

  std::unique_lock<std::mutex> lock(this->m_lock);
  OhmException(!this->m_recording, Error::APIError,
               "Attempting to record to a command buffer without starting a "
               "record operation.");

  auto src_old_layout = src.layout();

  if (src.layout() != vk::ImageLayout::eGeneral)
    this->transition(src, vk::ImageLayout::eGeneral);

  this->append(function);

  if (src_old_layout != vk::ImageLayout::eUndefined)
    this->transition(src, src_old_layout);

  this->m_dirty = true;
}

auto CommandBuffer::copy(const Buffer& src, unsigned char* dst, size_t amt)
    -> void {
  unsigned char* ptr;

  auto copy_amt = amt == 0 ? src.count() : amt;
  copy_amt *= src.elementSize();

  std::unique_lock<std::mutex> lock(this->m_lock);
  src.memory().map(reinterpret_cast<void**>(&ptr));
  std::copy(ptr, ptr + (copy_amt), dst);
  src.memory().unmap();
}

auto CommandBuffer::copy(const unsigned char* src, Buffer& dst, size_t amt)
    -> void {
  void* ptr;

  auto copy_amt = amt == 0 ? dst.count() : amt;
  copy_amt *= dst.elementSize();

  std::unique_lock<std::mutex> lock(this->m_lock);
  dst.memory().map(&ptr);
  std::memcpy(ptr, src,
              copy_amt);  // note: have to use memcpy because of void* usage.
  dst.memory().unmap();
}

auto CommandBuffer::copy(Image& src, Image& dst, size_t) -> void {
  vk::ImageCopy region;
  vk::Extent3D extent;

  extent.setWidth(dst.width());
  extent.setHeight(dst.height());
  extent.setDepth(dst.layers());

  region.setExtent(extent);
  region.setSrcOffset(src.offset());
  region.setDstOffset(dst.offset());
  region.setSrcSubresource(src.subresource());
  region.setDstSubresource(dst.subresource());

  std::unique_lock<std::mutex> lock(this->m_lock);
  OhmException(!this->m_recording, Error::APIError,
               "Attempting to record to a command buffer without starting a "
               "record operation.");

  auto function = [&](vk::CommandBuffer& cmd, size_t) {
    cmd.copyImage(src.image(), src.layout(), dst.image(), dst.layout(), 1,
                  &region, this->m_device->dispatch());
  };

  auto src_old_layout = src.layout();
  auto dst_old_layout = dst.layout();

  this->transition(src, vk::ImageLayout::eGeneral);
  this->transition(dst, vk::ImageLayout::eGeneral);
  this->append(function);
  if (src_old_layout != vk::ImageLayout::eUndefined)
    this->transition(src, src_old_layout);
  if (dst_old_layout != vk::ImageLayout::eUndefined)
    this->transition(dst, dst_old_layout);

  this->m_dirty = true;
}

auto CommandBuffer::clearDependancies() -> void {
  this->m_dependancies.clear();
}

auto CommandBuffer::addDependancy(vk::Semaphore semaphore) -> void {
  this->m_dependancies.push_back(semaphore);
}

auto CommandBuffer::bind(Descriptor& desc) -> void {
  //        auto& pipeline = desc.pipeline();
  //        const auto bind_point = pipeline.graphics() ?
  //        vk::PipelineBindPoint::eGraphics : vk::PipelineBindPoint::eCompute ;
  //        auto function = [&]( vk::CommandBuffer& cmd, size_t )
  //        {
  //          cmd.bindPipeline( bind_point, pipeline.pipeline(),
  //          this->m_device->dispatch()) ; if( desc.set() )
  //          cmd.bindDescriptorSets( bind_point, pipeline.layout(), 0, 1,
  //          &desc.set(), 0, nullptr, this->m_device->dispatch() ) ;
  //        };
  //
  //        std::unique_lock<std::mutex> lock( this->m_lock ) ;
  //        this->m_record( true ) ;
  //        this->m_append( function ) ;
  //        this->m_dirty = true ;
}

auto CommandBuffer::blit(Image& src, Image& dst, Filter in_filter) -> void {
  vk::ImageBlit blit;
  vk::Filter filter;

  switch (in_filter) {
    case Filter::Cubic:
      filter = vk::Filter::eCubicIMG;
      break;
    case Filter::Linear:
      filter = vk::Filter::eLinear;
      break;
    case Filter::Nearest:
      filter = vk::Filter::eNearest;
      break;
    default:
      filter = vk::Filter::eLinear;
      break;
  };

  std::array<vk::Offset3D, 2> src_offsets = {
      vk::Offset3D(0, 0, 0), vk::Offset3D(src.width(), src.height(), 1)};
  std::array<vk::Offset3D, 2> dst_offsets = {
      vk::Offset3D(0, 0, 0), vk::Offset3D(dst.width(), dst.height(), 1)};

  blit.setSrcSubresource(src.subresource());
  blit.setDstSubresource(dst.subresource());
  blit.setSrcOffsets(src_offsets);
  blit.setDstOffsets(dst_offsets);
  auto src_old_layout = src.layout();
  auto dst_old_layout = dst.layout();

  auto function = [&](vk::CommandBuffer& cmd, size_t) {
    cmd.blitImage(src.image(), src.layout(), dst.image(), dst.layout(), 1,
                  &blit, filter, this->m_device->dispatch());
  };

  std::unique_lock<std::mutex> lock(this->m_lock);
  OhmException(!this->m_recording, Error::APIError,
               "Attempting to record to a command buffer without starting a "
               "record operation.");

  this->transition(src, vk::ImageLayout::eGeneral);
  this->transition(dst, vk::ImageLayout::eGeneral);

  this->append(function);

  if (src_old_layout != vk::ImageLayout::eUndefined)
    this->transition(src, src_old_layout);
  if (dst_old_layout != vk::ImageLayout::eUndefined)
    this->transition(dst, dst_old_layout);

  this->m_dirty = true;
}

auto CommandBuffer::blit(RenderPass& src, RenderPass& dst, Filter in_filter,
                         unsigned, unsigned framebuffer) -> void {}

auto CommandBuffer::blit(RenderPass& src, Swapchain& dst, Filter in_filter,
                         unsigned, unsigned framebuffer) -> void {}

auto CommandBuffer::blit(Image& src, Swapchain& dst, Filter in_filter) -> void {

}

auto CommandBuffer::detach() -> void {}

auto CommandBuffer::combine(CommandBuffer& child) -> void {
  // Note, 2 locks here, but this is because we need to lock down both this
  // command buffer and the child's to ensure no 2 threads can alter them while
  // we're combnining them.
  std::unique_lock<std::mutex> lock(this->m_lock);
  std::unique_lock<std::mutex> lock2(child.m_lock);
  OhmException(!this->m_recording, Error::APIError,
               "Attempting to combine child command buffers without recording "
               "the parent first.");
  child.end();

  auto function = [&](vk::CommandBuffer& cmd, unsigned index) {
    cmd.executeCommands(1, &child.m_cmd_buffers[index],
                        this->m_device->dispatch());
  };

  this->append(function);

  this->m_dirty = true;
}

auto CommandBuffer::cmd(unsigned index) -> vk::CommandBuffer {
  return this->m_cmd_buffers[index];
}

auto CommandBuffer::draw(const Buffer& vertices, unsigned instance_count)
    -> void {
  std::unique_lock<std::mutex> lock(this->m_lock);

  const auto& offset = vertices.memory().offset;
  const auto& buffer = vertices.buffer();

  auto function = [&](vk::CommandBuffer& cmd, size_t) {
    cmd.bindVertexBuffers(0, 1, &buffer, &offset, this->m_device->dispatch());
    cmd.draw(vertices.count(), instance_count, 0, 0,
             this->m_device->dispatch());
  };

  OhmException(!this->m_recording, Error::APIError,
               "Attempting to record to a command buffer without starting a "
               "record operation.");
  this->append(function);
  this->m_dirty = true;
}

auto CommandBuffer::draw(const Buffer& indices, const Buffer& vertices,
                         unsigned instance_count) -> void {
  std::unique_lock<std::mutex> lock(this->m_lock);

  const auto& vertex_offset = vertices.memory().offset;

  const auto& vertex_buffer = vertices.buffer();
  const auto& index_buffer = indices.buffer();

  auto function = [&](vk::CommandBuffer& cmd, size_t) {
    cmd.bindVertexBuffers(0, 1, &vertex_buffer, &vertex_offset,
                          this->m_device->dispatch());
    cmd.bindIndexBuffer(index_buffer, 0, vk::IndexType::eUint32,
                        this->m_device->dispatch());
    cmd.drawIndexed(indices.count(), instance_count, 0, 0, 0,
                    this->m_device->dispatch());
  };

  OhmException(!this->m_recording, Error::APIError,
               "Attempting to record to a command buffer without starting a "
               "record operation.");
  OhmException(!this->m_render_pass, Error::APIError,
               "Attempting to record a rendering operation to a command buffer "
               "without attaching a render pass.");
  this->append(function);
  this->m_dirty = true;
}

auto CommandBuffer::dispatch(unsigned x, unsigned y, unsigned z) -> void {
  std::unique_lock<std::mutex> lock(this->m_lock);

  auto function = [&](vk::CommandBuffer& cmd, size_t) {
    cmd.dispatch(x, y, z, this->m_device->dispatch());
  };

  OhmException(!this->m_recording, Error::APIError,
               "Attempting to record to a command buffer without starting a "
               "record operation.");
  this->append(function);
  this->m_dirty = true;
}

auto CommandBuffer::depended() const -> bool { return this->m_depended; }

auto CommandBuffer::setDepended(bool flag) -> void { this->m_depended = flag; }

auto CommandBuffer::transition(Image& image, vk::ImageLayout layout) -> void {
  vk::ImageSubresourceRange range;
  vk::PipelineStageFlags src;
  vk::PipelineStageFlags dst;
  vk::DependencyFlags dep_flags;
  vk::ImageLayout new_layout;
  vk::ImageLayout old_layout;

  new_layout = layout;
  old_layout = image.layout();

  range.setBaseArrayLayer(0);
  range.setBaseMipLevel(0);
  range.setLevelCount(1);
  range.setLayerCount(image.layers());
  if (image.format() == vk::Format::eD24UnormS8Uint)
    range.setAspectMask(vk::ImageAspectFlagBits::eDepth |
                        vk::ImageAspectFlagBits::eStencil);
  else
    range.setAspectMask(vk::ImageAspectFlagBits::eColor);

  image.barrier().setOldLayout(old_layout);
  image.barrier().setNewLayout(new_layout);
  image.barrier().setImage(image.image());
  image.barrier().setSubresourceRange(range);

  //@JH TODO this is not ok and needs to be addressed. memory can get
  // invalidated doing this.
  image.barrier().setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
  image.barrier().setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

  dep_flags = vk::DependencyFlags();
  src = vk::PipelineStageFlagBits::eTopOfPipe;
  dst = vk::PipelineStageFlagBits::eBottomOfPipe;

  OhmException(!this->m_recording, Error::APIError,
               "Attempting to record to a command buffer without starting a "
               "record operation.");

  if (new_layout != vk::ImageLayout::eUndefined) {
    auto function = [&](vk::CommandBuffer& cmd, size_t) {
      cmd.pipelineBarrier(src, dst, dep_flags, 0, nullptr, 0, nullptr, 1,
                          &image.barrier(), this->m_device->dispatch());
    };

    this->append(function);
    this->m_dirty = true;
    image.setLayout(new_layout);
  };
}

auto CommandBuffer::synchronize() -> void {
  // Lock this command buffer access.
  std::unique_lock<std::mutex> lock1(this->m_lock);
  this->unsafe_synchronize();
}

auto CommandBuffer::submit() -> void {
  vk::SubmitInfo info;

  if (!this->m_dirty) return;
  std::unique_lock<std::mutex> lock(this->m_lock);

  auto vector = std::vector<vk::Semaphore>();
  auto masks = std::vector<vk::PipelineStageFlags>();

  this->unsafe_end();

  error(this->m_device->device().waitForFences(
      1, &this->m_sync_info[this->m_current_id].fence, true, UINT64_MAX,
      this->m_device->dispatch()));
  error(this->m_device->device().resetFences(
      1, &this->m_sync_info[this->m_current_id].fence,
      this->m_device->dispatch()));

  auto buffer = this->current();

  if (this->m_dependency != nullptr && this->m_dependency->m_first == false) {
    vector.push_back(
        this->m_dependency->m_sync_info[this->m_dependency->previousID()]
            .semaphore);
    masks.push_back(vk::PipelineStageFlagBits::eAllCommands);
  }

  for (auto& sem : this->m_dependancies) {
    vector.push_back(sem);
    masks.push_back(vk::PipelineStageFlagBits::eAllCommands);
  }

  info.setCommandBufferCount(1);
  info.setPCommandBuffers(&buffer);
  info.setWaitSemaphores(vector);
  info.setSignalSemaphoreCount(this->m_depended ? 1 : 0);
  info.setPSignalSemaphores(&this->m_sync_info[this->m_current_id].semaphore);
  info.setWaitDstStageMask(masks);

  auto fence = this->m_sync_info[this->m_current_id].fence;

  std::unique_lock<std::mutex> queue_lock(this->m_queue->lock);
  error(
      this->m_queue->queue.submit(1, &info, fence, this->m_device->dispatch()));

  this->advance();
  this->m_first = false;
}

auto CommandBuffer::present(Swapchain& swapchain) -> void {}

auto CommandBuffer::wait(CommandBuffer& cmd) -> void {
  OhmException(
      cmd.m_depended, Error::APIError,
      "Attempting to have multiple dependencies on one Command Buffer.");
  this->m_dependency = &cmd;
  cmd.m_depended = true;
}
}  // namespace ovk
}  // namespace ohm