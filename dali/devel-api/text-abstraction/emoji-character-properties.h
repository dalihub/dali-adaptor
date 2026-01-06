#ifndef DALI_TOOLKIT_TEXT_ABSTRACTION_EMOJI_CHARACTER_PROPERTIES_H
#define DALI_TOOLKIT_TEXT_ABSTRACTION_EMOJI_CHARACTER_PROPERTIES_H

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

// EXTERNAL INCLUDES
#include <set>
// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/defined-characters.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace TextAbstraction
{
/**
 * @brief Whether the character is emoji-component | emoji-modifier | COMBINING-ENCLOSING-KEYCAP | regional-indicator | TextPresentationSelector | EmojiPresentationSelector | TagSpec | TagEnd.
 *
 * @param[in] character The character
 *
 * @return @e true if the character  is emoji-component | emoji-modifier | COMBINING-ENCLOSING-KEYCAP | regional-indicator | TextPresentationSelector | EmojiPresentationSelector | TagSpec | TagEnd.
 */
DALI_ADAPTOR_API bool IsEmojiItem(Character character);

/**
 * @brief Whether the character is emoji component.
 * Emoji Properties and Data Files: https://www.unicode.org/reports/tr51/#Emoji_Properties_and_Data_Files
 *
 * @note True for characters used in emoji sequences that normally do not appear on emoji keyboards as separate choices, such as keycap base characters or Regional_Indicator characters.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is emoji component.
 */
DALI_ADAPTOR_API bool IsEmojiComponent(Character character);

/**
 * @brief Whether the character is emoji modifier.
 * Emoji Properties and Data Files: https://www.unicode.org/reports/tr51/#Emoji_Properties_and_Data_Files
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is emoji modifier.
 */
DALI_ADAPTOR_API bool IsEmojiModifier(Character character);

/**
 * @brief Whether the character is COMBINING ENCLOSING KEYCAP.
 * Combining Enclosing Keycap: https://emojipedia.org/combining-enclosing-keycap/
 *
 * @note The Codepoint of COMBINING ENCLOSING KEYCAP is {U+20E3}
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is COMBINING ENCLOSING KEYCAP.
 */
DALI_ADAPTOR_API bool IsCombiningEnclosingKeycap(Character character);

/**
 * @brief Whether the character is Regional Indicator.
 * Regional Indicator Symbol Letter A: https://emojipedia.org/regional-indicator-symbol-letter-a/
 * Regional Indicator Symbol Letter Z: https://emojipedia.org/regional-indicator-symbol-letter-z/
 *
 * @note The Codepoints of Regional Indicator Symbol Letters [A-Z] are [U+1F1E6 - U+1F1FF]
 *
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is Regional Indicator.
 */
DALI_ADAPTOR_API bool IsRegionalIndicator(Character character);

/**
 * @brief Whether the character is text presentation selector (VARIATION SELECTOR-15).
 * Text Presentation Selector: https://www.unicode.org/reports/tr51/#def_text_presentation_selector
 *
 * @note The character U+FE0E VARIATION SELECTOR-15 (VS15), used to request a text presentation for an emoji character. Also known as text variation selector.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is text presentation selector (VARIATION SELECTOR-15).
 */
DALI_ADAPTOR_API bool IsTextPresentationSelector(Character character);

/**
 * @brief Whether the character is emoji variation selector (VARIATION SELECTOR-16).
 * Emoji Presentation Selector: https://www.unicode.org/reports/tr51/#def_emoji_presentation_selector
 *
 * @note The character U+FE0F VARIATION SELECTOR-16 (VS16), used to request an emoji presentation for an emoji character. Also known as emoji variation selector.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is emoji presentation selector (VARIATION SELECTOR-16).
 */
DALI_ADAPTOR_API bool IsEmojiPresentationSelector(Character character);

/**
 * @brief Whether the character is tag spec.
 * Emoji Tag Sequence (ETS): https://www.unicode.org/reports/tr51/#def_emoji_tag_sequence
 *
 * @note The tag_spec consists of all characters from U+E0020 TAG SPACE to U+E007E TAG TILDE.
 * Each tag_spec defines a particular visual variant to be applied to the tag_base character(s).
 * Though tag_spec includes the values U+E0041 TAG LATIN CAPITAL LETTER A .. U+E005A TAG LATIN CAPITAL LETTER Z,
 * they are not used currently and are reserved for future extensions.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is tag spec.
 */
DALI_ADAPTOR_API bool IsTagSpec(Character character);

/**
 * @brief Whether the character is tag end.
 * Emoji Tag Sequence (ETS): https://www.unicode.org/reports/tr51/#def_emoji_tag_sequence
 *
 * @note The tag_end consists of the character U+E007F CANCEL TAG, and must be used to terminate the tag sequence.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is tag end.
 */
DALI_ADAPTOR_API bool IsTagEnd(Character character);

/**
 * @brief Whether the character is Miscellaneous Symbols and Arrows emoji.
 * Miscellaneous Symbols and Arrows emoji: https://unicode-table.com/en/blocks/miscellaneous-symbols-and-arrows/
 *
 * @note The Range is 2B00—2BFF and the quantity of characters is 256.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is Miscellaneous Symbols and Arrows emoji.
 */
DALI_ADAPTOR_API bool IsMiscellaneousSymbolsAndArrowsEmoji(Character character);

/**
 * @brief Whether the character is Dingbats emoji.
 * Dingbats emoji: https://unicode-table.com/en/blocks/dingbats/
 *
 * @note The Range is 2700—27BF and the quantity of characters is 192.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is Dingbats emoji.
 */
DALI_ADAPTOR_API bool IsDingbatsEmoji(Character character);

/**
 * @brief Whether the character is ASCII digits.
 * ASCII digits: https://unicode-table.com/en/blocks/basic-latin/
 *
 * @note The Range is 0030—0039 and the quantity of characters is 10.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is ASCII digits.
 */
DALI_ADAPTOR_API bool IsASCIIDigits(Character character);

/**
 * @brief Whether the character is ASCII PS.
 * ASCII PS: https://unicode-table.com/en/blocks/basic-latin/
 *
 * @note The Range is 0020-007E
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is ASCII PS.
 */
DALI_ADAPTOR_API bool IsASCIIPS(Character character);

/**
 * @brief Whether the character is emoji-variation-sequences.
 * Emoji Variation Sequences for UTS #51: https://www.unicode.org/Public/14.0.0/ucd/emoji/emoji-variation-sequences.txt
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is emoji-variation-sequences.
 */
DALI_ADAPTOR_API bool IsEmojiVariationSequences(Character character);

/**
 * @brief Whether the character is Negative Squared Latin Capital Letter.
 * Read more at White on black squared Latin letters: https://unicode-table.com/en/blocks/enclosed-alphanumeric-supplement/
 *
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is Negative Squared Latin Capital Letter.
 */
DALI_ADAPTOR_API bool IsNegativeSquaredLatinCapitalLetter(Character character);

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_TOOLKIT_TEXT_ABSTRACTION_EMOJI_CHARACTER_PROPERTIES_H