#ifndef __DALI_ENVIRONMENT_VARIABLE_H__
#define __DALI_ENVIRONMENT_VARIABLE_H__

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace EnvironmentVariable
{

/**
 * @brief Search the environment list for the specified variable name and return a pointer to the C string that is associated with the matched environment list member.
 *
 * @param[in] variable Null-terminated character string identifying the name of the environmental variable to look for.
 * @return A C-string containing the value of the specified environment variable.
 */
DALI_ADAPTOR_API const char * GetEnvironmentVariable( const char * variable );

/**
 * @brief Create or overwrite (when it does not exist) an environment variable.
 *
 * @param[in] variable Null-terminated character string identifying the name of the environmental variable.
 * @param[in] value Null-terminated character string to set as a value.
 * @return True on success, false on error.
 */
DALI_ADAPTOR_API bool SetEnvironmentVariable( const char * variable, const char * value );

} // namespace EnvironmentVariable

} // namespace Dali

#endif /*__DALI_ ENVIRONMENT_VARIABLE_H__ */
