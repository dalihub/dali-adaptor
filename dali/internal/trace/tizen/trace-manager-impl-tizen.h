#ifndef DALI_INTERNAL_TRACE_MANAGER_IMPL_TIZEN_H
#define DALI_INTERNAL_TRACE_MANAGER_IMPL_TIZEN_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

class TraceManagerTizen : public Dali::Internal::Adaptor::TraceManager
{
public:
  /**
   * Explicit Constructor
   */
  explicit TraceManagerTizen(PerformanceInterface* performanceInterface);

protected:
  /**
   * Destructor
   */
  ~TraceManagerTizen() override = default;

  /**
   * Obtain the LogContextFunction method (Tizen specific) used for tracing
   */
  Dali::Integration::Trace::LogContextFunction GetLogContextFunction() final;

private:
  /**
   * LogContext method (Tizen specific) used for tracing
   */
  static void LogContext(bool start, const char* tag);
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TRACE_MANAGER_IMPL_TIZEN_H
