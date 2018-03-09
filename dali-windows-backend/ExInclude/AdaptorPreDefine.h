#ifndef ADAPTOR_PREDEFINE_INCLUDE
#define ADAPTOR_PREDEFINE_INCLUDE

#ifdef __cplusplus
static unsigned long abs(unsigned long x)
{
  return (x)>=0?x:-x;
}

#ifdef _MSC_VER

#include <xlocale>

static int strncasecmp(const char *s1, const char *s2, register int n)
{
  while (--n >= 0 && toupper((unsigned char)*s1) == toupper((unsigned char)*s2++))
      if (*s1++ == 0)  return 0;
  return(n < 0 ? 0 : toupper((unsigned char)*s1) - toupper((unsigned char)*--s2));
}

#endif

int setsockopt( int s, int level, int optname, unsigned int * optval, unsigned int optlen );

int setsockopt( int s, int level, int optname, int * optval, unsigned int optlen );

#endif

#endif