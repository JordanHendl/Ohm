#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

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
    TesselationEvaluation,
    Compute,
  };

  struct Stage {
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
      Type type;
    };

    Type type;
    std::string name;
    std::map<std::string, Variable> variables;
    std::vector<uint32_t> spirv;
    std::vector<std::string> in_attributes;
    std::vector<std::string> out_attributes;
  };

  explicit Shader();
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

using VariableType = Shader::Stage::Variable::Type;
}  // namespace v1
}  // namespace io

}  // namespace ohm