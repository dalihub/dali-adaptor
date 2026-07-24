#include <dlfcn.h>
#include <windows.h>

#include <string>
#include <vector>

namespace
{
thread_local std::string gLastError;
thread_local std::string gReturnedError;
const int                gModuleAddressAnchor = 0;

void SetLastErrorMessage(const char* operation, const char* subject, DWORD errorCode)
{
  char* systemMessage = nullptr;
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 nullptr,
                 errorCode,
                 0u,
                 reinterpret_cast<char*>(&systemMessage),
                 0u,
                 nullptr);

  gLastError = operation;
  if(subject && subject[0] != '\0')
  {
    gLastError += " (";
    gLastError += subject;
    gLastError += ")";
  }
  gLastError += " failed with error ";
  gLastError += std::to_string(errorCode);
  if(systemMessage)
  {
    gLastError += ": ";
    gLastError += systemMessage;
    LocalFree(systemMessage);
  }
}

std::string GetWindowsLibraryName(const char* name)
{
  std::string path(name ? name : "");
  const auto  separator = path.find_last_of("/\\");
  const auto  fileStart = separator == std::string::npos ? 0u : separator + 1u;

  if(path.compare(fileStart, 3u, "lib") == 0)
  {
    path.erase(fileStart, 3u);
  }

  const auto sharedObjectExtension = path.find(".so", fileStart);
  if(sharedObjectExtension != std::string::npos)
  {
    path.erase(sharedObjectExtension);
    path += ".dll";
  }
  return path;
}

HMODULE LoadLibraryFromThisModuleDirectory(const std::string& libraryName)
{
  if(libraryName.empty() || libraryName.find_first_of("/\\") != std::string::npos)
  {
    return nullptr;
  }

  HMODULE thisModule = nullptr;
  if(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                        reinterpret_cast<LPCSTR>(&gModuleAddressAnchor),
                        &thisModule) == FALSE)
  {
    return nullptr;
  }

  std::vector<char> modulePath(MAX_PATH);
  DWORD             pathLength = 0u;
  bool              complete   = false;
  while(modulePath.size() <= 32768u)
  {
    pathLength = GetModuleFileNameA(thisModule, modulePath.data(), static_cast<DWORD>(modulePath.size()));
    if(pathLength == 0u)
    {
      return nullptr;
    }
    if(pathLength + 1u < modulePath.size())
    {
      complete = true;
      break;
    }
    if(modulePath.size() == 32768u)
    {
      break;
    }
    const size_t nextSize = modulePath.size() * 2u;
    modulePath.resize(nextSize < 32768u ? nextSize : 32768u);
  }

  if(!complete)
  {
    return nullptr;
  }

  std::string fullPath(modulePath.data(), pathLength);
  const auto  separator = fullPath.find_last_of("/\\");
  if(separator == std::string::npos)
  {
    return nullptr;
  }
  fullPath.resize(separator + 1u);
  fullPath += libraryName;
  return LoadLibraryA(fullPath.c_str());
}
} // unnamed namespace

int dlclose(void* handle)
{
  gLastError.clear();
  if(!handle)
  {
    SetLastErrorMessage("FreeLibrary", nullptr, ERROR_INVALID_HANDLE);
    return -1;
  }

  if(FreeLibrary(static_cast<HMODULE>(handle)) == FALSE)
  {
    SetLastErrorMessage("FreeLibrary", nullptr, GetLastError());
    return -1;
  }
  return 0;
}

char* dlerror()
{
  if(gLastError.empty())
  {
    return nullptr;
  }

  gReturnedError.swap(gLastError);
  gLastError.clear();
  return gReturnedError.data();
}

void* dlopen(const char* name, int mode)
{
  (void)mode;
  gLastError.clear();

  if(!name)
  {
    HMODULE module = nullptr;
    if(GetModuleHandleExA(0u, nullptr, &module) == FALSE)
    {
      SetLastErrorMessage("GetModuleHandleEx", nullptr, GetLastError());
      return nullptr;
    }
    return module;
  }

  const std::string windowsName = GetWindowsLibraryName(name);
  if(HMODULE module = LoadLibraryFromThisModuleDirectory(windowsName))
  {
    return module;
  }

  HMODULE module = LoadLibraryA(name);
  if(module)
  {
    return module;
  }

  if(windowsName != name)
  {
    module = LoadLibraryA(windowsName.c_str());
    if(module)
    {
      return module;
    }
  }

  SetLastErrorMessage("LoadLibrary", windowsName.c_str(), GetLastError());
  return nullptr;
}

void* dlsym(void* handle, const char* name)
{
  gLastError.clear();
  if(!name)
  {
    SetLastErrorMessage("GetProcAddress", nullptr, ERROR_INVALID_PARAMETER);
    return nullptr;
  }

  HMODULE module = handle ? static_cast<HMODULE>(handle) : GetModuleHandleA(nullptr);
  FARPROC symbol = module ? GetProcAddress(module, name) : nullptr;
  if(!symbol)
  {
    SetLastErrorMessage("GetProcAddress", name, GetLastError());
    return nullptr;
  }
  return reinterpret_cast<void*>(symbol);
}
