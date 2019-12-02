/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/internal/trace/android/trace-manager-impl-android.h>
#include <dali/internal/system/common/performance-interface.h>

// INTERNAL INCLUDES

namespace Dali
{


namespace Internal
{

namespace Adaptor
{

TraceManagerAndroid* TraceManagerAndroid::traceManagerAndroid = nullptr;

TraceManagerAndroid::TraceManagerAndroid( PerformanceInterface* performanceInterface )
: TraceManager( performanceInterface )
{
  TraceManagerAndroid::traceManagerAndroid = this;
}

Dali::Integration::Trace::LogContextFunction TraceManagerAndroid::GetLogContextFunction()
{
  return LogContext;
}

void TraceManagerAndroid::LogContext( bool start, const char* tag )
{
  if( start )
  {
    unsigned short contextId = traceManagerAndroid->mPerformanceInterface->AddContext( tag );
    traceManagerAndroid->mPerformanceInterface->AddMarker( PerformanceInterface::START, contextId );
  }
  else
  {
    unsigned short contextId = traceManagerAndroid->mPerformanceInterface->AddContext( tag );
    traceManagerAndroid->mPerformanceInterface->AddMarker( PerformanceInterface::END, contextId );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
