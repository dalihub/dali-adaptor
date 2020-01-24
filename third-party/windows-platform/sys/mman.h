#ifndef _MMAN_INCLUDE_
#define _MMAN_INCLUDE_

#define PROT_READ   0
#define MAP_SHARED  1

static void* mmap( void*, long long length, int, int, int handle, int )
{
  char *buffer = new char[length];
  read( handle, buffer, length );
  return buffer;
}

static void munmap( void* buffer, long long )
{
  delete[] buffer;
}

#endif