/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
 */

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/environment-variable.h>

// EXTERNAL INCLUDE
#include <cstdlib>
#include <string>
#include <unordered_map>

#if defined(DALI_PROFILE_WINDOWS)
#ifdef GetEnvironmentVariable
#undef GetEnvironmentVariable
#endif
#endif

namespace Dali
{
namespace EnvironmentVariable
{
const char* GetEnvironmentVariable(const char* variable)
{
#if defined(DALI_PROFILE_WINDOWS)
  char*       buffer = nullptr;
  std::size_t length = 0u;
  if(_dupenv_s(&buffer, &length, variable) != 0 || buffer == nullptr)
  {
    return nullptr;
  }

  thread_local std::unordered_map<std::string, std::string> values;
  std::string&                                               value = values[variable];
  value.assign(buffer);
  std::free(buffer);
  return value.c_str();
#else
  return std::getenv(variable);
#endif
}

bool SetEnvironmentVariable(const char* variable, const char* value)
{
#if defined(DALI_PROFILE_WINDOWS)
  return _putenv_s(variable, value) == 0;
#else
  return setenv(variable, value, 1) == 0;
#endif
}

} // namespace EnvironmentVariable

} // namespace Dali
