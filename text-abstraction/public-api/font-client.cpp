/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include "font-client.h"

// INTERNAL INCLUDES
#include <internal/font-client-impl.h>

namespace Dali
{

namespace TextAbstraction
{

FontClient::FontClient()
{
}
FontClient::~FontClient()
{
}
FontClient::FontClient(Dali::Internal::TextAbstraction::FontClient *impl)
  : BaseHandle(impl)
{
}

FontClient FontClient::Get()
{
  return Dali::Internal::TextAbstraction::FontClient::Get();
}


} // namespace TextAbstraction

} // namespace Dali
