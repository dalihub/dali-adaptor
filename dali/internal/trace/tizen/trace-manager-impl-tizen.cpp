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

// EXTERNAL INCLUDES
#include <ttrace.h>
#include <dali/internal/trace/tizen/trace-manager-impl-tizen.h>

// INTERNAL INCLUDES

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

TraceManagerTizen::TraceManagerTizen( PerformanceInterface* performanceInterface )
: TraceManager( performanceInterface )
{
}

Dali::Integration::Trace::LogContextFunction TraceManagerTizen::GetLogContextFunction()
{
  return LogContext;
}

void TraceManagerTizen::LogContext( bool start, const char* tag )
{
  if( start )
  {
    traceBegin( TTRACE_TAG_GRAPHICS, tag );
  }
  else
  {
    traceEnd( TTRACE_TAG_GRAPHICS );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
