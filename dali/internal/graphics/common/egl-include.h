#ifndef EGL_INCLUDE_H
#define EGL_INCLUDE_H

// EXTERNAL INCLUDES
#include <EGL/egl.h>

// Undef unneded symbols that fail to compile on MS Windows
#if defined(_WIN32) || defined(__VC32__) && !defined(__CYGWIN__) /* Win32 and WinCE */

#undef ERROR

#undef OPAQUE
#undef TRANSPARENT

#undef TRUE
#undef FALSE

#undef CopyMemory
#undef CreateWindow

#undef min
#undef max

#undef DIFFERENCE
#endif

#include <EGL/eglext.h>

#endif // EGL_INCLUDE_H
