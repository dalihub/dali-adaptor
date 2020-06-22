#ifndef EGL_INCLUDE_H
#define EGL_INCLUDE_H

// EXTERNAL INCLUDES
#include <EGL/egl.h>

// Undef unneded symbols that fail to compile on MS Windows
#undef ERROR

#undef OPAQUE
#undef TRANSPARENT

#undef TRUE
#undef FALSE

#undef CopyMemory
#undef CreateWindow

#undef min
#undef max

#include <EGL/eglext.h>

#endif // EGL_INCLUDE_H
