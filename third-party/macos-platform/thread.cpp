#include <sys/prctl.h>
#include <pthread.h>

extern "C" int prctl( int type, const char *str )
{
  if (PR_SET_NAME == type)
  {
    pthread_setname_np(str);
  }
  return 0;
}
