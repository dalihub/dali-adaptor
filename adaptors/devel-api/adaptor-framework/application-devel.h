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

/*
 * @brief An enum of memory status.
 */
struct MemoryStatus
{
  enum Type
  {
    /*
     * Normal status.
     */
    NORMAL,

    /*
     * Soft warning status.
     */
    SOFT_WARNING,

    /*
     * Hard warning status.
     */
    HARD_WARNING
  };
};

/*
 * @brief An enum of battery status.
 */
struct BatteryStatus
{
  enum Type
  {
    /*
     * Normal status.
     * Battery status is over 5%.
     */
    NORMAL,

    /*
     * Battery status is under 5%.
     */
    CRITICAL_LOW,

    /*
     * Device can be turned off anytime.
     * Battery status is under 1%.
     */
    POWER_OFF
  };
};

typedef Signal< void (BatteryStatus::Type) > LowBatterySignalType;
typedef Signal< void (MemoryStatus::Type) > LowMemorySignalType;

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

/**
 * @brief This is used to improve application launch performance.
 * It preloads so files, initializes some functions in advance and makes a window in advance.
 *
 * @param[in,out]  argc         A pointer to the number of arguments
 * @param[in,out]  argv         A pointer to the argument list
 */
DALI_IMPORT_API void PreInitialize( int* argc, char** argv[] );

/**
 * @brief This is used to get region information from device.
 *
 * @return Region information
 */
DALI_IMPORT_API std::string GetRegion( Application application );

/**
 * @brief This is used to get language information from device.
 *
 * @return Language information
 */
DALI_IMPORT_API std::string GetLanguage( Application application );

/**
 * @brief This signal is emitted when the battery level of the device is low.
 * @SINCE_1_0.0
 * @return The signal to connect to
 */
DALI_IMPORT_API LowBatterySignalType& LowBatterySignal( Application application );

/**
 * @brief This signal is emitted when the memory level of the device is low.
 * @SINCE_1_0.0
 * @return The signal to connect to
 */
DALI_IMPORT_API LowMemorySignalType& LowMemorySignal( Application application );


} // namespace DevelApplication

} // namespace Dali

#endif // DALI_APPLICATION_DEVEL_H
