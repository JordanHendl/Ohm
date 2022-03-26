#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "shader.h"
#include <fstream>
#include <istream>
#include <map>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "buffer.h"
#include "device.h"
#include "error.h"
#include "image.h"
#include "ohm/io/shader.h"

namespace ohm {
namespace ovk {
static inline auto convert(io::VariableType type) -> vk::DescriptorType;
static inline auto convert(io::ShaderType stage) -> vk::ShaderStageFlagBits;
static inline auto convert(io::AttributeType type) -> vk::Format;
static inline auto convert(vk::ShaderStageFlagBits flag) -> io::ShaderType;
static inline auto byteSize(io::AttributeType type) -> size_t;

auto byteSize(io::AttributeType type) -> size_t {
  switch (type) {
    case io::AttributeType::eMat4:
      return 4 * 4 * sizeof(float);
    case io::AttributeType::eMat3:
      return 3 * 3 * sizeof(float);
    case io::AttributeType::eMat2:
      return 2 * 2 * sizeof(float);
    case io::AttributeType::eVec4:
      return 4 * sizeof(float);
    case io::AttributeType::eVec3:
      return 3 * sizeof(float);
    case io::AttributeType::eVec2:
      return 2 * sizeof(float);
    case io::AttributeType::eIVec4:
      return 4 * sizeof(int);
    case io::AttributeType::eIVec3:
      return 3 * sizeof(int);
    case io::AttributeType::eIVec2:
      return 2 * sizeof(int);
    case io::AttributeType::eFloat:
      return sizeof(float);
    case io::AttributeType::eInt:
      return sizeof(int);
    default:
      return 4;
  }
}

auto convert(io::AttributeType type) -> vk::Format {
  switch (type) {
    case io::AttributeType::eMat4:
      return vk::Format::eR32G32B32A32Sfloat;
    case io::AttributeType::eVec4:
      return vk::Format::eR32G32B32A32Sfloat;
    case io::AttributeType::eIVec4:
      return vk::Format::eR32G32B32A32Sint;
    case io::AttributeType::eMat3:
      return vk::Format::eR32G32B32A32Sfloat;
    case io::AttributeType::eVec3:
      return vk::Format::eR32G32B32A32Sfloat;
    case io::AttributeType::eIVec3:
      return vk::Format::eR32G32B32A32Sint;
    case io::AttributeType::eMat2:
      return vk::Format::eR32G32Sfloat;
    case io::AttributeType::eVec2:
      return vk::Format::eR32G32Sfloat;
    case io::AttributeType::eIVec2:
      return vk::Format::eR32G32Sint;
    case io::AttributeType::eFloat:
      return vk::Format::eR32Sfloat;
    case io::AttributeType::eInt:
      return vk::Format::eR32Sint;
    default:
      return vk::Format::eR32Sfloat;
  }
}

auto convert(io::ShaderType stage) -> vk::ShaderStageFlagBits {
  switch (stage) {
    case io::ShaderType::Fragment:
      return vk::ShaderStageFlagBits::eFragment;
    case io::ShaderType::Geometry:
      return vk::ShaderStageFlagBits::eGeometry;
    case io::ShaderType::TesselationControl:
      return vk::ShaderStageFlagBits::eTessellationControl;
    case io::ShaderType::TesselationEval:
      return vk::ShaderStageFlagBits::eTessellationEvaluation;
    case io::ShaderType::Compute:
      return vk::ShaderStageFlagBits::eCompute;
    case io::ShaderType::Vertex:
      return vk::ShaderStageFlagBits::eVertex;
    default:
      return vk::ShaderStageFlagBits::eFragment;
  }
}

auto convert(io::VariableType type) -> vk::DescriptorType {
  switch (type) {
    case io::VariableType::Uniform:
      return vk::DescriptorType::eUniformBuffer;
      break;
    case io::VariableType::Sampler:
      return vk::DescriptorType::eCombinedImageSampler;
      break;
    case io::VariableType::Image:
      return vk::DescriptorType::eStorageImage;
      break;
    case io::VariableType::Storage:
      return vk::DescriptorType::eStorageBuffer;
      break;
    case io::VariableType::None:
      return vk::DescriptorType::eUniformBuffer;
      break;
    default:
      return vk::DescriptorType::eMutableVALVE;
  }
}

auto convert(vk::ShaderStageFlagBits flag) -> io::ShaderType {
  switch (flag) {
    case vk::ShaderStageFlagBits::eVertex:
      return io::ShaderType::Vertex;
    case vk::ShaderStageFlagBits::eFragment:
      return io::ShaderType::Fragment;
    case vk::ShaderStageFlagBits::eCompute:
      return io::ShaderType::Compute;
    case vk::ShaderStageFlagBits::eGeometry:
      return io::ShaderType::Geometry;
    default:
      return io::ShaderType::Vertex;
  }
}

Shader::Shader() { this->m_rate = vk::VertexInputRate::eVertex; }

Shader::Shader(Device& device, std::string_view path) {
  this->m_rate = vk::VertexInputRate::eVertex;
  this->m_device = &device;
  this->m_file = std::make_unique<io::Shader>(path);

  this->parse();
  this->makeDescriptorLayout();
  this->makeShaderModules();
  this->makePipelineShaderInfos();
}

Shader::Shader(Device& device,
               std::vector<std::pair<std::string, std::string>> inline_files) {
  this->m_rate = vk::VertexInputRate::eVertex;
  this->m_device = &device;
  this->m_file = std::make_unique<io::Shader>(inline_files);

  this->parse();
  this->makeDescriptorLayout();
  this->makeShaderModules();
  this->makePipelineShaderInfos();
}

Shader::~Shader() {
  auto device = this->m_device->device();
  auto* alloc_cb = this->m_device->allocationCB();
  auto& dispatch = this->m_device->dispatch();

  for (auto module : this->m_modules) {
    device.destroy(module.second, alloc_cb, dispatch);
  }

  if (this->m_layout) device.destroy(this->m_layout, alloc_cb, dispatch);

  this->m_modules.clear();
  this->m_inputs.clear();
  this->m_descriptors.clear();
  this->m_bindings.clear();
  this->m_spirv_map.clear();
  this->m_infos.clear();
}

auto Shader::parse() -> void {
  auto binding_map = std::map<std::string, vk::DescriptorSetLayoutBinding>();
  auto binding = vk::DescriptorSetLayoutBinding();
  auto module_info = vk::ShaderModuleCreateInfo();
  auto attr = vk::VertexInputAttributeDescription();
  auto bind = vk::VertexInputBindingDescription();

  auto offset = 0u;
  for (auto& stage : this->m_file->stages()) {
    for (auto& attribute : stage.in_attributes) {
      attr.setLocation(attribute.location);
      attr.setBinding(0);
      attr.setFormat(convert(attribute.type));
      attr.setOffset(offset);

      this->m_inputs.push_back(attr);
      offset += byteSize(attribute.type);
    }

    for (auto& variable : stage.variables) {
      auto iter = binding_map.find(variable.first);
      if (iter != binding_map.end()) {
        auto& flags = iter->second.stageFlags;
        flags |= convert(stage.type);
      } else {
        auto& var = variable.second;
        binding.setBinding(var.binding);
        binding.setDescriptorCount(var.size);
        binding.setStageFlags(binding.stageFlags | convert(stage.type));
        binding.setDescriptorType(convert(var.type));
        binding_map.insert(iter, {variable.first, binding});
      }
    }
    module_info.setCodeSize(stage.spirv.size() * sizeof(unsigned));
    module_info.setPCode(stage.spirv.data());
    this->m_spirv_map[convert(stage.type)] = module_info;
  }

  bind.setBinding(0);
  bind.setInputRate(this->m_rate);
  bind.setStride(offset);

  this->m_bindings.push_back(bind);
  this->m_descriptors.reserve(binding_map.size());
  for (auto bind : binding_map) {
    this->m_descriptors.push_back(bind.second);
  }
}

auto Shader::makeDescriptorLayout() -> void {
  auto info = vk::DescriptorSetLayoutCreateInfo();

  info.setBindings(this->m_descriptors);
  this->m_layout = error(this->m_device->device().createDescriptorSetLayout(
      info, this->m_device->allocationCB(), this->m_device->dispatch()));
}

auto Shader::makeShaderModules() -> void {
  auto device = this->m_device->device();
  auto* alloc_cb = this->m_device->allocationCB();
  auto& dispatch = this->m_device->dispatch();
  auto mod = vk::ShaderModule();
  this->m_modules.clear();

  for (auto shader : this->m_spirv_map) {
    mod = error(device.createShaderModule(shader.second, alloc_cb, dispatch));
    this->m_modules[convert(shader.first)] = mod;
  }
}

auto Shader::makePipelineShaderInfos() -> void {
  auto info = vk::PipelineShaderStageCreateInfo();
  auto iter = 0u;

  this->m_infos.clear();
  this->m_infos.resize(this->m_modules.size());

  for (auto& it : this->m_modules) {
    info.setStage(convert(it.first));
    info.setModule(it.second);
    info.setPName("main");
    this->m_infos[iter++] = info;
  }
}
}  // namespace ovk
}  // namespace ohm
