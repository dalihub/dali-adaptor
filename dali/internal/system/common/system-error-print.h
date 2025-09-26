#ifndef DALI_TIZEN_SYSTEM_ERROR_PRINT_H
#define DALI_TIZEN_SYSTEM_ERROR_PRINT_H

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

#include <dali/integration-api/debug.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace TizenPlatform
{
/**
 * @brief Print errno and the error message to Dali::TizenPlatform::LogMessage as error level.
 * Please use this function with DALI_PRINT_SYSTEM_ERROR_LOG macro
 */
DALI_ADAPTOR_API void PrintSystemError(const char* fileName, const char* functionName, const int lineNumber);

} // namespace TizenPlatform

#ifndef DALI_PRINT_SYSTEM_ERROR_LOG
#define DALI_PRINT_SYSTEM_ERROR_LOG() Dali::TizenPlatform::PrintSystemError(DALI_LOG_FORMAT_PREFIX_ARGS);
#endif

} // namespace Dali

#endif // DALI_TIZEN_SYSTEM_ERROR_PRINT_H
