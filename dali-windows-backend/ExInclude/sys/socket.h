#ifndef _SOCKET_INCLUDE_
#define _SOCKET_INCLUDE_

#include <winsock2.h>

#ifdef ERROR
#undef ERROR
#endif

#ifdef CopyMemory
#undef CopyMemory
#endif

#ifdef TRANSPARENT
#undef TRANSPARENT
#endif

typedef int socklen_t;

char* strerror_r(int, char *, int)
{
  return "";
}

int pipe( int* )
{
  return 1;
}

#endif