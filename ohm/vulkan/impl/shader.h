#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "ohm/io/shader.h"
#include "ohm/vulkan/impl/device.h"
namespace ohm {
namespace ovk {
class Device;
class Shader {
 public:
  enum class InputRate {
    Vertex,
    Instanced,
  };

  Shader();
  Shader(const Shader& shader);
  Shader(Device& device, std::string_view path);
  Shader(Device& device,
         std::vector<std::pair<std::string, std::string>> inline_files);
  Shader(Shader&& mv);
  ~Shader();
  Shader& operator=(Shader&& mv);
  auto file() const -> const io::Shader& { return *this->m_file; }

  auto device() const -> const Device& { return *this->m_device; }

  auto layout() const -> const vk::DescriptorSetLayout& {
    return this->m_layout;
  }

  auto inputs() const
      -> const std::vector<vk::VertexInputAttributeDescription>& {
    return this->m_inputs;
  }

  auto bindings() const
      -> const std::vector<vk::VertexInputBindingDescription>& {
    return this->m_bindings;
  }

  auto shaderInfos() const
      -> const std::vector<vk::PipelineShaderStageCreateInfo>& {
    return this->m_infos;
  }

  auto descriptorLayouts() const
      -> const std::vector<vk::DescriptorSetLayoutBinding>& {
    return this->m_descriptors;
  }

 private:
  using SPIRVMap =
      std::map<vk::ShaderStageFlagBits, vk::ShaderModuleCreateInfo>;
  using ShaderModules = std::map<io::Shader::Type, vk::ShaderModule>;
  using Attributes = std::vector<vk::VertexInputAttributeDescription>;
  using Bindings = std::vector<vk::VertexInputBindingDescription>;
  using Infos = std::vector<vk::PipelineShaderStageCreateInfo>;
  using Descriptors = std::vector<vk::DescriptorSetLayoutBinding>;

  ShaderModules m_modules;
  Descriptors m_descriptors;
  SPIRVMap m_spirv_map;
  Attributes m_inputs;
  Bindings m_bindings;
  Infos m_infos;
  std::unique_ptr<io::Shader> m_file;
  Device* m_device;
  vk::DescriptorSetLayout m_layout;
  vk::PipelineVertexInputStateCreateInfo m_info;
  vk::VertexInputRate m_rate;

  inline auto parse() -> void;
  inline auto makeDescriptorLayout() -> void;
  inline auto makeShaderModules() -> void;
  inline auto makePipelineShaderInfos() -> void;
};
}  // namespace ovk
}  // namespace ohm
