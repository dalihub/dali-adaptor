/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// WINDOWS INCLUDES
#include <Windows.h> // For GetLastError and FormatMessage

// Need to undef the following constants as they are defined in one of the headers in Windows.h but used in DALi (via debug.h)
#undef TRANSPARENT // Used in constants.h
#undef CopyMemory  // Used in dali-vector.h

// FUNCTION HEADER
#include <dali/internal/system/common/system-error-print.h>

// EXTERNAL INCLUDES
#include <sstream> ///< for std::ostringstream
#include <string>

// INTERNAL INCLUDES
#include <dali/internal/system/common/logging.h>

namespace Dali
{
namespace TizenPlatform
{
void PrintSystemError(const char* fileName, const char* functionName, const int lineNumber)
{
  std::ostringstream oss;

  // Windows error messages are very long! (error code 10401 message length is 2174)
  // Use FormatMessage limitation length here : 65535
  const static DWORD       errorMessageMaxLength               = 0xffff;
  thread_local static char errorMessage[errorMessageMaxLength] = {}; // Initialze as null.

  DWORD copiedErrorNumber = GetLastError();

  // FORMAT_MESSAGE_IGNORE_INSERTS Should be used when we try to get FORMAT_MESSAGE_FROM_SYSTEM
  DWORD errorMessageLength = FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, copiedErrorNumber, 0, (LPTSTR) & (&errorMessage[0]), errorMessageMaxLength - 1, NULL);

  if(DALI_LIKELY(fileName))
  {
    oss << fileName << ": ";
  }
  if(DALI_LIKELY(functionName))
  {
    oss << functionName << "";
  }
  oss << "(" << lineNumber << ") > ";

  oss << "errno [" << copiedErrorNumber << "] ";
  oss << std::string(&errorMessage[0], (size_t)errorMessageLength) << "\n";

  std::string message = oss.str();
  LogMessage(Dali::Integration::Log::DebugPriority::ERROR, message);
}

} // namespace TizenPlatform

} // namespace Dali
