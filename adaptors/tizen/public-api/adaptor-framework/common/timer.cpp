//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include <dali/public-api/adaptor-framework/common/timer.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>

// INTERNAL INCLUDES
#include <internal/common/timer-impl.h>

namespace Dali
{

Timer::Timer()
{
}

Timer Timer::New( unsigned int milliSec )
{
  Internal::Adaptor::TimerPtr internal = Internal::Adaptor::Timer::New( milliSec );
  return Timer(internal.Get());
}

Timer::Timer( const Timer& timer )
: BaseHandle(timer)
{
}

Timer& Timer::operator=( const Timer& timer )
{
  // check self assignment
  if( *this != timer )
  {
    BaseHandle::operator=(timer);
  }
  return *this;
}

Timer::~Timer()
{
}

void Timer::Start()
{
  Internal::Adaptor::GetImplementation(*this).Start();
}

void Timer::Stop()
{
  Internal::Adaptor::GetImplementation(*this).Stop();
}

void Timer::SetInterval( unsigned int interval )
{
  Internal::Adaptor::GetImplementation(*this).SetInterval( interval );
}

unsigned int Timer::GetInterval() const
{
  return Internal::Adaptor::GetImplementation(*this).GetInterval();
}

bool Timer::IsRunning() const
{
  return Internal::Adaptor::GetImplementation(*this).IsRunning();
}

Timer::TimerSignalV2& Timer::TickSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TickSignal();
}

Timer::Timer(Internal::Adaptor::Timer* timer)
: BaseHandle(timer)
{
}


} // namespace Dali
