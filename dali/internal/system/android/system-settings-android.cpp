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

// CLASS HEADER
#include <dali/internal/system/common/system-settings.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace SystemSettings
{
std::string GetResourcePath()
{
  return DALI_DATA_RO_DIR;
}

std::string GetDataPath()
{
  return "";
}

} // namespace SystemSettings

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
