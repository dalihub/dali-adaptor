#ifndef DALI_APPLICATION_DEVEL_H
#define DALI_APPLICATION_DEVEL_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#ifdef DALI_ADAPTOR_COMPILATION  // full path doesn't exist until adaptor is installed so we have to use relative
#include <application.h>
#else
#include <dali/public-api/adaptor-framework/application.h>
#endif

namespace Dali
{

namespace DevelApplication
{

/**
 * @brief This is the constructor for applications.
 *
 * @PRIVLEVEL_PUBLIC
 * @PRIVILEGE_DISPLAY
 * @param[in,out]  argc         A pointer to the number of arguments
 * @param[in,out]  argv         A pointer to the argument list
 * @param[in]      stylesheet   The path to user defined theme file
 * @param[in]      windowMode   A member of WINDOW_MODE
 * @param[in]      positionSize A position and a size of the window
 * @return A handle to the Application
 * @note If the stylesheet is not specified, then the library's default stylesheet will not be overridden.
 */
DALI_IMPORT_API Application New( int* argc, char **argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize );

} // namespace DevelApplication

} // namespace Dali

#endif // DALI_APPLICATION_DEVEL_H
