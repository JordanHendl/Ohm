#pragma once
#include <exception>
#include <stdexcept>

namespace ohm {
enum class Error {
  None,
  LogicError,
  APIError,
  NullPointer,
};

class Exception : public std::exception {
 public:
  inline Exception();
  inline Exception(Error error, std::string_view str);
  inline auto what() const noexcept -> const char*;
  inline auto error() -> Error;

 private:
  std::string m_description;
  Error m_error;
};

/** Throw function declaration. On a debug build of the library, checks and throws.
 * On a non-debug build, the condition and exception handling is compiled out.
 */
#ifdef Ohm_Debug
#define OhmException(cond, error, description) \
  if (cond) {                                  \
    throw ohm::Exception(error, description);  \
  }
#else
#define OhmException(cond, error, description)
#endif

Exception::Exception() {
  this->m_description = "";
  this->m_error = Error::None;
}

Exception::Exception(Error error, std::string_view str) {
  this->m_description = str;
  this->m_error = error;
}

auto Exception::what() const noexcept -> const char* {
  return this->m_description.c_str();
}

auto Exception::error() -> Error { return this->m_error; }
}  // namespace ohm