#ifndef _MMAN_INCLUDE_
#define _MMAN_INCLUDE_

#define PROT_READ   0
#define MAP_SHARED  1

void* mmap( void*, long long, int, int, int, int )
{
  return NULL;
}

void munmap( void*, long long )
{

}

#endif