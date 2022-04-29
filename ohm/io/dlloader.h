#pragma once
#include <utility>
#include <memory>

namespace ohm {
namespace io {
/** A Class to manage symbols loaded from a Dynamic Library.
 */
template <class RETURN, class... ARGS>
class Symbol {
 public:
  /** Default Constructor.
   */
  Symbol();

  /** Default Deconstructor.
   */
  ~Symbol() = default;

  /** Conversion operator to check for validity of this object.
   * @return Whether this object is valid or not.
   */
  operator bool() const;

  /** Paren operator to call this symbol as a function.
   * @param args The arguments specified in the template parameters.
   * @return The return type that this object's template parameters specify.
   */
  RETURN operator()(ARGS... args);

  /** Assignment operator.
   * @param func The function pointer to associate with this object.
   * @return This object after the assignment operation.
   */
  Symbol& operator=(void (*func)());

 private:
  RETURN (*func)(ARGS... args);
};

/** A Class to manage loading of shared objects from a .DLL / .SO.
 */
class Dlloader {
 public:
  typedef void (*DL_FUNC)();

  /** Default Constructor.
   */
  Dlloader();

  /** Default Deconstructor. Releases this object's allocated data.
   */
  ~Dlloader();

  /** Method to load the shared object at the specified path.
   * @param lib_path The path to use for loading the shared object.
   */
  void load(const char* lib_path);

  /** Method to retrieve whether this object has loaded symbols or not.
   * @return Whether or not this object has loaded a library or not.
   */
  bool initialized() const;

  /** Method to reset this object and release all allocated data and opened
   * shared objects.
   */
  void reset();

  /** Method to return a function pointer to a symbol loaded by a Shared Object.
   * @param symbol_name The name of symbol to attempt to load.
   * @return The function pointer associated with the loaded symbol. Returns
   * nullptr if the symbol is not valid.
   */
  DL_FUNC symbol(const char* symbol_name);

 private:
  /** The forward declared structure containing this object's data.
   */
  struct DlloaderData;
  
  std::shared_ptr<DlloaderData> m_data;

  /** Method to retrieve a reference to this object's internal data structure.
   * @return A reference to this object's internal data structure.
   */
  DlloaderData& data();

  /** Method to retrieve a reference to this object's internal data structure.
   * @return A reference to this object's internal data structure.
   */
  const DlloaderData& data() const;
};

template <class RETURN, class... ARGS>
Symbol<RETURN, ARGS...>::Symbol() {
  this->func = nullptr;
}

template <class RETURN, class... ARGS>
Symbol<RETURN, ARGS...>::operator bool() const {
  if (this->func == nullptr) return false;
  return true;
}

template <class RETURN, class... ARGS>
RETURN Symbol<RETURN, ARGS...>::operator()(ARGS... args) {
  return std::forward((this->func)(args...));
}

template <class RETURN, class... ARGS>
Symbol<RETURN, ARGS...>& Symbol<RETURN, ARGS...>::operator=(void (*func)()) {
  this->func = reinterpret_cast<RETURN (*)(ARGS...)>(func);
  return *this;
}
}  // namespace io
}  // namespace ohm
