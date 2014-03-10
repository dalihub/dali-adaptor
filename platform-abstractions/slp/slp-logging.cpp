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

// FUNCTION HEADER
#include "slp-logging.h"

// EXTERNAL INCLUDES
#include <dlog.h>

namespace Dali
{

namespace SlpPlatform
{

void LogMessage(Dali::Integration::Log::DebugPriority level, std::string& message)
{
  const char* DALI_TAG = "DALI";
  switch(level)
  {
    case Dali::Integration::Log::DebugInfo:
      LOG(LOG_INFO, DALI_TAG, "%s", message.c_str());
      break;
    case Dali::Integration::Log::DebugWarning:
      LOG(LOG_WARN, DALI_TAG, "%s", message.c_str());
      break;
    case Dali::Integration::Log::DebugError:
      LOG(LOG_ERROR, DALI_TAG, "%s", message.c_str());
      break;
    default:
      LOG(LOG_DEFAULT, DALI_TAG, "%s", message.c_str());
      break;
  }
}

} // namespace SlpPlatform

} // namespace Dali
