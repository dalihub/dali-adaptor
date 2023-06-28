/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <cstdio>

namespace Dali
{
namespace TizenPlatform
{
void LogMessage(Dali::Integration::Log::DebugPriority level, std::string& message)
{
  const char* DALI_TAG = "DALI";

  const char* format = NULL;
  switch(level)
  {
    case Dali::Integration::Log::DEBUG: ///< Gray color
      format = "\e[1;38;5;243mDEBUG:\e[21m %s: %s\e[0m";
      break;
    case Dali::Integration::Log::INFO: ///< Blue color
      format = "\e[1;34mINFO:\e[21m %s: %s\e[0m";
      break;
    case Dali::Integration::Log::WARNING: ///< Yellow color
      format = "\e[1;33mWARN:\e[21m %s: %s\e[0m";
      break;
    case Dali::Integration::Log::ERROR: ///< Bright Red color
      format = "\e[1;91mERROR:\e[21m %s: %s\e[0m";
      break;
    default:
      format = ":\e[21m %s: %s\e[0m";
      break;
  }

  // Append line feed if message is not end with line feed.
  if(message.size() == 0u || message.back() != '\n')
  {
    // Reserve to avoid doubled memory reallocation.
    message.reserve(message.size() + 1u);
    message.push_back('\n');
  }

  printf(format, DALI_TAG, message.c_str());
}

} // namespace TizenPlatform

} // namespace Dali
