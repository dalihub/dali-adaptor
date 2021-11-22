#ifndef DALI_TOOLKIT_TEXT_ABSTRACTION_EMOJI_HELPER_H
#define DALI_TOOLKIT_TEXT_ABSTRACTION_EMOJI_HELPER_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/script.h>

namespace Dali
{
namespace TextAbstraction
{
/**
 * @brief Whether the script is symbol.
 *
 * @note Check whether the script is in [SYMBOLS1, SYMBOLS2, SYMBOLS3, SYMBOLS4, SYMBOLS5]
 *
 * @param[in] script The script.
 *
 * @return @e true if the script is symbol.
 */
DALI_ADAPTOR_API bool IsSymbolScript(const TextAbstraction::Script& script);

/**
 * @brief Whether the script is EMOJI.
 *
 * @param[in] script The script.
 *
 * @return @e true if the script is EMOJI.
 */
DALI_ADAPTOR_API bool IsEmojiScript(const TextAbstraction::Script& script);

/**
 * @brief Whether the script is EMOJI_TEXT.
 *
 * @param[in] script The script.
 *
 * @return @e true if the script is EMOJI_TEXT.
 */
DALI_ADAPTOR_API bool IsEmojiTextScript(const TextAbstraction::Script& script);

/**
 * @brief Whether the script is EMOJI_COLOR.
 *
 * @param[in] script The script.
 *
 * @return @e true if the script is EMOJI_COLOR.
 */
DALI_ADAPTOR_API bool IsEmojiColorScript(const TextAbstraction::Script& script);

/**
 * @brief Whether the script is symbol or emoji.
 *
 * @note At least one of IsSymbol and IsEmoji is true.
 *
 * @param[in] script The script.
 *
 * @return @e true if the script is symbol or emoji.
 */
DALI_ADAPTOR_API bool IsSymbolOrEmojiScript(const TextAbstraction::Script& script);

/**
 * @brief Whether the script is SYMBOL | EMOJI | TEXT.
 *
 * @note At least one of IsSymbol and IsEmoji and IsText is true.
 *
 * @param[in] script The script.
 *
 * @return @e true if the script is SYMBOL | EMOJI | EMOJI_COLOR| EMOJI_TEXT.
 */
DALI_ADAPTOR_API bool IsSymbolOrEmojiOrTextScript(const TextAbstraction::Script& script);

/**
 * @brief Whether the script is EMOJI | EMOJI_COLOR| EMOJI_TEXT.
 *
 * @param[in] script The script.
 *
 * @return @e true if the script is EMOJI | EMOJI_COLOR| EMOJI_TEXT.
 */
DALI_ADAPTOR_API bool IsOneOfEmojiScripts(const TextAbstraction::Script& script);

/**
 * @brief Retrieve the unicode for invisible glyphs
 *
 * @note Used CHAR_ZWS it is the unicode for Zero Width Space character
 *
 * @return Retrieve the unicode for Zero Width Space character
 */
DALI_ADAPTOR_API Character GetUnicodeForInvisibleGlyph();

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_TOOLKIT_TEXT_ABSTRACTION_EMOJI_HELPER_H