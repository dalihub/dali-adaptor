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

// CLASS HEADER
#include <dali/devel-api/text-abstraction/icu.h>

// INTERNAL INCLUDES
#include <dali/internal/text/text-abstraction/icu-impl.h>

namespace Dali
{
namespace TextAbstraction
{
ICU::ICU()
{
}

ICU::~ICU()
{
}

ICU::ICU(Internal::ICU* impl)
: BaseHandle(impl)
{
}

ICU ICU::New()
{
  auto icuImpl = new Internal::ICU();

  return ICU(icuImpl);
}

void ICU::UpdateLineBreakInfoByLocale(const std::string& text,
                                      Length             numberOfCharacters,
                                      const char*        locale,
                                      LineBreakInfo*     breakInfo)
{
  GetImplementation(*this).UpdateLineBreakInfoByLocale(text, numberOfCharacters, locale, breakInfo);
}

} // namespace TextAbstraction

} // namespace Dali
