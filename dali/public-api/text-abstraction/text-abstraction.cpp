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

// CLASS HEADER
#include <dali/public-api/text-abstraction/text-abstraction.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/integration-api/string-utils.h>

namespace Dali
{
namespace TextAbstraction
{

bool AddCustomFontDirectory(const Dali::String& path)
{
  FontClient fontClient = FontClient::Get();
  if(!fontClient)
  {
    return false;
  }

  return fontClient.AddCustomFontDirectory(Dali::Integration::ToStdString(path));
}

} // namespace TextAbstraction

} // namespace Dali
