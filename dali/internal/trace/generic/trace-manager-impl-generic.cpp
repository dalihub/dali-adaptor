/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include "trace-manager-impl-generic.h"
#include <dali/internal/system/common/performance-interface.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
TraceManagerGeneric* TraceManagerGeneric::traceManagerGeneric = nullptr;

TraceManagerGeneric::TraceManagerGeneric(PerformanceInterface* performanceInterface)
: TraceManager(performanceInterface)
{
  TraceManagerGeneric::traceManagerGeneric = this;
}

Dali::Integration::Trace::LogContextFunction TraceManagerGeneric::GetLogContextFunction()
{
  return LogContext;
}

void TraceManagerGeneric::LogContext(bool start, const char* tag, const char* message)
{
  if(traceManagerGeneric && traceManagerGeneric->mPerformanceInterface)
  {
    if(start)
    {
      unsigned short contextId = traceManagerGeneric->mPerformanceInterface->GetContextId(tag);
      if(!contextId)
      {
        contextId = traceManagerGeneric->mPerformanceInterface->AddContext(tag);
      }
      traceManagerGeneric->mPerformanceInterface->AddMarker(PerformanceInterface::START, contextId);
    }
    else
    {
      unsigned short contextId = traceManagerGeneric->mPerformanceInterface->GetContextId(tag);
      traceManagerGeneric->mPerformanceInterface->AddMarker(PerformanceInterface::END, contextId);
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
