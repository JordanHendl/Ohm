#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ohm/api/image.h"

namespace ohm {
namespace io {
inline namespace v1 {
/** Class to handle combining multiple glsl/hlsl shaders into a reflected binary
 * compacted with SPIR-V. This object handles configuring and saving as a binary
 * (denoted by an .osh file), or loading that configuration and reading it.
 */
class Shader {
 public:
  enum class Type : int {
    None,
    Vertex,
    Fragment,
    Geometry,
    TesselationControl,
    TesselationEval,
    Compute,
  };

  struct Stage {
    struct Attribute {
      enum class Type : int {
        Undefined,
        eInt,
        eFloat,
        eVec2,
        eIVec2,
        eVec3,
        eIVec3,
        eVec4,
        eIVec4,
        eMat2,
        eMat3,
        eMat4,
      };

      std::string name;
      Type type;
      size_t location;
    };

    struct Variable {
      enum class Type : int {
        None,
        Sampler,
        Sampled,
        Image,
        UTexel,
        STexel,
        Uniform,
        Storage,
        UniformDynamic,
        StorateDynamic,
        Input,
        Acceleration,
      };

      size_t set;
      size_t binding;
      size_t size;
      Type type;
    };

    Type type;
    std::string name;
    std::map<std::string, Variable> variables;
    std::vector<uint32_t> spirv;
    std::vector<Attribute> in_attributes;
    std::vector<Attribute> out_attributes;
  };

  explicit Shader();
  explicit Shader(std::string_view osh_file);
  explicit Shader(const std::vector<std::string>& glsl_to_load);
  explicit Shader(
      const std::vector<std::pair<std::string, std::string>>& inline_files);
  explicit Shader(Shader&& mv);
  ~Shader();
  auto operator=(Shader&& mv) -> Shader&;
  auto stages() -> const std::vector<Stage>&;
  auto save(std::string_view path) -> bool;
  auto load(std::string_view path) -> bool;

 private:
  struct ShaderData;
  std::shared_ptr<ShaderData> data;
};

using ShaderType = Shader::Type;
using VariableType = Shader::Stage::Variable::Type;
using AttributeType = Shader::Stage::Attribute::Type;
}  // namespace v1
}  // namespace io

}  // namespace ohm