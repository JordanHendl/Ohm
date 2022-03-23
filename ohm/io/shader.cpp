#include "shader.h"
#include "ohm/api/exception.h"
#include <shaderc/shaderc.hpp>
#include <spirv-tools/libspirv.hpp>
#include <string>

namespace ohm {
namespace io {
inline namespace v1 {
  struct Shader::ShaderData {
    std::vector<Shader::Stage> stages;
    std::vector<std::string> macros;
    
    auto preprocess(std::string_view name, shaderc_shader_kind kind, std::string_view src) -> std::string;
    auto assemblize(std::string_view name, shaderc_shader_kind kind, std::string_view src, bool optimize = false) -> std::string;
    auto assemble(shaderc_shader_kind kind, std::string_view src, bool optimize = false) -> std::vector<uint32_t>;
  };
  
  auto Shader::ShaderData::preprocess(std::string_view name, shaderc_shader_kind kind, std::string_view src) -> std::string {
    auto compiler = shaderc::Compiler();
    auto options = shaderc::CompileOptions();
    
    for(auto& macro : this->macros)
      options.AddMacroDefinition(macro);
    
    auto result = compiler.PreprocessGlsl(src.begin(), kind, name.begin(), options);
    
    OhmException(result.GetCompilationStatus() != shaderc_compilation_status_success, Error::LogicError, "Failed to preprocess shader.");
    
    return {result.cbegin(), result.cend()};
  }
  
  auto Shader::ShaderData::assemblize(std::string_view name, shaderc_shader_kind kind, std::string_view src, bool optimize) -> std::string {
    auto compiler = shaderc::Compiler();
    auto options = shaderc::CompileOptions();
    
    for(auto& macro : this->macros)
      options.AddMacroDefinition(macro);
    
    if(optimize) options.SetOptimizationLevel(shaderc_optimization_level_performance);
    
    auto result = compiler.CompileGlslToSpvAssembly(src.begin(), kind, name.begin(), options);
    
    OhmException(result.GetCompilationStatus() != shaderc_compilation_status_success, Error::LogicError, "Failed to preprocess shader.");
    
    return {result.cbegin(), result.cend()};
  }
  
  auto Shader::ShaderData::assemble(shaderc_shader_kind kind, std::string_view src, bool optimize) -> std::vector<uint32_t> {
    auto compiler = shaderc::Compiler();
    auto options = shaderc::CompileOptions();
    
    for(auto& macro : this->macros)
      options.AddMacroDefinition(macro);
    
    if(optimize) options.SetOptimizationLevel(shaderc_optimization_level_performance);
    
    auto result = compiler.AssembleToSpv(src.begin(), src.size(), options);
    
    OhmException(result.GetCompilationStatus() != shaderc_compilation_status_success, Error::LogicError, "Failed to preprocess shader.");
    
    return {result.cbegin(), result.cend()};
  }
  
  Shader::Shader() {
    this->data = std::make_shared<Shader::ShaderData>();
  }
  
  Shader::Shader(const std::vector<std::string>& glsl_to_load) {
    this->data = std::make_shared<Shader::ShaderData>();
    
//    for(auto& str : glsl_to_load) {
//    }
  }
  
  Shader::Shader(Shader&& mv) {
    this->data = std::make_shared<Shader::ShaderData>();
  }
  
  Shader::~Shader() {
    
  }
  
  auto Shader::operator=(Shader&& mv) -> Shader& {
    this->data = std::move(mv.data);
    return *this;
  }
  
  auto Shader::stages() -> const std::vector<Stage>& {
    return this->data->stages;
  }
  
  auto Shader::save(std::string_view path) -> bool {
    return true;
  }
  
  auto Shader::load(std::string_view path) -> bool {
    return true;
  }
}
}
}