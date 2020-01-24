#include <winsock2.h>

int __cdecl setsockopt( int s, int level, int optname, unsigned int * optval, unsigned int optlen )
{
  return setsockopt( (SOCKET)s, level, optname, (const char*)optval, optlen );
}

int __cdecl setsockopt( int s, int level, int optname, int * optval, unsigned int optlen )
{
  return setsockopt( (SOCKET)s, level, optname, (const char*)optval, optlen );
}