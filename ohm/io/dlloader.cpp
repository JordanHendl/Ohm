#include "dlloader.h"
#include <iostream>
#include <map>

#ifdef _WIN32
#include <windows.h>
typedef UINT(CALLBACK* LPFNDLLFUNC1)(DWORD, UINT);
using LibHandle = HINSTANCE;
using SymHandle = LPFNDLLFUNC1;

static inline LibHandle loadSharedObject(const char* input) {
  return LoadLibrary(input);
}

static inline const char* getError() {
  return "Windows Error handler not yet implemented.";
}

static inline SymHandle loadSymbol(LibHandle& handle, const char* symbol_name) {
  return reinterpret_cast<SymHandle>(GetProcAddress(handle, symbol_name));
}

static inline void releaseHandle(LibHandle handle) { FreeLibrary(handle); }

#elif __linux__
#include <dlfcn.h>
using LibHandle = void*;
using SymHandle = void*;
static inline LibHandle loadSharedObject(const char* input) {
  return dlopen(input, RTLD_LAZY);
}

static inline const char* getError() { return dlerror(); }

static inline SymHandle loadSymbol(LibHandle& handle, const char* symbol_name) {
  return dlsym(handle, symbol_name);
}

static inline void releaseHandle(LibHandle handle) { dlclose(handle); }

#endif

namespace ohm {
namespace io {
struct DlloaderData {
  using FunctionMap = std::map<std::string, Dlloader::DL_FUNC>;
  using SharedLibraryMap = std::map<std::string, bool>;

  LibHandle handle;
  FunctionMap map;

  DlloaderData() { this->handle = NULL; }
  ~DlloaderData() {
    if (this->handle) {
      ::releaseHandle(this->handle);
      this->handle = nullptr;
    }
  }
};

Dlloader::Dlloader() { this->loader_data = new DlloaderData(); }

Dlloader::~Dlloader() { delete this->loader_data; }

Dlloader::DL_FUNC Dlloader::symbol(const char* symbol_name) {
  Dlloader::DL_FUNC func;

  if (data().map.find(symbol_name) == data().map.end()) {
    func = reinterpret_cast<Dlloader::DL_FUNC>(
        loadSymbol(data().handle, symbol_name));

    if (func) {
      data().map.insert({symbol_name, func});
    } else {
      // todo
      return nullptr;
    }
  }

  return data().map.at(symbol_name);
}

void Dlloader::load(const char* lib_path) {
  data().handle = loadSharedObject(lib_path);

  if (data().handle == nullptr) {
    // todo
  }
}

bool Dlloader::initialized() const { return data().handle != nullptr; }
void Dlloader::reset() {
  if (data().handle) ::releaseHandle(data().handle);
}

DlloaderData& Dlloader::data() { return *this->loader_data; }

const DlloaderData& Dlloader::data() const { return *this->loader_data; }
}  // namespace io
}  // namespace ohm
