#include <Win32WindowSystem.h>

#define CRYPTO_LOCK 0xFFFFFFFF

int CRYPTO_num_locks()
{
  return 1;
}

void CRYPTO_set_id_callback( unsigned long (*func)() )
{

}

void CRYPTO_set_locking_callback(void (*func)( int mode, int n, const char* file, int line ) )
{

}