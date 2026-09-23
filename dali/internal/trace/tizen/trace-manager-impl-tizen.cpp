/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/trace/tizen/trace-manager-impl-tizen.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <ttrace.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/system/common/environment-variables.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const char* EMPTY_TAG                   = "(null)";
static bool gTraceManagerEnablePrintLog = false;
static bool gTraceManagerEnableTTrace   = true;

} // namespace

TraceManagerTizen::TraceManagerTizen(PerformanceInterface* performanceInterface)
: TraceManager(performanceInterface)
{
  const char* enablePrintLog = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_TRACE_ENABLE_PRINT_LOG);
  if(enablePrintLog && std::atoi(enablePrintLog) != 0)
  {
    gTraceManagerEnablePrintLog = true;
  }

  const char* enableTTrace = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_TRACE_ENABLE_PLATFORM_TRACE);
  if(enableTTrace && std::atoi(enableTTrace) == 0)
  {
    gTraceManagerEnableTTrace = false;
  }
}

Dali::Integration::Trace::LogContextFunction TraceManagerTizen::GetLogContextFunction()
{
  return LogContext;
}

void TraceManagerTizen::LogContext(bool start, const char* tag, const char* message)
{
  if(start)
  {
    if(gTraceManagerEnableTTrace)
    {
      traceBegin(TTRACE_TAG_GRAPHICS, tag ? tag : EMPTY_TAG);
    }

    if(gTraceManagerEnablePrintLog)
    {
      DALI_LOG_DEBUG_INFO("BEGIN: %s%s%s\n", tag ? tag : EMPTY_TAG, message ? " " : "", message ? message : "");
    }
  }
  else
  {
    if(gTraceManagerEnableTTrace)
    {
      traceEnd(TTRACE_TAG_GRAPHICS);
    }

    if(gTraceManagerEnablePrintLog)
    {
      DALI_LOG_DEBUG_INFO("END: %s%s%s\n", tag ? tag : EMPTY_TAG, message ? " " : "", message ? message : "");
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE
