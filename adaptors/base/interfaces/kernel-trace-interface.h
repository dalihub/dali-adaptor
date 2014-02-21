#ifndef __DALI_INTERNAL_BASE_KERNEL_TRACE_INTERFACE_H__
#define __DALI_INTERNAL_BASE_KERNEL_TRACE_INTERFACE_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <string>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Abstract Kernel Tracing Interface.
 * Used to log trace messages to the kernel.
 * E.g. On Linux this will use ftrace
 *
 */
class KernelTraceInterface
{

public:

  /**
   * Write a trace message
   * @param traceMessage trace message
   */
  virtual void Trace( const std::string& traceMessage ) = 0;

protected:

  /**
   * Constructor
   */
  KernelTraceInterface()
  {
  }

  /**
   * virtual destructor
   */
  virtual ~KernelTraceInterface()
  {
  }

  // Undefined copy constructor.
  KernelTraceInterface( const KernelTraceInterface& );

  // Undefined assignment operator.
  KernelTraceInterface& operator=( const KernelTraceInterface& );
};

} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif
