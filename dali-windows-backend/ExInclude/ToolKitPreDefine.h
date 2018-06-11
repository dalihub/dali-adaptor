#ifndef _TOOLKIT_PREDEFINE_INCLUDE_
#define _TOOLKIT_PREDEFINE_INCLUDE_

#ifdef __cplusplus
#include <cctype>

static unsigned int abs(unsigned int x)
{
  return (x)>=0?x:-x;
}

#include <random>
static unsigned int _seed = -1;

static int rand_r(unsigned int *seed)
{
  if (*seed != _seed)
  {
    _seed = *seed;
	srand(_seed);
  }
  
  return rand();
}

#endif

#endif