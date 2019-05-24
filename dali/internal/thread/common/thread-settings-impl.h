#ifndef DALI_INTERNAL_THREAD_SETTINGS_H
#define DALI_INTERNAL_THREAD_SETTINGS_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <string>
#include <sys/prctl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Implementation of the Thread Settings
 */
namespace ThreadSettings
{

/**
 * @brief Set the thread name.
 *
 * @param [in] threadName The name of thread. The name can be up to 16 bytes long, and should be null-terminated if it contains fewer bytes.
 */
void SetThreadName(const std::string& threadName);

} // namespace ThreadSettings

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_THREAD_SETTINGS_H
