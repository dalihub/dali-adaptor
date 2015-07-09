/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// CLASS HEADER
#include "conditional-wait.h"

// EXTERNAL INCLUDES
#include <pthread.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
} // unnamed namespace

struct ConditionalWait::ConditionalWaitImpl
{
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  volatile unsigned int count;
};

ConditionalWait::ConditionalWait()
: mImpl( new ConditionalWaitImpl )
{
  pthread_mutex_init( &mImpl->mutex, NULL );
  pthread_cond_init( &mImpl->condition, NULL );
  mImpl->count = 0;
}

ConditionalWait::~ConditionalWait()
{
  pthread_cond_destroy( &mImpl->condition );
  pthread_mutex_destroy( &mImpl->mutex );
  delete mImpl;
}

void ConditionalWait::Notify()
{
  // pthread_cond_wait requires a lock to be held
  pthread_mutex_lock( &mImpl->mutex );
  volatile unsigned int previousCount = mImpl->count;
  mImpl->count = 0; // change state before broadcast as that may wake clients immediately
  // broadcast does nothing if the thread is not waiting but still has a system call overhead
  // broadcast all threads to continue
  if( 0 != previousCount )
  {
    pthread_cond_broadcast( &mImpl->condition );
  }
  pthread_mutex_unlock( &mImpl->mutex );
}

void ConditionalWait::Wait()
{
  // pthread_cond_wait requires a lock to be held
  pthread_mutex_lock( &mImpl->mutex );
  ++(mImpl->count);
  // pthread_cond_wait may wake up without anyone calling Notify
  do
  {
    // wait while condition changes
    pthread_cond_wait( &mImpl->condition, &mImpl->mutex ); // releases the lock whilst waiting
  }
  while( 0 != mImpl->count );
  // when condition returns the mutex is locked so release the lock
  pthread_mutex_unlock( &mImpl->mutex );
}

unsigned int ConditionalWait::GetWaitCount() const
{
  return mImpl->count;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
