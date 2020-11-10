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
 *
 */

// HEADER
#include <dali/public-api/dali-adaptor-version.h>

// EXTERNAL INCLUDES
#ifdef DEBUG_ENABLED
#include <iostream>
#endif

namespace Dali
{
const unsigned int ADAPTOR_MAJOR_VERSION = 1;
const unsigned int ADAPTOR_MINOR_VERSION = 9;
const unsigned int ADAPTOR_MICRO_VERSION = 36;
const char* const  ADAPTOR_BUILD_DATE    = __DATE__ " " __TIME__;

#ifdef DEBUG_ENABLED
namespace
{
/// Allows the printing of the version number ONLY when debug is enabled
struct PrintVersion
{
  PrintVersion()
  {
    std::cerr << "DALi Adaptor:   " << ADAPTOR_MAJOR_VERSION << "." << ADAPTOR_MINOR_VERSION << "." << ADAPTOR_MICRO_VERSION << " (" << ADAPTOR_BUILD_DATE << ")" << std::endl;
  }
};
PrintVersion ADAPTOR_VERSION;
} // unnamed namespace
#endif

} // namespace Dali
