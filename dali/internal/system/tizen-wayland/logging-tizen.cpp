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

// FUNCTION HEADER
#include <dali/internal/system/common/logging.h>

// EXTERNAL INCLUDES
// Dlog uses C style casts internally
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <dlog.h>

namespace Dali
{
namespace TizenPlatform
{
void LogMessage(Dali::Integration::Log::DebugPriority level, std::string& message)
{
  const char* DALI_TAG = "DALI";

  switch(level)
  {
    case Dali::Integration::Log::DebugDebug:
      print_log(DLOG_DEBUG, DALI_TAG, "%s", message.c_str());
      break;
    case Dali::Integration::Log::DebugInfo:
      print_log(DLOG_INFO, DALI_TAG, "%s", message.c_str());
      break;
    case Dali::Integration::Log::DebugWarning:
      print_log(DLOG_WARN, DALI_TAG, "%s", message.c_str());
      break;
    case Dali::Integration::Log::DebugError:
      print_log(DLOG_ERROR, DALI_TAG, "%s", message.c_str());
      break;
    default:
      print_log(DLOG_DEFAULT, DALI_TAG, "%s", message.c_str());
      break;
  }
#pragma GCC diagnostic pop
}

} // namespace TizenPlatform

} // namespace Dali
