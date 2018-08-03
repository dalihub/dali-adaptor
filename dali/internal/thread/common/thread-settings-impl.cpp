/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <dali/internal/thread/common/thread-settings-impl.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace ThreadSettings
{

void SetThreadName(const std::string& threadName)
{
  int err = prctl(PR_SET_NAME, threadName.c_str());
  if ( err )
  {
    DALI_LOG_ERROR( "prctl(PR_SET_NAME, %s) failed\n", threadName.c_str() );
  }
}

} // namespace ThreadSettings

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
