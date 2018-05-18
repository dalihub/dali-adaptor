#ifndef DALI_APPLICATION_DEVEL_H
#define DALI_APPLICATION_DEVEL_H

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
 *
 */

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/application.h>

namespace Dali
{

namespace DevelApplication
{
/**
 * @brief This is used to improve application launch performance.
 * It preloads so files, initializes some functions in advance and makes a window in advance.
 *
 * @param[in,out]  argc         A pointer to the number of arguments
 * @param[in,out]  argv         A pointer to the argument list
 */
DALI_ADAPTOR_API void PreInitialize( int* argc, char** argv[] );

} // namespace DevelApplication

} // namespace Dali

#endif // DALI_APPLICATION_DEVEL_H
