#pragma once
#include <map>
#include <string>
#include <vector>
#include <memory>

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
        Image,
        Sampler,
        ConstVariable,
        Storage,
      };
      size_t binding;
      std::string name;
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
}  // namespace v1
}  // namespace io

}  // namespace ohm