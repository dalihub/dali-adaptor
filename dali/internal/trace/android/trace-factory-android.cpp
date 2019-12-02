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

#include <dali/internal/trace/common/trace-factory.h>
#include <dali/internal/trace/android/trace-manager-impl-android.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace TraceManagerFactory
{

// TraceManager Factory to be implemented by the platform
TraceManagerUPtr CreateTraceFactory( PerformanceInterface* performanceInterface )
{
  return TraceManagerUPtr( new Dali::Internal::Adaptor::TraceManagerAndroid( performanceInterface ) );
}

} // namespace TraceManagerFactory

} // namespace Adaptor

} // namespace Internal

} // namespace Dali // namespace Dali
