#pragma once
#include <map>
#include <vulkan/vulkan.hpp>
#include "device.h"
#include "ohm/io/shader.h"
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
  auto file() const -> const io::Shader&;
  auto device() const -> const Device&;
  auto layout() const -> const vk::DescriptorSetLayout&;
  auto inputs() const
      -> const std::vector<vk::VertexInputAttributeDescription>&;
  auto bindings() const
      -> const std::vector<vk::VertexInputBindingDescription>&;
  auto shaderInfos() const
      -> const std::vector<vk::PipelineShaderStageCreateInfo>&;
  auto descriptorLayouts() const
      -> const std::vector<vk::DescriptorSetLayoutBinding>&;

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
