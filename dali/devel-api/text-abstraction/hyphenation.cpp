/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/text-abstraction/hyphenation.h>

// INTERNAL INCLUDES
#include <dali/internal/text/text-abstraction/hyphenation-impl.h>

namespace Dali
{
namespace TextAbstraction
{
Hyphenation::Hyphenation()
{
}

Hyphenation::~Hyphenation()
{
}

Hyphenation::Hyphenation(Internal::Hyphenation* impl)
: BaseHandle(impl)
{
}

Hyphenation Hyphenation::Get()
{
  return Internal::Hyphenation::Get();
}

Hyphenation Hyphenation::New()
{
  auto hyphenationImpl = new Internal::Hyphenation();

  return Hyphenation(hyphenationImpl);
}

const char* Hyphenation::GetDictionaryEncoding(const char* lang)
{
  return GetImplementation(*this).GetDictionaryEncoding(lang);
}

Vector<bool> Hyphenation::GetWordHyphens(const char* word,
                                         Length      wordSize,
                                         const char* lang)
{
  return GetImplementation(*this).GetWordHyphens(word,
                                                 wordSize,
                                                 lang);
}

} // namespace TextAbstraction

} // namespace Dali
