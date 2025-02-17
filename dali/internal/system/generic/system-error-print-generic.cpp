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

// FUNCTION HEADER
#include <dali/internal/system/common/system-error-print.h>

// EXTERNAL INCLUDES
#include <cerrno>  ///< for errno
#include <cstring> ///< for strerror_r
#include <sstream> ///< for std::ostringstream
#include <string>

// INTERNAL INCLUDES
#include <dali/internal/system/common/logging.h>

namespace Dali
{
namespace TizenPlatform
{
namespace
{
// Reference : https://github.com/intel/intel-graphics-compiler/issues/213
// Seperate function logic for GNU-specific and XSI-compliant by the type of return value.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
// For GNU case
std::string ConvertResultToString(const char* errorName, const char* /* not used */)
{
  return std::string(DALI_LIKELY(errorName) ? errorName : "null");
}

// For XSI case
std::string ConvertResultToString(int /* not used */, const char* errorName)
{
  return std::string(DALI_LIKELY(errorName) ? errorName : "null");
}
#pragma GCC diagnostic pop
} // namespace

void PrintSystemError(const char* fileName, const char* functionName, const int lineNumber)
{
  std::ostringstream oss;

  const static int         errorMessageMaxLength               = 128;
  thread_local static char errorMessage[errorMessageMaxLength] = {}; // Initialze as null.

  int copiedErrorNumber = errno;

  auto reternValue = strerror_r(copiedErrorNumber, errorMessage, errorMessageMaxLength - 1);

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
  oss << ConvertResultToString(reternValue, static_cast<const char*>(&errorMessage[0])) << "\n";

  std::string message = oss.str();
  LogMessage(Dali::Integration::Log::DebugPriority::ERROR, message);
}

} // namespace TizenPlatform

} // namespace Dali
