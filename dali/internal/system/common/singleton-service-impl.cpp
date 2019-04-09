/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/singleton-service-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/core.h>
#include <dali/integration-api/adaptor.h>
#include <dali/integration-api/processor-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

// INTERNAL INCLUDES
#if defined(DEBUG_ENABLED)
#include <dali/internal/system/common/logging.h>
Debug::Filter* gSingletonServiceLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_SINGLETON_SERVICE" );

// Need to define own macro as the log function is not installed when this object is created so no logging is shown with DALI_LOG_INFO at construction and destruction
#define DALI_LOG_SINGLETON_SERVICE_DIRECT(level, message)                        \
    if(gSingletonServiceLogFilter && gSingletonServiceLogFilter->IsEnabledFor(level)) { std::string string(message); Dali::TizenPlatform::LogMessage( Debug::DebugInfo, string );  }

#define DALI_LOG_SINGLETON_SERVICE(level, format, ...) DALI_LOG_INFO(gSingletonServiceLogFilter, level, format, ## __VA_ARGS__ )
#else

#define DALI_LOG_SINGLETON_SERVICE_DIRECT(level, message)
#define DALI_LOG_SINGLETON_SERVICE(level, format, ...)

#endif

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
thread_local SingletonService * gSingletonService = 0;
} // unnamed namespace

Dali::SingletonService SingletonService::New()
{
  Dali::SingletonService singletonService( new SingletonService );
  return singletonService;
}

Dali::SingletonService SingletonService::Get()
{
  Dali::SingletonService singletonService;
  if ( gSingletonService )
  {
    singletonService = Dali::SingletonService( gSingletonService );
  }
  return singletonService;
}

void SingletonService::Register( const std::type_info& info, BaseHandle singleton )
{
  if( singleton )
  {
    DALI_LOG_SINGLETON_SERVICE( Debug::General, "Singleton Added: %s\n", info.name() );
    mSingletonContainer.push_back( SingletonPair( info.name(), singleton ) );

    Integration::Processor* processor = dynamic_cast<Integration::Processor*>( &singleton.GetBaseObject() );
    if( processor )
    {
      Dali::Adaptor& adaptor = Dali::Adaptor::Get();
      Dali::Internal::Adaptor::Adaptor& adaptorImpl = Adaptor::GetImplementation( adaptor );
      Integration::Core& core = adaptorImpl.GetCore();
      core.RegisterProcessor( *processor );
    }
  }
}

void SingletonService::UnregisterAll( )
{
  mSingletonContainer.clear();
}

BaseHandle SingletonService::GetSingleton( const std::type_info& info ) const
{
  BaseHandle object;

  const SingletonContainer::const_iterator end = mSingletonContainer.end();
  for( SingletonContainer::const_iterator iter = mSingletonContainer.begin(); iter != end; ++iter )
  {
    // comparing the addresses as these are allocated statically per library
    if( ( *iter ).first == info.name() )
    {
      object = ( *iter ).second;
    }
  }

  return object;
}

SingletonService::SingletonService()
: mSingletonContainer()
{
  // Can only have one instance of SingletonService
  DALI_ASSERT_ALWAYS( !gSingletonService && "Only one instance of SingletonService is allowed");

  gSingletonService = this;

  DALI_LOG_SINGLETON_SERVICE_DIRECT( Debug::Concise, "SingletonService Created\n" );
}

SingletonService::~SingletonService()
{
  gSingletonService = 0;

  DALI_LOG_SINGLETON_SERVICE_DIRECT( Debug::Concise, "SingletonService Destroyed\n" );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
