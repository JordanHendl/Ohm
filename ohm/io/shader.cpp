#include "shader.h"
#include <spirv_reflect.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <shaderc/shaderc.hpp>
#include <string>
#include "ohm/api/exception.h"

namespace ohm {
namespace io {
inline namespace v1 {
constexpr auto osh_magic_number = 0x6F686D79676F64;
constexpr auto target_spirv_version = shaderc_spirv_version_1_3;
constexpr auto target_environment = shaderc_target_env_vulkan;
constexpr auto target_env_version = shaderc_env_version_vulkan_1_2;

inline auto sanitize(std::string_view view) -> std::string {
  if (view.find(".osh") == std::string::npos) {
    return std::string(view) + std::string(".osh");
  }

  return std::string(view);
}

inline auto type_from_name(const std::string& type) -> Shader::Type {
  auto find = [](std::string_view a, std::string_view b) {
    return a.find(b) != std::string::npos;
  };

  if (find(type, std::string(".comp"))) return Shader::Type::Compute;
  if (find(type, std::string(".vert"))) return Shader::Type::Vertex;
  if (find(type, std::string(".frag"))) return Shader::Type::Fragment;
  if (find(type, std::string(".geom"))) return Shader::Type::Geometry;
  if (find(type, std::string(".tessc")))
    return Shader::Type::TesselationControl;
  if (find(type, std::string(".tesse"))) return Shader::Type::TesselationEval;
  return Shader::Type::None;
}

inline auto convert(Shader::Type type) -> shaderc_shader_kind {
  switch (type) {
    case Shader::Type::Compute:
      return shaderc_compute_shader;
    case Shader::Type::Vertex:
      return shaderc_vertex_shader;
    case Shader::Type::Fragment:
      return shaderc_fragment_shader;
    case Shader::Type::Geometry:
      return shaderc_geometry_shader;
    case Shader::Type::TesselationControl:
      return shaderc_tess_control_shader;
    case Shader::Type::TesselationEval:
      return shaderc_tess_evaluation_shader;
    default:
      return shaderc_glsl_infer_from_source;
  }
}

inline auto convert(SpvReflectDescriptorType type)
    -> Shader::Stage::Variable::Type {
  using Type = Shader::Stage::Variable::Type;

  switch (type) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
      return Type::Sampler;
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      return Type::Sampler;
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      return Type::Sampled;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      return Type::Image;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      return Type::UTexel;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return Type::STexel;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      return Type::Uniform;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      return Type::Storage;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      return Type::UniformDynamic;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      return Type::StorateDynamic;
    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      return Type::Input;
    case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
      return Type::Acceleration;
    default:
      return Type::None;
  }
}

inline auto convert(SpvReflectFormat format) -> Shader::Stage::Attribute::Type {
  using Type = Shader::Stage::Attribute::Type;
  switch (format) {
    case SPV_REFLECT_FORMAT_UNDEFINED:
      return Type::Undefined;
    case SPV_REFLECT_FORMAT_R32_UINT:
      return Type::eInt;
    case SPV_REFLECT_FORMAT_R32_SINT:
      return Type::eInt;
    case SPV_REFLECT_FORMAT_R32_SFLOAT:
      return Type::eFloat;
    case SPV_REFLECT_FORMAT_R32G32_UINT:
      return Type::eIVec2;
    case SPV_REFLECT_FORMAT_R32G32_SINT:
      return Type::eIVec2;
    case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
      return Type::eVec2;
    case SPV_REFLECT_FORMAT_R32G32B32_UINT:
      return Type::eIVec3;
    case SPV_REFLECT_FORMAT_R32G32B32_SINT:
      return Type::eIVec3;
    case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
      return Type::eVec3;
    case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
      return Type::eIVec4;
    case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
      return Type::eIVec4;
    case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
      return Type::eVec4;
    default:
      return Type::Undefined;
  }
}
struct Shader::ShaderData {
  std::vector<std::vector<uint32_t>> spirv;
  std::vector<Shader::Stage> stages;
  std::vector<std::string> macros;

  inline auto reflect(Shader::Stage& stage) -> void;
  inline auto reflect_variables(Shader::Stage& stage,
                                SpvReflectShaderModule& module) -> void;
  inline auto reflect_io(Shader::Stage& stage, SpvReflectShaderModule& module)
      -> void;
  inline auto preprocess(std::string_view name, shaderc_shader_kind kind,
                         std::string_view src) -> std::string;
  inline auto assemblize(std::string_view name, shaderc_shader_kind kind,
                         std::string_view src, bool optimize = false)
      -> std::string;
  inline auto assemble(shaderc_shader_kind kind, std::string_view src,
                       bool optimize = false) -> std::vector<uint32_t>;
};

auto Shader::ShaderData::reflect(Shader::Stage& stage) -> void {
  constexpr auto success = SPV_REFLECT_RESULT_SUCCESS;

  auto module = SpvReflectShaderModule{};
  auto& spv = stage.spirv;
  auto result = spvReflectCreateShaderModule(spv.size() * sizeof(uint32_t),
                                             spv.data(), &module);
  OhmException(result != success, Error::LogicError, "Failed to parse SPV.");

  this->reflect_variables(stage, module);
  this->reflect_io(stage, module);
  (void)result;
  (void)success;
}

auto Shader::ShaderData::reflect_io(Shader::Stage& stage,
                                    SpvReflectShaderModule& module) -> void {
  constexpr auto success = SPV_REFLECT_RESULT_SUCCESS;
  auto count = 0u;
  auto result = spvReflectEnumerateInputVariables(&module, &count, nullptr);
  OhmException(result != success, Error::LogicError, "Failed to enumerate SPV");

  auto inputs = std::vector<SpvReflectInterfaceVariable*>(count);
  result = spvReflectEnumerateInputVariables(&module, &count, inputs.data());
  OhmException(result != success, Error::LogicError, "Failed to enumerate SPV");

  //---

  count = 0u;
  result = spvReflectEnumerateOutputVariables(&module, &count, nullptr);
  OhmException(result != success, Error::LogicError, "Failed to enumerate SPV");

  auto outputs = std::vector<SpvReflectInterfaceVariable*>(count);
  result = spvReflectEnumerateOutputVariables(&module, &count, outputs.data());
  OhmException(result != success, Error::LogicError, "Failed to enumerate SPV");

  for (const auto* input : inputs) {
    auto attribute = Stage::Attribute();
    attribute.name = input->name;
    attribute.location = input->location;
    attribute.type = convert(input->format);
    if (attribute.name != std::string("gl_GlobalInvocationID"))
      stage.in_attributes.push_back(attribute);
  }

  for (const auto* output : outputs) {
    auto attribute = Stage::Attribute();
    attribute.name = output->name;
    attribute.location = output->location;
    attribute.type = convert(output->format);
    stage.out_attributes.push_back(attribute);
  }
  (void)result;
  (void)success;
}

auto Shader::ShaderData::reflect_variables(Shader::Stage& stage,
                                           SpvReflectShaderModule& module)
    -> void {
  constexpr auto success = SPV_REFLECT_RESULT_SUCCESS;
  auto count = 0u;
  auto result = spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
  OhmException(result != success, Error::LogicError, "Failed to enumerate SPV");

  auto sets = std::vector<SpvReflectDescriptorSet*>(count);
  result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
  OhmException(result != success, Error::LogicError, "Failed to enumerate SPV");

  for (const auto* set : sets) {
    for (auto index = 0u; index < set->binding_count; index++) {
      auto& binding = set->bindings[index];
      auto tmp = Stage::Variable();
      tmp.binding = binding->binding;
      tmp.type = convert(binding->descriptor_type);
      tmp.set = set->set;
      tmp.size = binding->count;
      stage.variables[binding->name] = tmp;
    }
  }
  (void)success;
  (void)result;
}
auto Shader::ShaderData::preprocess(std::string_view name,
                                    shaderc_shader_kind kind,
                                    std::string_view src) -> std::string {
  auto compiler = shaderc::Compiler();
  auto options = shaderc::CompileOptions();

  for (auto& macro : this->macros) options.AddMacroDefinition(macro);

  options.SetTargetEnvironment(target_environment, target_env_version);
  options.SetTargetSpirv(target_spirv_version);
  auto result =
      compiler.PreprocessGlsl(src.begin(), kind, name.begin(), options);

  OhmException(
      result.GetCompilationStatus() != shaderc_compilation_status_success,
      Error::LogicError, "Failed to preprocess shader.");

  return {result.cbegin(), result.cend()};
}

auto Shader::ShaderData::assemblize(std::string_view name,
                                    shaderc_shader_kind kind,
                                    std::string_view src, bool optimize)
    -> std::string {
  auto compiler = shaderc::Compiler();
  auto options = shaderc::CompileOptions();

  for (auto& macro : this->macros) options.AddMacroDefinition(macro);

  options.SetTargetEnvironment(target_environment, target_env_version);
  options.SetTargetSpirv(target_spirv_version);
  if (optimize)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

  auto result = compiler.CompileGlslToSpvAssembly(src.begin(), kind,
                                                  name.begin(), options);

  OhmException(
      result.GetCompilationStatus() != shaderc_compilation_status_success,
      Error::LogicError, "Failed to assemblize shader.");

  return {result.cbegin(), result.cend()};
}

auto Shader::ShaderData::assemble(shaderc_shader_kind kind,
                                  std::string_view src, bool optimize)
    -> std::vector<uint32_t> {
  auto compiler = shaderc::Compiler();
  auto options = shaderc::CompileOptions();

  for (auto& macro : this->macros) options.AddMacroDefinition(macro);

  options.SetTargetEnvironment(target_environment, target_env_version);
  options.SetTargetSpirv(target_spirv_version);
  if (optimize)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

  auto result = compiler.AssembleToSpv(src.begin(), src.size(), options);

  OhmException(
      result.GetCompilationStatus() != shaderc_compilation_status_success,
      Error::LogicError, "Failed to assemble shader.");

  return {result.cbegin(), result.cend()};
}

Shader::Shader() { this->data = std::make_shared<Shader::ShaderData>(); }

Shader::Shader(std::string_view osh_file) {
  this->data = std::make_shared<Shader::ShaderData>();
  this->load(osh_file);
}

Shader::Shader(const std::vector<std::string>& glsl_to_load) {
  this->data = std::make_shared<Shader::ShaderData>();

  // This is horribly inefficient, but the argument is that if you're loading
  // files its gonna be slow anyways, and this is code reuse sooooooooo yeah.
  auto pair_vector = std::vector<std::pair<std::string, std::string>>();
  pair_vector.reserve(glsl_to_load.size());
  for (auto& str : glsl_to_load) {
    auto stream = std::ifstream(str);

    if (stream) {
      auto tmp = std::string();
      stream.seekg(0, std::ios::end);
      tmp.reserve(stream.tellg());
      stream.seekg(0, std::ios::beg);

      tmp.assign((std::istreambuf_iterator<char>(stream)),
                 std::istreambuf_iterator<char>());
      pair_vector.push_back({str, tmp});
      stream.close();
    }
  }

  *this = std::move(Shader(pair_vector));
}

Shader::Shader(Shader&& mv) {
  this->data = std::make_shared<Shader::ShaderData>();
}

Shader::Shader(
    const std::vector<std::pair<std::string, std::string>>& inline_files) {
  this->data = std::make_shared<Shader::ShaderData>();

  for (auto& shdr : inline_files) {
    auto& name = shdr.first;
    auto& file = shdr.second;

    auto type = type_from_name(name);
    auto kind = convert(type);

    auto preprocessed = this->data->preprocess(name, kind, file);
    auto assembly = this->data->assemblize(name, kind, preprocessed);

    auto stage = Shader::Stage();
    stage.name = name;
    stage.spirv = this->data->assemble(kind, assembly);
    stage.type = type;

    this->data->reflect(stage);
    this->data->stages.push_back(stage);
  }
}

Shader::~Shader() {}

auto Shader::operator=(Shader&& mv) -> Shader& {
  this->data = std::move(mv.data);
  return *this;
}

auto Shader::stages() const -> const std::vector<Stage>& {
  return this->data->stages;
}

auto Shader::save(std::string_view path) -> bool {
  auto sanitized_path = sanitize(path);
  auto stream = std::ofstream(sanitized_path, std::ios::binary | std::ios::out);
  OhmException(stream, Error::LogicError, "Could not load file!");
  auto& stages = this->stages();
  stream << osh_magic_number;
  stream << stages.size();

  for (auto& stage : stages) {
    stream << static_cast<int>(stage.type);
    stream << stage.name;
    stream.write(reinterpret_cast<const char*>(stage.spirv.data()),
                 stage.spirv.size());
  }

  stream.close();
  return true;
}

auto Shader::load(std::string_view path) -> bool {
  auto stream = std::ifstream(path.begin(), std::ios::binary | std::ios::in);
  OhmException(stream, Error::LogicError, "Could not load file!");
  auto num_stages = 0;
  auto magic = 0;

  this->data = std::make_shared<Shader::ShaderData>();
  stream >> magic;
  OhmException(magic != osh_magic_number, Error::LogicError,
               "Shader magic number not valid.");
  stream >> num_stages;
  this->data->stages.resize(num_stages);

  for (auto& stage : this->data->stages) {
    auto type = 0;
    stream >> type;
    stream >> stage.name;
    stage.type = static_cast<Type>(type);
    stream.read(reinterpret_cast<char*>(stage.spirv.data()),
                stage.spirv.size());
    this->data->reflect(stage);
  }

  stream.close();
  return true;
}
}  // namespace v1
}  // namespace io
}  // namespace ohm