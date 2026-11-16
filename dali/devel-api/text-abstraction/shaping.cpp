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
#include <dali/devel-api/text-abstraction/shaping.h>

// EXTERNAL INCLUDES
#include <array>

// INTERNAL INCLUDES
#include <dali/internal/text/text-abstraction/shaping-impl.h>
#include <dali/internal/text/text-abstraction/text-language.h>

namespace DALI_NAMESPACE
{
namespace TextAbstraction
{
bool NormalizeLanguageTag(const std::string& languageTag, std::string& normalizedLanguageTag)
{
  std::array<char, 256u> normalized;
  if(!Internal::NormalizeLanguageTag(languageTag, normalized))
  {
    return false;
  }
  normalizedLanguageTag.assign(normalized.data(), languageTag.size());
  return true;
}

Shaping::Shaping()
{
}

Shaping::~Shaping()
{
}

Shaping::Shaping(Internal::Shaping* impl)
: BaseHandle(impl)
{
}

Shaping Shaping::Get()
{
  return Internal::Shaping::Get();
}

Shaping Shaping::New()
{
  auto shapingImpl = Internal::Shaping::New();

  return Shaping(shapingImpl.Get());
}

Length Shaping::Shape(TextAbstraction::FontClient& fontClient,
                      const Character* const       text,
                      Length                       numberOfCharacters,
                      FontId                       fontId,
                      Script                       script)
{
  return GetImplementation(*this).Shape(fontClient,
                                        text,
                                        numberOfCharacters,
                                        fontId,
                                        script);
}

Length Shaping::ShapeWithLanguage(TextAbstraction::FontClient& fontClient,
                                  const Character*             text,
                                  Length                       numberOfCharacters,
                                  FontId                       fontId,
                                  Script                       script,
                                  const std::string&           language)
{
  return GetImplementation(*this).ShapeWithLanguage(fontClient, text, numberOfCharacters, fontId, script, language);
}

void Shaping::GetGlyphs(GlyphInfo*      glyphInfo,
                        CharacterIndex* glyphToCharacterMap)
{
  GetImplementation(*this).GetGlyphs(glyphInfo,
                                     glyphToCharacterMap);
}

} // namespace TextAbstraction

} //namespace DALI_NAMESPACE
