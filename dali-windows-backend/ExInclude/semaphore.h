#ifndef __SEMAPHORE_INCLUDE_
#define __SEMAPHORE_INCLUDE_

#include <pthread.h>


typedef long sem_t;

void sem_init( sem_t *sem, int p1, int p2 );

void sem_post( sem_t *sem );

void sem_wait( sem_t *sem );

long sem_timedwait( sem_t *sem, timespec *time );

#endif