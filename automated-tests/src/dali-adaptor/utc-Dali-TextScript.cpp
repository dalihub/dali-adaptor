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

#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/devel-api/text-abstraction/script.h>

using namespace Dali;
using namespace Dali::TextAbstraction;

void utc_dali_text_script_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_text_script_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
void GetCharacterScriptCheckRange(Character start, Character end, Script expectedScript, const char* location)
{
  for(auto character = start; character <= end; ++character)
  {
    if(!IsCommonScript(character)) // Some characters are part of the common script and can be in the range
    {
      DALI_TEST_EQUALS(expectedScript, GetCharacterScript(character), location);
    }
  }
}
} // namespace

int UtcDaliGetCharacterScript(void)
{
  GetCharacterScriptCheckRange(0x0000, 0x001F, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0020, 0x002F, ASCII_PS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0030, 0x0039, ASCII_DIGITS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x003A, 0x0040, ASCII_PS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0041, 0x005A, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x005B, 0x0060, ASCII_PS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0061, 0x007A, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x007B, 0x007E, ASCII_PS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x007F, 0x009F, C1_CONTROLS, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x00A0, 0x00A8, C1_PS, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x00A9), EMOJI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x00AA, 0x00AD, C1_PS, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x00AE), EMOJI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x00AF, 0x00BF, C1_PS, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x00C0, 0x00D6, LATIN, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x00D7), C1_MATH, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x00D8, 0x00F6, LATIN, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x00F7), C1_MATH, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x00F8, 0x02B8, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x02B9, 0x02BF, SML_P, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x02C0, 0x02FF, LATIN, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x0370, 0x03ff, GREEK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0400, 0x04ff, CYRILLIC, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0500, 0x052f, CYRILLIC, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0530, 0x058f, ARMENIAN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0591, 0x05f4, HEBREW, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0600, 0x06ff, ARABIC, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0750, 0x077f, ARABIC, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x08A0, 0x08ff, ARABIC, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0900, 0x097f, DEVANAGARI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0980, 0x09ff, BENGALI, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x0a00, 0x0a7f, GURMUKHI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0a80, 0x0aff, GUJARATI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0b00, 0x0b7f, ORIYA, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x0b80, 0x0bff, TAMIL, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0c00, 0x0c7f, TELUGU, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0c80, 0x0cff, KANNADA, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x0d00, 0x0d7f, MALAYALAM, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0d80, 0x0dff, SINHALA, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0e00, 0x0e7f, THAI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x0e80, 0x0eff, LAO, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1000, 0x109f, BURMESE, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x10a0, 0x10ff, GEORGIAN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1100, 0x11ff, HANGUL, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1200, 0x137f, GEEZ, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1380, 0x139f, GEEZ, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1700, 0x171f, BAYBAYIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1780, 0x17ff, KHMER, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x19e0, 0x19ff, KHMER, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1b80, 0x1bbf, SUNDANESE, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1c50, 0x1c7f, OL_CHIKI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1cc0, 0x1ccf, SUNDANESE, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x1d00, 0x1D25, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1D26, 0x1D2B, PHONETIC_U, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1D2C, 0x1D5C, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1D5D, 0x1D61, PHONETIC_SS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1D62, 0x1D65, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1D66, 0x1D6A, PHONETIC_SS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1D6B, 0x1D77, LATIN, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x1D78), PHONETIC_SS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1D79, 0x1DBE, LATIN, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x1DBF), PHONETIC_SS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1DC0, 0x1EFF, LATIN, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x1f00, 0x1fff, GREEK, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x203c), EMOJI, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x2049), EMOJI, TEST_LOCATION);

  DALI_TEST_EQUALS(GetCharacterScript(0x2070), NUMERIC_SS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2071, 0x2073, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2074, 0x207E, NUMERIC_SS, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x207F, 0x209f, LATIN, TEST_LOCATION);

  DALI_TEST_EQUALS(GetCharacterScript(0x20e3), EMOJI, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x2100, 0x2120, LETTER_LIKE, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x2122), EMOJI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x212A, 0x212B, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x212C, 0x2131, LETTER_LIKE, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x2132), LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2133, 0x2138, LETTER_LIKE, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x2139), EMOJI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2140, 0x214D, LETTER_LIKE, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x214E), LATIN, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x214F), LETTER_LIKE, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2150, 0x215F, FRACTIONS_NF, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2160, 0x2188, LATIN, TEST_LOCATION);
  DALI_TEST_EQUALS(GetCharacterScript(0x2189), FRACTIONS_NF, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x2194, 0x259f, EMOJI, TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS2, GetCharacterScript(0x25a0), TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS1, GetCharacterScript(0x25a1), TEST_LOCATION);
  GetCharacterScriptCheckRange(0x25a2, 0x25a9, EMOJI, TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS4, GetCharacterScript(0x25aa), TEST_LOCATION);
  GetCharacterScriptCheckRange(0x25ab, 0x25ca, EMOJI, TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS1, GetCharacterScript(0x25cb), TEST_LOCATION);
  GetCharacterScriptCheckRange(0x25cc, 0x25ce, EMOJI, TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS1, GetCharacterScript(0x25cf), TEST_LOCATION);
  GetCharacterScriptCheckRange(0x25d0, 0x2605, EMOJI, TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS4, GetCharacterScript(0x2606), TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2607, 0x2629, EMOJI, TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS5, GetCharacterScript(0x262a), TEST_LOCATION);
  GetCharacterScriptCheckRange(0x262b, 0x2660, EMOJI, TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS3, GetCharacterScript(0x2661), TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS3, GetCharacterScript(0x2662), TEST_LOCATION);
  DALI_TEST_EQUALS(EMOJI, GetCharacterScript(0x2663), TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS3, GetCharacterScript(0x2664), TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2665, 0x2666, EMOJI, TEST_LOCATION);
  DALI_TEST_EQUALS(SYMBOLS3, GetCharacterScript(0x2667), TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2668, 0x2B55, EMOJI, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x2c60, 0x2c7f, LATIN, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x2c60, 0x2c7f, LATIN, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x2d00, 0x2d2f, GEORGIAN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2d80, 0x2ddf, GEEZ, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2de0, 0x2dff, CYRILLIC, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2e80, 0x2eff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2f00, 0x2fdf, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x3000, 0x303f, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x3040, 0x309f, HIRAGANA, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x30a0, 0x30ff, KATAKANA, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x3100, 0x312f, BOPOMOFO, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x3130, 0x318f, HANGUL, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x31a0, 0x31bf, BOPOMOFO, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x3200, 0x32ff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x3400, 0x4dbf, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x4e00, 0x62ff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x6300, 0x77ff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x7800, 0x8cff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x8d00, 0x9fff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xa640, 0xa69f, CYRILLIC, TEST_LOCATION);

  GetCharacterScriptCheckRange(0xa720, 0xA721, PHONETIC_U, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xa722, 0xA787, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xA788, 0xA78A, NON_LATIN_LED, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xA78B, 0xa7ff, LATIN, TEST_LOCATION);

  GetCharacterScriptCheckRange(0xa960, 0xa97f, HANGUL, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xa980, 0xa9fd, JAVANESE, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xab00, 0xab2f, GEEZ, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xab30, 0xab6f, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xaae0, 0xaaff, MEITEI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xabc0, 0xabff, MEITEI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xac00, 0xd7af, HANGUL, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xd7b0, 0xd7ff, HANGUL, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xfb00, 0xfb06, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xfb13, 0xfb17, ARMENIAN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xfb1d, 0xfb4f, HEBREW, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xfb50, 0xfdff, ARABIC, TEST_LOCATION);

  GetCharacterScriptCheckRange(0xfe70, 0xfeff, ARABIC, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xFF00, 0xFF20, HWFW_S, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xFF21, 0xFF3A, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xFF3B, 0xFF40, HWFW_S, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xFF41, 0xFF5A, LATIN, TEST_LOCATION);
  GetCharacterScriptCheckRange(0xFF5B, 0xFFEF, HWFW_S, TEST_LOCATION);

  GetCharacterScriptCheckRange(0x1ee00, 0x1eeff, ARABIC, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1f170, 0x1f189, SYMBOLS_NSLCL, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1f18a, 0x1f6ff, EMOJI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x1f900, 0x1f9ff, EMOJI, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x20000, 0x215ff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x21600, 0x230ff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x23100, 0x245ff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x24600, 0x260ff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x26100, 0x275ff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x27600, 0x290ff, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x29100, 0x2a6df, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2a700, 0x2b73f, CJK, TEST_LOCATION);
  GetCharacterScriptCheckRange(0x2b740, 0x2b81f, CJK, TEST_LOCATION);

  END_TEST;
}
