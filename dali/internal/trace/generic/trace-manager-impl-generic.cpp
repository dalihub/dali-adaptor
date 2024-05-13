/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/trace/generic/trace-manager-impl-generic.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/system/common/performance-interface.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/system/common/environment-variables.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const char* EMPTY_TAG                   = "(null)";
static bool gTraceManagerEnablePrintLog = false;
} // namespace

TraceManagerGeneric* TraceManagerGeneric::traceManagerGeneric = nullptr;

TraceManagerGeneric::TraceManagerGeneric(PerformanceInterface* performanceInterface)
: TraceManager(performanceInterface)
{
  const char* enablePrintLog = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_TRACE_ENABLE_PRINT_LOG);
  if(enablePrintLog && std::atoi(enablePrintLog) != 0)
  {
    gTraceManagerEnablePrintLog = true;
  }

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

  if(gTraceManagerEnablePrintLog)
  {
    if(start)
    {
      DALI_LOG_DEBUG_INFO("BEGIN: %s%s%s\n", tag ? tag : EMPTY_TAG, message ? " " : "", message ? message : "");
    }
    else
    {
      DALI_LOG_DEBUG_INFO("END: %s%s%s\n", tag ? tag : EMPTY_TAG, message ? " " : "", message ? message : "");
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
