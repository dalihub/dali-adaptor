#ifndef DALI_PLATFORM_TEXT_ABSTRACTION_SHAPING_H
#define DALI_PLATFORM_TEXT_ABSTRACTION_SHAPING_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/emoji-helper.h>
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/devel-api/text-abstraction/script.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>
#include <string>

namespace DALI_NAMESPACE
{
namespace TextAbstraction
{
struct GlyphInfo;

/**
 * @brief Validates a BCP 47 language tag and normalizes it to ASCII lowercase.
 *
 * Accepts one ASCII tag of at most 255 bytes, such as "bg" (Bulgarian) or
 * "sr-Cyrl" (Serbian Cyrillic). Language, script and region subtags use hyphens.
 * Tags are case-insensitive. POSIX locale forms such as "bg_BG" and
 * "bg_BG.UTF-8" are not accepted.
 *
 * Validation follows RFC 5646 syntax, including grandfathered and private-use
 * tags. Registry membership is not checked and deprecated aliases are not replaced.
 *
 * An empty string is accepted and remains empty. Shaping APIs interpret an
 * empty language as using the system language.
 *
 * @param[in] languageTag The BCP 47 language tag to normalize, or empty.
 * @param[out] normalizedLanguageTag The lowercase tag, or empty, on success. Unchanged on failure.
 * @return True if the input is accepted, false otherwise.
 * @note Input and output may refer to the same string object.
 */
DALI_ADAPTOR_API bool NormalizeLanguageTag(const std::string& languageTag, std::string& normalizedLanguageTag);

namespace Internal DALI_INTERNAL
{
class Shaping;

} //namespace Internal DALI_INTERNAL

/**
 * @brief Shaping provides an interface to retrieve glyphs from complex text.
 *
 * This module shapes text for a unique font id and script. If the text contains different fonts and scripts
 * it needs to be split in runs of consecutive characters with the same font id and script.
 *
 * @code
 * Shaping shaping = Shaping::Get();
 * FontClient fontClient = FontClient::Get();
 *
 * // Shapes a number of characters with the given font id and script.
 * const Length numberOfGlyphs = shaping.Shape( fontClient, text, numberOfCharacters, fontId, script );
 *
 * // Allocate memory to retrieve the glyphs and the character to glyph conversion map.
 * GlyphInfo* glyphInfo = reinterpret_cast<GlyphInfo*>( malloc( numberOfGlyphs * sizeof( GlyphInfo ) ) );
 * CharacterIndex* glyphToCharacterMap = reinterpret_cast<CharacterIndex*>( malloc( numberOfGlyphs * sizeof( CharacterIndex ) ) );
 *
 * // Retrieve the glyphs and the conversion map.
 * shaping.GetGlyphs( glyphInfo, glyphToCharacterMap );
 * @endcode
 */
class DALI_ADAPTOR_API Shaping : public BaseHandle
{
public:
  /**
   * @brief Create an uninitialized Shaping handle.
   *
   */
  Shaping();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~Shaping();

  /**
   * @brief This constructor is used by Shaping::Get().
   *
   * @param[in] implementation A pointer to the internal shaping object.
   */
  explicit DALI_INTERNAL Shaping(Internal::Shaping* implementation);

  /**
   * @brief Retrieve a handle to the Shaping instance.
   *
   * @return A handle to the Shaping.
   * @remarks A reference to the singleton instance of Shaping.
   */
  static Shaping Get();

  /**
   * @brief Create a handle to the new Shaping instance.
   *
   * @return A handle to the Shaping.
   * @remarks All functions of this are not thread-safe,
   * so create new handles for each worker thread to utilize them.
   */
  static Shaping New();

  /**
   * @brief Shapes the text using the system shaping language.
   *
   * Call GetGlyphs() to retrieve the glyphs.
   *
   * @param[in] fontClient FontClient to use in this function.
   * @param[in] text Pointer to the first character of the text coded in UTF32.
   * @param[in] numberOfCharacters The number of characters to be shaped.
   * @param[in] fontId The font to be used to shape the text.
   * @param[in] script The text's script.
   *
   * @return The size of the buffer required to get the shaped text.
   */
  Length Shape(TextAbstraction::FontClient& fontClient,
               const Character* const       text,
               Length                       numberOfCharacters,
               FontId                       fontId,
               Script                       script);

  /**
   * @brief Shapes text using language-specific glyph forms from the given font.
   *
   * The per-call language selects localized forms and positioning supported by
   * the font. For example, Russian ("ru") and Bulgarian ("bg") may use different
   * glyphs for the same Unicode characters. Call GetGlyphs() to retrieve the result.
   *
   * An empty language ("") uses the same system-language path as Shape().
   * This does not change the global locale, font selection, font fallback,
   * the supplied script or layout direction.
   *
   * The language is not validated by this function. Validate and normalize it
   * once before shaping, for example:
   * @code
   * std::string normalized;
   * if(TextAbstraction::NormalizeLanguageTag("BG", normalized))
   * {
   *   shaping.ShapeWithLanguage(fontClient, text, numberOfCharacters, fontId, script, normalized);
   * }
   * @endcode
   *
   * @param[in] fontClient The font client to use for shaping.
   * @param[in] text Pointer to the first UTF-32 character to shape.
   * @param[in] numberOfCharacters The number of characters to shape.
   * @param[in] fontId The font to use for shaping.
   * @param[in] script The script of the text run.
   * @param[in] language A validated, normalized BCP 47 language tag, or "" to use the system language.
   * @return The number of shaped glyphs.
   * @pre The language must be empty or a BCP 47 tag validated and normalized using Dali::TextAbstraction::NormalizeLanguageTag().
   * @note Shape() retains its original binary interface and member-function type.
   */
  Length ShapeWithLanguage(TextAbstraction::FontClient& fontClient,
                           const Character*             text,
                           Length                       numberOfCharacters,
                           FontId                       fontId,
                           Script                       script,
                           const std::string&           language);

  /**
   * @brief Gets the shaped text data.
   *
   * @pre @p glyphInfo and @p glyphToCharacterMap must have enough space allocated for the number of glyphs.
   * Call Shape() or ShapeWithLanguage() first to shape the text and get the number of glyphs.
   *
   * @param[out] glyphInfo Vector with indices to the glyph within the font, glyph's metrics and advance.
   * @param[out] glyphToCharacterMap The glyph to character conversion map.
   */
  void GetGlyphs(GlyphInfo*      glyphInfo,
                 CharacterIndex* glyphToCharacterMap);
};

} // namespace TextAbstraction

} //namespace DALI_NAMESPACE

#endif // DALI_PLATFORM_TEXT_ABSTRACTION_SHAPING_H
