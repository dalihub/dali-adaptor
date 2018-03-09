#ifndef _DLFCN_INCLUDE_
#define _DLFCN_INCLUDE_

#undef PlaySound

#define RTLD_NOW      0
#define RTLD_GLOBAL   1
#define RTLD_LAZY     2

bool dlclose( void* handle );

char* dlerror();

void* dlopen( const char *name, int mode );

void* dlsym( void *handle, const char *name );

#endif