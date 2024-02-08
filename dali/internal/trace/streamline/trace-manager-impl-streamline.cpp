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
#include <dali/internal/trace/streamline/trace-manager-impl-streamline.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/system/common/performance-interface.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/trace/streamline/streamline_annotate.h>

ANNOTATE_DEFINE;

namespace Dali::Internal::Adaptor
{
namespace
{
const char* DALI_TRACE_ENABLE_PRINT_LOG_ENV = "DALI_TRACE_ENABLE_PRINT_LOG";
const char* EMPTY_TAG                       = "(null)";
bool        gTraceManagerEnablePrintLog     = false;
} // namespace

TraceManagerStreamline* TraceManagerStreamline::traceManagerStreamline = nullptr;

TraceManagerStreamline::TraceManagerStreamline(PerformanceInterface* performanceInterface)
: TraceManager(performanceInterface)
{
  ANNOTATE_SETUP;
  const char* enablePrintLog = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_TRACE_ENABLE_PRINT_LOG_ENV);
  if(enablePrintLog && std::atoi(enablePrintLog) != 0)
  {
    gTraceManagerEnablePrintLog = true;
  }

  TraceManagerStreamline::traceManagerStreamline = this;
}

Dali::Integration::Trace::LogContextFunction TraceManagerStreamline::GetLogContextFunction()
{
  return LogContext;
}

void TraceManagerStreamline::LogContext(bool start, const char* tag, const char* message)
{
  if(traceManagerStreamline)
  {
    if(start)
    {
      ANNOTATE(tag);
    }
    else
    {
      ANNOTATE_END();
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

} // namespace Dali::Internal::Adaptor
