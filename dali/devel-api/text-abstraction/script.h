#ifndef DALI_TOOLKIT_TEXT_ABSTRACTION_SCRIPT_H
#define DALI_TOOLKIT_TEXT_ABSTRACTION_SCRIPT_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/devel-api/text-abstraction/text-abstraction-definitions.h>

namespace Dali
{

namespace TextAbstraction
{

/**
 * @brief Script is the writing system used by a language.
 * Typically one script can be used to write different languages although one language could be written in different scrips.
 */
enum Script
{
  COMMON,        ///< Valid for all scripts. i.e white space or '\n'.

  ASCII_DIGITS,  ///< ASCII digits.
  ASCII_PS,      ///< ASCII punctuation and symbols.

  C1_CONTROLS,   ///< Controls of the C1 Controls and Latin-1 Supplement unicode block.
  C1_PS,         ///< Punctuation and symbols of the C1 Controls and Latin-1 Supplement unicode block.
  C1_MATH,       ///< Math symbols of the C1 Controls and Latin-1 Supplement unicode block.

  SML_P,         ///< Punctuation symbols of the Spacing Modifier Letters unicode block.
  PHONETIC_U,    ///< Uralic phonetic symbols of the Phonetic Extensions unicode block.
  PHONETIC_SS,   ///< Subscripts and superscripts of the Phonetic Extensions unicode block.

  NUMERIC_SS,    ///< Numeric subscripts and superscripts.

  LETTER_LIKE,   ///< Symbols of the Letterlike unicode block.
  NUMBER_FORMS,  ///< Number Forms unicode block.
  FRACTIONS_NF,  ///< Numeric fraction symbols of the Number Forms unicode block.
  NON_LATIN_LED, ///< Non latin symbols within the Latin Extended D unicode block.
  HWFW_S,        ///< Non latin symbols within the Halfwidth and fullwidth unicode block.

  CYRILLIC,      ///< The Cyrillic script. Used by Russian, Bulgarian, Ukrainian, Macedonian, ...
  GREEK,         ///< The Greek script. Used by Greek.
  LATIN,         ///< The latin script. Used by many western languages and others around the world.

  ARABIC,        ///< The arabic script. Used by Arab and Urdu among others.
  HEBREW,        ///< The Hebrew script. Used by the Hebrew, Yiddish, Ladino, and Judeo-Arabic.

  ARMENIAN,      ///< The Armenian script. Used by Armenian.
  GEORGIAN,      ///< The Georgian script. Used by Georgian.

  CJK,           ///< The CJK script. Used by Chinese, Japanese, Korean and Vietnamese(old writing system).
  HANGUL,        ///< The Hangul jamo script. Used by Korean.
  HIRAGANA,      ///< The Hiragana script. Used by the Japanese.
  KATAKANA,      ///< The Katakana script. Used by the Japanese.
  BOPOMOFO,      ///< The Bopomofo script. Also called Zhuyin fuhao or Zhuyin. A phonetic notation used for the transcription of spoken Chinese.

  BENGALI,       ///< The Bengali script. Used by Bangla, Assamese, Bishnupriya Manipuri, Daphla, Garo, Hallam, Khasi, Mizo, Munda, Naga, Rian, and Santali.
  BURMESE,       ///< The Burmese script. Used by the Burmese (Myanmar) language.
  DEVANAGARI,    ///< The devanagari script. Used by Hindi, Marathi, Sindhi, Nepali and Sanskrit.
  GUJARATI,      ///< The Gujarati script. Used by Gujarati.
  GURMUKHI,      ///< The Gurmukhi script. Used by Punjabi.
  KANNADA,       ///< The Kannada script. Used by Kannada and Tulu.
  MALAYALAM,     ///< The Malayalam script. Used by Malayalam.
  ORIYA,         ///< The Oriya script. Used by Oriya (Odia), Khondi, and Santali.
  SINHALA,       ///< The Sinhala script. Used by Sinhala and Pali.
  TAMIL,         ///< The Tamil script. Used by Tamil, Badaga, and Saurashtra.
  TELUGU,        ///< The Telugu script. Used by Telugu, Gondi, and Lambadi.

  LAO,           ///< The Lao script. Used by the Lao language.
  THAI,          ///< The Thai script. Used by the Thai language
  KHMER,         ///< The Khmer script. Used by the Khmer language.
  JAVANESE,      ///< The Javanese script. Used by the Javanese language.
  SUNDANESE,     ///< The Sundanese script. Used by the Sundanese language.

  GEEZ,          ///< The Ge'ez script. Used by the Amharic, Tigrinya and other languages in Ethiopia and Eritrea.
  OL_CHIKI,      ///< The Ol Chiki script. Used by the Santali.
  BAYBAYIN,      ///< The Baybayin script. Used by the Tagalog, Bikol languages, Ilocano, Pangasinan, Visayan and other languages in Philippines.
  MEITEI,        ///< The Meitei script used for the Meitei language in Manipur, India.

  EMOJI,         ///< The Emoji which map to standardized Unicode characters.

  SYMBOLS1,      ///< Some symbols.
  SYMBOLS2,      ///< Some symbols.
  SYMBOLS3,      ///< Some symbols.
  SYMBOLS4,      ///< Some symbols.
  SYMBOLS5,      ///< Some symbols.

  UNKNOWN        ///< The script is unknown.
};

const char* const ScriptName[] =
{
  "COMMON",        ///< Valid for all scripts. i.e white space or '\n'.

  "ASCII_DIGITS",  ///< ASCII digits.
  "ASCII_PS",      ///< ASCII punctuation and symbols.

  "C1_CONTROLS",   ///< Controls of the C1 Controls and Latin-1 Supplement unicode block.
  "C1_PS",         ///< Punctuation and symbols of the C1 Controls and Latin-1 Supplement unicode block.
  "C1_MATH",       ///< Math symbols of the C1 Controls and Latin-1 Supplement unicode block.

  "SML_P",         ///< Punctuation symbols of the Spacing Modifier Letters unicode block.
  "PHONETIC_U",    ///< Uralic phonetic symbols of the Phonetic Extensions unicode block.
  "PHONETIC_SS",   ///< Subscripts and superscripts of the Phonetic Extensions unicode block.

  "NUMERIC_SS",    ///< Numeric subscripts and superscripts.

  "LETTER_LIKE",   ///< Symbols of the Letterlike unicode block.
  "NUMBER_FORMS",  ///< Number Forms unicode block.
  "FRACTIONS_NF",  ///< Numeric fraction symbols of the Number Forms unicode block.
  "NON_LATIN_LED", ///< Non latin symbols within the Latin Extended D unicode block.
  "HWFW_S",        ///< Non latin symbols within the Halfwidth and fullwidth unicode block.

  "CYRILLIC",      ///< The Cyrillic script. Used by Russian, Bulgarian, Ukrainian, Macedonian, ...
  "GREEK",         ///< The Greek script. Used by Greek.
  "LATIN",         ///< The latin script. Used by many western languages and others around the world.

  "ARABIC",        ///< The arabic script. Used by Arab and Urdu among others.
  "HEBREW",        ///< The Hebrew script. Used by the Hebrew, Yiddish, Ladino, and Judeo-Arabic.

  "ARMENIAN",      ///< The Armenian script. Used by Armenian.
  "GEORGIAN",      ///< The Georgian script. Used by Georgian.

  "CJK",           ///< The CJK script. Used by Chinese, Japanese, Korean and Vietnamese(old writing system).
  "HANGUL",        ///< The Hangul jamo script. Used by Korean.
  "HIRAGANA",      ///< The Hiragana script. Used by the Japanese.
  "KATAKANA",      ///< The Katakana script. Used by the Japanese.
  "BOPOMOFO",      ///< The Bopomofo script. Also called Zhuyin fuhao or Zhuyin. A phonetic notation used for the transcription of spoken Chinese.

  "BENGALI",       ///< The Bengali script. Used by Bangla, Assamese, Bishnupriya Manipuri, Daphla, Garo, Hallam, Khasi, Mizo, Munda, Naga, Rian, and Santali.
  "BURMESE",       ///< The Burmese script. Used by the Burmese (Myanmar) language.
  "DEVANAGARI",    ///< The devanagari script. Used by Hindi, Marathi, Sindhi, Nepali and Sanskrit.
  "GUJARATI",      ///< The Gujarati script. Used by Gujarati.
  "GURMUKHI",      ///< The Gurmukhi script. Used by Punjabi.
  "KANNADA",       ///< The Kannada script. Used by Kannada and Tulu.
  "MALAYALAM",     ///< The Malayalam script. Used by Malayalam.
  "ORIYA",         ///< The Oriya script. Used by Oriya (Odia), Khondi, and Santali.
  "SINHALA",       ///< The Sinhala script. Used by Sinhala and Pali.
  "TAMIL",         ///< The Tamil script. Used by Tamil, Badaga, and Saurashtra.
  "TELUGU",        ///< The Telugu script. Used by Telugu, Gondi, and Lambadi.

  "LAO",           ///< The Lao script. Used by the Lao language.
  "THAI",          ///< The Thai script. Used by the Thai language
  "KHMER",         ///< The Khmer script. Used by the Khmer language.
  "JAVANESE",      ///< The Javanese script. Used by the Javanese language.
  "SUNDANESE",     ///< The Sundanese script. Used by the Sundanese language.

  "GEEZ",          ///< The Ge'ez script also known as Ethiopic. Used by the Amharic, Tigrinya and other languages in Ethiopia and Eritrea.
  "OL_CHIKI",      ///< The Ol Chiki script. Used by the Santali.
  "BAYBAYIN",      ///< The Baybayin script. Used by the Tagalog, Bikol languages, Ilocano, Pangasinan, Visayan and other languages in Philippines.
  "MEITEI",        ///< The Meitei script used for the Meitei language in Manipur, India.

  "EMOJI",         ///< The Emoji which map to standardized Unicode characters.

  "SYMBOLS1",      ///< Some symbols.
  "SYMBOLS2",      ///< Some symbols.
  "SYMBOLS3",      ///< Some symbols.
  "SYMBOLS4",      ///< Some symbols.
  "SYMBOLS5",      ///< Some symbols.

  "UNKNOWN"        ///< The script is unknown.
};

/**
 * @brief Whether the script is a right to left script.
 *
 * @param[in] script The script.
 *
 * @return @e true if the script is right to left.
 */
DALI_ADAPTOR_API bool IsRightToLeftScript( Script script );

/**
 * @brief Retrieves a character's script.
 *
 * @param[in] character The character.
 *
 * @return The chraracter's script.
 */
DALI_ADAPTOR_API Script GetCharacterScript( Character character );

/**
 * @brief Whether the character is a white space.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is a white space.
 */
DALI_ADAPTOR_API bool IsWhiteSpace( Character character );

/**
 * @brief Whether the character is a new paragraph character.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is a new paragraph character.
 */
DALI_ADAPTOR_API bool IsNewParagraph( Character character );

/**
 * @brief Whether the character is a zero width non joiner.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is a zero width non joiner.
 */
DALI_ADAPTOR_API bool IsZeroWidthNonJoiner( Character character );

/**
 * @brief Whether the character is a zero width joiner.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is a zero width joiner.
 */
DALI_ADAPTOR_API bool IsZeroWidthJoiner( Character character );

/**
 * @brief Whether the character is a zero width space.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is a zero width space.
 */
DALI_ADAPTOR_API bool IsZeroWidthSpace( Character character );

/**
 * @brief Whether the character is a left to right mark.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is a left to right mark.
 */
DALI_ADAPTOR_API bool IsLeftToRightMark( Character character );

/**
 * @brief Whether the character is a right to left mark.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is a right to left mark.
 */
DALI_ADAPTOR_API bool IsRightToLeftMark( Character character );

/**
 * @brief Whether the character is a thin space.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is a thin space.
 */
DALI_ADAPTOR_API bool IsThinSpace( Character character );

/**
 * @brief Whether the character is common within all scripts.
 *
 * @param[in] character The character.
 *
 * @return @e true if the character is common within all scripts.
 */
DALI_ADAPTOR_API bool IsCommonScript( Character character );

/**
 * @brief Whether the script contains ligatures that must be 'broken' for selection or cursor position.
 *
 * i.e The latin script has the 'ff' or 'fi' ligatures that need to be broken to position the cursor
 * between the two characters. Equally the arabic script has the 'ﻻ' ligature that needs to be broken.
 *
 * @param[in] script The script.
 *
 * @return @e true if the script has ligatures that must be 'broken'.
 */
DALI_ADAPTOR_API bool HasLigatureMustBreak( Script script );
} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_TOOLKIT_TEXT_ABSTRACTION_SCRIPT_H
