#ifndef DALI_INTERNAL_TRACE_MANAGER_IMPL_H
#define DALI_INTERNAL_TRACE_MANAGER_IMPL_H

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
#include <dali/integration-api/trace.h>

// INTERNAL INCLUDES


namespace Dali
{

namespace Internal
{

namespace Adaptor
{
class PerformanceInterface;

class TraceManager
{
public:

  /**
   * Explicit Constructor
   */
  explicit TraceManager( PerformanceInterface* performanceInterface );

  /**
   * Constructor
   */
  TraceManager() = default;

  /**
   * Virtual Destructor
   */
  virtual ~TraceManager() = default;

  /**
   * Second phase initialisation of TraceManager
   */
  bool Initialise();

  /**
   * Used for network server logging on Ubuntu
   */
  PerformanceInterface* mPerformanceInterface;

protected:

  /**
   * Obtain the platform dependent LogContextFunction method used for tracing
   */
  virtual Dali::Integration::Trace::LogContextFunction GetLogContextFunction() { return nullptr; };

private:

  /**
   * Set / Install the platform dependent trace function
   * @param logContextFunction - Platform dependent trace function
   */
  void SetTraceLogContextFunction( const Dali::Integration::Trace::LogContextFunction& logContextFunction );

  TraceManager( const TraceManager& ) = delete;
  TraceManager& operator=( TraceManager& )  = delete;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TRACE_MANAGER_IMPL_H
