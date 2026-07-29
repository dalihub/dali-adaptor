#pragma once

#include <windows.h>

constexpr int RTLD_LAZY     = 0;
constexpr int RTLD_DEEPBIND = 0;

inline void* dlopen(const char* path, int)
{
  return reinterpret_cast<void*>(LoadLibraryA(path));
}
