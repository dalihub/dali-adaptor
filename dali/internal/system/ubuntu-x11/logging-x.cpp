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
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <cstdio>

// INTERNAL INCLUDES
#include <dali/internal/system/common/environment-variables.h>

namespace Dali
{
namespace TizenPlatform
{
namespace
{
static Dali::Integration::Log::DebugPriority gPrintLogLevel = Dali::Integration::Log::DebugPriority::INFO;

Dali::Integration::Log::DebugPriority GetAllowedPrintLogLevel()
{
  static bool gEnvironmentApplied = false;

  if(DALI_UNLIKELY(!gEnvironmentApplied))
  {
    gEnvironmentApplied = true;

    const char* printLogLevel = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_PRINT_LOG_LEVEL);
    if(printLogLevel)
    {
      auto logLevelInteger = std::strtoul(printLogLevel, nullptr, 10);
      if(logLevelInteger >= static_cast<unsigned long>(Dali::Integration::Log::DebugPriority::DEBUG) && logLevelInteger <= static_cast<unsigned long>(Dali::Integration::Log::DebugPriority::ERROR))
      {
        gPrintLogLevel = static_cast<Dali::Integration::Log::DebugPriority>(logLevelInteger);
      }
    }
  }

  return gPrintLogLevel;
}
} // namespace
void LogMessage(Dali::Integration::Log::DebugPriority level, std::string& message)
{
  const char* DALI_TAG = "DALI";

  const char* format = NULL;
  if(level < GetAllowedPrintLogLevel())
  {
    return;
  }

  switch(level)
  {
    case Dali::Integration::Log::DEBUG: ///< Gray color
      format = "\e[1;38;5;243mDEBUG:\e[21m %s: %s\e[0m";
      break;
    case Dali::Integration::Log::INFO: ///< Green color
      format = "\e[1;32mINFO:\e[21m %s: %s\e[0m";
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
