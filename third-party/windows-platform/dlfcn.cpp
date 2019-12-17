#include <dlfcn.h>
#include <windows.h>

namespace
{
  char DL_ERROR[2] = "";
}

bool dlclose( void* handle )
{
  return true;
}

char* dlerror()
{
  return DL_ERROR;
}

void* dlopen( const char *name, int mode )
{
  const char* szStr = name;

  return LoadLibrary( szStr );
}

void* dlsym( void *handle, const char *name )
{
  return GetProcAddress( (HMODULE)handle, "CreateFeedbackPlugin" );
}
