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
  volatile bool wait;
};

ConditionalWait::ConditionalWait()
: mImpl( new ConditionalWaitImpl )
{
  pthread_mutex_init( &mImpl->mutex, NULL );
  pthread_cond_init( &mImpl->condition, NULL );
  mImpl->wait = false;
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
  bool wasWaiting = mImpl->wait;
  mImpl->wait = false;
  pthread_mutex_unlock( &mImpl->mutex );
  // broadcast does nothing if the thread is not waiting but still has a system call overhead
  // broadcast all threads to continue
  if( wasWaiting )
  {
    pthread_cond_broadcast( &mImpl->condition );
  }
}

void ConditionalWait::Wait()
{
  // pthread_cond_wait requires a lock to be held
  pthread_mutex_lock( &mImpl->mutex );
  mImpl->wait = true;
  // pthread_cond_wait may wake up without anyone calling Notify
  while( mImpl->wait )
  {
    // wait while condition changes
    pthread_cond_wait( &mImpl->condition, &mImpl->mutex ); // releases the lock whilst waiting
  }
  // when condition returns the mutex is locked so release the lock
  pthread_mutex_unlock( &mImpl->mutex );
}

bool ConditionalWait::IsWaiting() const
{
  bool isWaiting( false );
  pthread_mutex_lock( &mImpl->mutex );
  isWaiting = mImpl->wait;
  pthread_mutex_unlock( &mImpl->mutex );
  return isWaiting;
}



} // namespace Adaptor

} // namespace Internal

} // namespace Dali
