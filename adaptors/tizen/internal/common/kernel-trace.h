#ifndef __DALI_INTERNAL_ADAPTOR_KERNEL_TRACE_H__
#define __DALI_INTERNAL_ADAPTOR_KERNEL_TRACE_H__

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

#include <base/interfaces/kernel-trace-interface.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Concrete Kernel Tracing Interface.
 * Used to log trace messages to the kernel using ftrace.
 *
 */
class KernelTrace : public KernelTraceInterface
{
public:

  /**
   * Constructor
   */
  KernelTrace();

  /**
   * Destructor
   */
  virtual ~KernelTrace();

  /**
   * @copydoc KernelTracerInterface::KernelTrace()
   */
  virtual void Trace( const std::string& traceMessage );

private:

  int mFileDescriptor;
  bool mLoggedError:1;

};

} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif // __DALI_INTERNAL_ADAPTOR_KERNEL_TRACE_H__
