#ifndef DALI_THREAD_SETTINGS_H
#define DALI_THREAD_SETTINGS_H

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

// EXTERNAL INCLUDES
#include <cstdint>
#include <string>

namespace Dali
{
/**
 * @brief Set the thread name.
 *
 * @param [in] threadName The name of thread. The name can be up to 16 bytes long, and should be null-terminated if it contains fewer bytes.
 */
DALI_ADAPTOR_API void SetThreadName(const std::string& threadName);

/**
 * @brief Get the thread id.
 * @note If thread id getter doesn't supported, it will return 0 as default.
 * @return The thread id.
 */
DALI_ADAPTOR_API int32_t GetThreadId();

/**
 * @brief Get the main thread id.
 * @note If thread id getter doesn't supported, it will return 0 as default.
 * @return The main thread id.
 */
DALI_ADAPTOR_API int32_t GetMainThreadId();

/**
 * @brief Get the UI thread id, which DALi::Adaptor runs on.
 * @pre Need to call SetCurrentThreadAsUiThread() in the UI thread before calling this API.
 * @note If thread id getter doesn't supported, it will return 0 as default.
 * @return The UI thread id.
 */
DALI_ADAPTOR_API int32_t GetUiThreadId();

} // namespace Dali

#endif // DALI_THREAD_SETTINGS_H