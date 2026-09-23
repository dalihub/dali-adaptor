#ifndef DALI_TIZEN_LOGGING_H
#define DALI_TIZEN_LOGGING_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <cstdarg>
#include <cstdio>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace DALI_NAMESPACE
{
namespace TizenPlatform
{
/**
 * @copydoc Dali::Integration::Log:LogMessage
 */
void LogMessage(Dali::Integration::Log::DebugPriority level, std::string& message);

/**
 * Formats a printf-style message and passes it to LogMessage(). Unlike
 * Dali::Integration::Log::LogMessage(), this reaches the platform logging backend even
 * before the adaptor installs the DALi log function for the current thread.
 */
inline void LogMessageFormat(Dali::Integration::Log::DebugPriority level, const char* format, ...)
{
  if(DALI_UNLIKELY(Dali::Integration::Log::IsLogDisabled()))
  {
    return;
  }

  va_list args;
  va_start(args, format);
  va_list argsCopy;
  va_copy(argsCopy, args);
  const int length = vsnprintf(nullptr, 0, format, args);
  va_end(args);

  if(length >= 0)
  {
    std::string message(static_cast<std::size_t>(length), '\0');
    vsnprintf(&message[0], static_cast<std::size_t>(length) + 1, format, argsCopy);
    LogMessage(level, message);
  }
  va_end(argsCopy);
}

} // namespace TizenPlatform

} //namespace DALI_NAMESPACE

#endif // DALI_TIZEN_LOGGING_H
