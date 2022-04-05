#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "event.h"
#include "ohm/api/event.h"
#include "image.h"
namespace ohm {
namespace ovk {
auto keyFromSDL( Sint32 key ) -> Key
{
  switch( key )
  {
    case SDLK_ESCAPE      : return Key::ESC          ;
    case SDLK_F1          : return Key::F1           ;
    case SDLK_F2          : return Key::F2           ;
    case SDLK_F3          : return Key::F3           ;
    case SDLK_F4          : return Key::F4           ;
    case SDLK_F5          : return Key::F5           ;
    case SDLK_F6          : return Key::F6           ;
    case SDLK_F7          : return Key::F7           ;
    case SDLK_F8          : return Key::F8           ;
    case SDLK_F9          : return Key::F9           ;
    case SDLK_F10         : return Key::F10          ;
    case SDLK_F11         : return Key::F11          ;
    case SDLK_F12         : return Key::F12          ;
    case SDLK_1           : return Key::One          ;
    case SDLK_2           : return Key::Two          ;
    case SDLK_3           : return Key::Three        ;
    case SDLK_4           : return Key::Four         ;
    case SDLK_5           : return Key::Five         ;
    case SDLK_6           : return Key::Six          ;
    case SDLK_7           : return Key::Seven        ;
    case SDLK_8           : return Key::Eight        ;
    case SDLK_9           : return Key::Nine         ;
    case SDLK_0           : return Key::Zero         ;
    case SDLK_EQUALS      : return Key::Equals       ;
    case SDLK_MINUS       : return Key::Hyphen       ;
    case SDLK_TAB         : return Key::Tab          ;
    case SDLK_q           : return Key::Q            ;
    case SDLK_w           : return Key::W            ;
    case SDLK_e           : return Key::E            ;
    case SDLK_r           : return Key::R            ;
    case SDLK_t           : return Key::T            ;
    case SDLK_y           : return Key::Y            ;
    case SDLK_u           : return Key::U            ;
    case SDLK_i           : return Key::I            ;
    case SDLK_o           : return Key::O            ;
    case SDLK_p           : return Key::P            ;
    case SDLK_LEFTBRACKET : return Key::LBracket     ;
    case SDLK_RIGHTBRACKET: return Key::RBracket     ;
    case SDLK_RETURN      : return Key::Return       ;
    case SDLK_LCTRL       : return Key::LCtrl        ;
    case SDLK_RCTRL       : return Key::RCtrl        ;
    case SDLK_LALT        : return Key::LAlt         ;
    case SDLK_RALT        : return Key::RAlt         ;
    case SDLK_BACKSPACE   : return Key::Backspace    ;
    case SDLK_a           : return Key::A            ;
    case SDLK_s           : return Key::S            ;
    case SDLK_d           : return Key::D            ;
    case SDLK_f           : return Key::F            ;
    case SDLK_g           : return Key::G            ;
    case SDLK_h           : return Key::H            ;
    case SDLK_j           : return Key::J            ;
    case SDLK_k           : return Key::K            ;
    case SDLK_l           : return Key::L            ;
    case SDLK_SEMICOLON   : return Key::Semicolon    ;
    case SDLK_QUOTE       : return Key::Quote        ;
    case SDLK_BACKQUOTE   : return Key::Backtick     ;
    case SDLK_LSHIFT      : return Key::LShift       ;
    case SDLK_RSHIFT      : return Key::RShift       ;
    case SDLK_BACKSLASH   : return Key::Backslash    ;
    case SDLK_z           : return Key::Z            ;
    case SDLK_x           : return Key::X            ;
    case SDLK_c           : return Key::C            ;
    case SDLK_v           : return Key::V            ;
    case SDLK_b           : return Key::B            ;
    case SDLK_n           : return Key::N            ;
    case SDLK_m           : return Key::M            ;
    case SDLK_COMMA       : return Key::Comma        ;
    case SDLK_PERIOD      : return Key::Period       ;
    case SDLK_SLASH       : return Key::Forwardslash ;
    case SDLK_SPACE       : return Key::Space        ; 
    
    case SDLK_UP    : return Key::Up    ;
    case SDLK_LEFT  : return Key::Left  ;
    case SDLK_RIGHT : return Key::Right ;
    case SDLK_DOWN  : return Key::Down  ;
    
    default  : return Key::None ;
  }
}
auto mouseButtonFromSDL( Uint8 key ) -> MouseButton
{
  switch( key )
  {
    case SDL_BUTTON_LEFT   : return MouseButton::LeftClick   ;
    case SDL_BUTTON_RIGHT  : return MouseButton::MiddleClick ;
    case SDL_BUTTON_MIDDLE : return MouseButton::RightClick  ;
    case SDL_BUTTON_X1     : return MouseButton::Button01    ;
    case SDL_BUTTON_X2     : return MouseButton::Button02    ;
    default : return MouseButton::None                       ;
  }
}
auto convert( SDL_Event& event ) -> ohm::Event
{
  auto lib_event = ohm::Event();
  
  switch( event.type )
  {
    case SDL_KEYDOWN :
    {
      lib_event = ohm::Event( ohm::Event::Type::KeyDown, keyFromSDL( event.key.keysym.sym ) ) ;
      break ;
    }
    case SDL_KEYUP :
    {
      lib_event = ohm::Event( ohm::Event::Type::KeyUp, keyFromSDL( event.key.keysym.sym ) ) ;
      break ;
    }
    case SDL_MOUSEBUTTONDOWN :
    {
      lib_event = ohm::Event( ohm::Event::Type::MouseButtonDown, mouseButtonFromSDL( event.button.button ) ) ;
      break ;
    }
    case SDL_MOUSEBUTTONUP :
    {
      lib_event = ohm::Event( ohm::Event::Type::MouseButtonUp, mouseButtonFromSDL( event.button.button ) ) ;
      break ;
    }
    case SDL_QUIT :
    {
      lib_event = ohm::Event( ohm::Event::Type::WindowExit, Key::ESC ) ;
      break ;
    }
    default : ;
  }
  
  return lib_event;
}

Event::Event() {
  
}

Event::Event(Event&& mv) {
  *this = std::move(mv);
}

Event::~Event() {
  
}

auto Event::operator=(Event&& mv) -> Event& {
  this->m_callbacks = std::move(mv.m_callbacks);
  return *this;
}

auto Event::add(std::function<void(const ohm::Event&)> cb) -> void {
  this->m_callbacks.push_back(cb);
}

auto Event::push(SDL_Event& e) -> void {
  auto event = convert(e);
  for(auto& cb : this->m_callbacks) {
    cb(event);
  }
}
}
}