#ifndef DALI_INTERNAL_TRACE_MANAGER_IMPL_UBUNTU_H
#define DALI_INTERNAL_TRACE_MANAGER_IMPL_UBUNTU_H

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

// INTERNAL INCLUDES
#include <dali/internal/trace/common/trace-manager-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
class PerformanceInterface;

class TraceManagerGeneric : public Dali::Internal::Adaptor::TraceManager
{
public:
  /**
   * Static member to hold TraceManagerUbuntu instance. This allows
   * to access PerformanceInterface for network logging.
   */
  static TraceManagerGeneric* traceManagerGeneric;

  /**
   * Explicit Constructor
   */
  explicit TraceManagerGeneric( PerformanceInterface* performanceInterface );

protected:
  /**
   * Destructor
   */
  ~TraceManagerGeneric() override = default;

  /**
   * Obtain the LogContextFunction method (Ubuntu specific) used for tracing
   */
  virtual Dali::Integration::Trace::LogContextFunction GetLogContextFunction() final;

private:

  /**
   * LogContext method (Ubuntu specific) used for tracing
   */
  static void LogContext( bool start, const char* tag );
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TRACE_MANAGER_IMPL_UBUNTU_H
