#pragma once

#include <windows.h>

constexpr int RTLD_LAZY     = 0;
constexpr int RTLD_DEEPBIND = 0;
constexpr int RTLD_NOLOAD   = 4;

inline void* dlopen(const char* path, int mode)
{
  if(mode & RTLD_NOLOAD)
  {
    HMODULE module = nullptr;
    // GetModuleHandleEx adds a reference like dlopen(RTLD_NOLOAD).
    if(GetModuleHandleExA(0u, path, &module) == FALSE)
    {
      return nullptr;
    }
    return reinterpret_cast<void*>(module);
  }
  return reinterpret_cast<void*>(LoadLibraryA(path));
}

inline void* dlsym(void* handle, const char* name)
{
  return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(handle), name));
}

inline int dlclose(void* handle)
{
  return FreeLibrary(reinterpret_cast<HMODULE>(handle)) ? 0 : -1;
}
