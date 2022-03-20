#pragma once
#include "memory.h"
#include "system.h"
namespace ohm {
//  /** Texture formats
//   */
//  enum class Format : unsigned
//  {
//    R8,     ///< Single channel Char.
//    R32U,   ///< Single channel Integer.
//    R32I,   ///< Single channel Integer.
//    R32F,   ///< Single channel Float.
//    RG32F,  ///< Two channel Float.
//    RGB8,   ///< Three channel Char.
//    BGR8,   ///< Three channel Char.
//    RGB32U, ///< Three channel Integer.
//    RGB32I, ///< Three channel Integer.
//    RGB32F, ///< Three channel Float.
//    RGBA8,  ///< Four channel Char.
//    BGRA8,  ///< Three channel Char.
//    RGBA32I,///< Four channel Integer.
//    RGBA32U,///< Four channel Unsigned Integer.
//    RGBA32F,///< Four channel Float.
//    Depth,  ///< Depth Texture.
//  };
//
//  /** The type of filter algorithm to use.
//   */
//  enum class Filter : unsigned
//  {
//    Nearest,
//    Linear,
//    Cubic
//  };
//
//  /** The type of submition possible.
//   */
//  enum class SubmitType : unsigned
//  {
//    Graphics, ///< Graphics GPU queue's are used.
//    Compute,  ///< Compute GPU queue's are used.
//    Transfer, ///< Transfer GPU queue's are used.
//    Sparse    ///< Sparse GPU queue's are used.
//  };
//
//  /** Layouts for a texture to be in.
//   */
//  enum class Layout : unsigned
//  {
//    Undefined,       ///< Undefined.
//    General,         ///< General purpose, default.
//    ColorAttachment, ///< Color Attachment.
//    Depth,           ///< Depth Attachment.
//    ShaderRead,      ///< Only valid for shader read.
//    Present,         ///< Being presented to a swapchain.
//  };
//
//  /** Possible Primitive Topology of Vertices.
//   */
//  enum class Topology : unsigned
//  {
//    Point,
//    Line,
//    LineStrip,
//    Triangle,
//    TriangleStrip,
//  };
//
//  struct TextureInfo {
//    size_t m_num_mips ;
//    size_t m_width    ;
//    size_t m_height   ;
//    size_t m_layers   ;
//    Format m_format   ;
//    Layout m_layout   ;
//    bool   is_cubemap ;
//
//    TextureInfo() {
//      this->m_format   = Format::RGBA32F ;
//      this->m_layout   = Layout::General ;
//      this->m_width    = 1280            ;
//      this->m_height   = 1024            ;
//      this->m_layers   = 1               ;
//      this->m_num_mips = 1               ;
//      this->is_cubemap = false           ;
//    }
//  };
//
//  template<typename API, typename Allocator>
//  class Memory {
//    public:
//      Memory();
//    private:
//  };
//
//
//  class Texture {
//    public:
//      explicit Texture(TextureInfo& info);
//
//    private:
//      size_t handle;
//  };

}