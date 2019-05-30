#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_TRACE_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_TRACE_H

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

#include <dali/internal/network/common/trace-interface.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Concrete System  Tracing Interface.
 * Used to log trace messages to the system using ttrace
 *
 */
class SystemTrace : public TraceInterface
{
public:

  /**
   * Constructor
   */
  SystemTrace();

  /**
   * Destructor
   */
  virtual ~SystemTrace();

  /**
   * @copydoc KernelTracerInterface::KernelTrace()
   */
  virtual void Trace( const PerformanceMarker& marker, const std::string& traceMessage );

};

} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_TRACE_H
