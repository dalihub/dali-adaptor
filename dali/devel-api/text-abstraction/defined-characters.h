#ifndef DALI_TOOLKIT_TEXT_ABSTRACTION_DEFINED_CHARACTERS_H
#define DALI_TOOLKIT_TEXT_ABSTRACTION_DEFINED_CHARACTERS_H

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

namespace Dali
{
namespace TextAbstraction
{
constexpr unsigned int CHAR_COMBINING_ENCLOSING_KEYCAP = 0x20E3; ///< Combining Enclosing Keycap. https://unicode-table.com/en/1F1E6/

//Emoji Components (Hair style)
constexpr unsigned int CHAR_EMOJI_COMPONENT_BALD       = 0x1F9B2; ///< Emoji CompOnent Bald. https://unicode-table.com/en/1F9B2/
constexpr unsigned int CHAR_EMOJI_COMPONENT_RED_HAIR   = 0x1F9B0; ///< Emoji CompOnent Red Hair. https://unicode-table.com/en/1F9B0/
constexpr unsigned int CHAR_EMOJI_COMPONENT_CURLY_HAIR = 0x1F9B1; ///< Emoji CompOnent Curly Hair. https://unicode-table.com/en/1F9B1/
constexpr unsigned int CHAR_EMOJI_COMPONENT_WHITE_HAIR = 0x1F9B3; ///< Emoji CompOnent White Hair. https://unicode-table.com/en/1F9B3/

//Regional Indicators
constexpr unsigned int CHAR_REGIONAL_INDICATOR_SYMBOL_LETTER_A = 0x1F1E6; ///< Regional Indicator Symbol Letter A. https://unicode-table.com/en/1F1E6/
constexpr unsigned int CHAR_REGIONAL_INDICATOR_SYMBOL_LETTER_Z = 0x1F1FF; ///< Regional Indicator Symbol Letter Z. https://unicode-table.com/en/1F1FF/

//Variation Selectors
constexpr unsigned int CHAR_VARIATION_SELECTOR_15 = 0xFE0E; ///< Variation Selector-15. https://unicode-table.com/en/FE0E/
constexpr unsigned int CHAR_VARIATION_SELECTOR_16 = 0xFE0F; ///< Variation Selector-16. https://unicode-table.com/en/FE0F/

//Emoji Tags
constexpr unsigned int CHAR_CANCEL_TAG = 0xE007F; ///< Stateful tag terminator: Cancel Tag. https://unicode-table.com/en/E007F/
constexpr unsigned int CHAR_TAG_SPACE  = 0xE0020; ///< Tag components: Tag Space. The first tag in tag_spec https://unicode-table.com/en/E0020/
constexpr unsigned int CHAR_TAG_TILDE  = 0xE007E; ///< Tag components: Tag Tilde. The last tag in tag_spec https://unicode-table.com/en/E007E/

//Emoji Modifiers (Skin tone)
constexpr unsigned int CHAR_EMOJI_MODIFIER_FITZPATRICK_TYPE_1_2 = 0x1F3FB; ///< Emoji Modifier Fitzpatrick Type-1-2 (Light Skin Tone). https://unicode-table.com/en/1F3FB/
constexpr unsigned int CHAR_EMOJI_MODIFIER_FITZPATRICK_TYPE_3   = 0x1F3FC; ///< Emoji Modifier Fitzpatrick Type-3 (Medium-Light Skin Tone). https://unicode-table.com/en/1F3FC/
constexpr unsigned int CHAR_EMOJI_MODIFIER_FITZPATRICK_TYPE_4   = 0x1F3FD; ///< Emoji Modifier Fitzpatrick Type-4 (Medium Skin Tone). https://unicode-table.com/en/1F3FD/
constexpr unsigned int CHAR_EMOJI_MODIFIER_FITZPATRICK_TYPE_5   = 0x1F3FE; ///< Emoji Modifier Fitzpatrick Type-5 (Medium-Dark Skin Tone). https://unicode-table.com/en/1F3FE/
constexpr unsigned int CHAR_EMOJI_MODIFIER_FITZPATRICK_TYPE_6   = 0x1F3FF; ///< Emoji Modifier Fitzpatrick Type-6 (Dark Skin Tone). https://unicode-table.com/en/1F3FF/

//Miscellaneous Symbols and Arrows. Range: 2B00—2BFF
constexpr unsigned int CHAR_NORTH_EAST_WHITE_ARROW     = 0x2B00; ///< North East White Arrow. https://unicode-table.com/en/2B00/
constexpr unsigned int CHAR_HELLSCHREIBER_PAUSE_SYMBOL = 0x2BFF; ///< Hellschreiber Pause Symbol . https://unicode-table.com/en/2BFF/

//Dingbats. Range: 2700—27BF
constexpr unsigned int CHAR_BLACK_SAFETY_SCISSORS   = 0x2700; ///< Black Safety Scissors. https://unicode-table.com/en/2700/
constexpr unsigned int CHAR_DOUBLE_CURLY_LOOP_EMOJI = 0x27BF; ///< Double Curly Loop Emoji . https://unicode-table.com/en/27BF/

//Basic Latin. Range: 0000—007F
constexpr unsigned int CHAR_NUMBER_SIGN               = 0x0023; ///< # Number Sign. https://unicode-table.com/en/0023/
constexpr unsigned int CHAR_ASTERISK                  = 0x002A; ///< * Asterisk. https://unicode-table.com/en/002A/
constexpr unsigned int CHAR_ASCII_DIGIT_0             = 0x0030; ///< 0 Digit Zero. https://unicode-table.com/en/0030/
constexpr unsigned int CHAR_ASCII_DIGIT_9             = 0x0039; ///< 9 Digit Nine. https://unicode-table.com/en/0039/
constexpr unsigned int CHAR_ASCII_SPACE               = 0x0020; ///< Space. https://unicode-table.com/en/0020/
constexpr unsigned int CHAR_ASCII_SOLIDUS             = 0x002F; ///< / Solidus. https://unicode-table.com/en/002F/
constexpr unsigned int CHAR_ASCII_COLON               = 0x003A; ///< : Colon. https://unicode-table.com/en/003A/
constexpr unsigned int CHAR_ASCII_COMMERCIAL_AT       = 0x0040; ///< @ Commercial At. https://unicode-table.com/en/0040/
constexpr unsigned int CHAR_ASCII_LEFT_SQUARE_BRACKET = 0x005B; ///< [ Left Square Bracket. https://unicode-table.com/en/005B/
constexpr unsigned int CHAR_ASCII_GRAVE_ACCENT        = 0x0060; ///< ` Grave Accent. https://unicode-table.com/en/0060/
constexpr unsigned int CHAR_ASCII_LEFT_CURLY_BRACKET  = 0x007B; ///< { Left Curly Bracket. https://unicode-table.com/en/007B/
constexpr unsigned int CHAR_ASCII_TILDE               = 0x007E; ///< ~ Tilde. https://unicode-table.com/en/007E/

// General unicodes
constexpr unsigned int CHAR_ZWS = 0x200B; ///< Zero width space.

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_TOOLKIT_TEXT_ABSTRACTION_DEFINED_CHARACTERS_H