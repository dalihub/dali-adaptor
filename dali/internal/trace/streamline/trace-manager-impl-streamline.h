#pragma once

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

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include <dali/internal/trace/common/trace-manager-impl.h>

namespace Dali::Internal::Adaptor
{
class PerformanceInterface;

class TraceManagerStreamline : public Dali::Internal::Adaptor::TraceManager
{
public:
  /**
   * Static member to hold TraceManagerStreamline instance. This allows
   * to access PerformanceInterface for network logging.
   */
  static TraceManagerStreamline* traceManagerStreamline;

  /**
   * Explicit Constructor
   */
  explicit TraceManagerStreamline(PerformanceInterface* performanceInterface);

protected:
  /**
   * Destructor
   */
  ~TraceManagerStreamline() override = default;

  /**
   * Obtain the LogContextFunction method used for tracing
   */
  Dali::Integration::Trace::LogContextFunction GetLogContextFunction() final;

private:
  /**
   * LogContext method (Generic specific) used for tracing
   */
  static void LogContext(bool start, const char* tag, const char* message);
};

} // namespace Dali::Internal::Adaptor
