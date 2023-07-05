#ifndef DALI_INTERNAL_SYSTEM_SETTINGS_H
#define DALI_INTERNAL_SYSTEM_SETTINGS_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace SystemSettings
{
/**
 *  Gets the path at which application resources are stored.
 */
std::string GetResourcePath();

/**
 *  Gets the path at which application data are stored.
 */
std::string GetDataPath();

} // namespace SystemSettings

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_SYSTEM_SETTINGS_H
