/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#pragma once

#include <dali/public-api/common/dali-namespace.h>
#include <string>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief The current DALi locale, independent of the system settings and Adaptor lifetime.
 *
 * Access and updates belong to the DALi event thread. Before Adaptor startup,
 * callers must serialize access. Workers use copies supplied by their callers.
 */
namespace Locale
{

/**
 * @brief Initializes empty values from LC_MESSAGES without changing the process locale.
 */
void EnsureLocale();

/**
 * @brief Returns the cached language component using the existing TextAbstraction parsing rules.
 *
 * @return A reference to the cached language component
 */
const std::string& GetLanguageCode();

/**
 * @brief Returns the cached locale name with the suffix starting at the first dot removed.
 *
 * @return A reference to the cached locale name
 */
const std::string& GetLocaleName();

/**
 * @brief Stores the locale using the existing parsing rules.
 *
 * Does not emit signals or update font caches.
 * Getter references are not snapshots; copy values needed
 * across subsequent updates. This function does not change the process locale.
 *
 * @param[in] locale The locale to store
 */
void SetLocale(const std::string& locale);

} // namespace Locale
} // namespace Adaptor
} // namespace Internal
} // namespace DALI_NAMESPACE
