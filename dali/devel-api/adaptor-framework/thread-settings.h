#ifndef DALI_THREAD_SETTINGS_H
#define DALI_THREAD_SETTINGS_H

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

// EXTERNAL INCLUDES
#include <string>

namespace Dali
{
/**
 * @brief Set the thread name.
 *
 * @param [in] threadName The name of thread. The name can be up to 16 bytes long, and should be null-terminated if it contains fewer bytes.
 */
DALI_ADAPTOR_API void SetThreadName(const std::string& threadName);

} // Dali

#endif // DALI_THREAD_SETTINGS_H