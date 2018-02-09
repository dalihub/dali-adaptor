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

// FILE HEADER
#include <dali/devel-api/text-abstraction/script.h>

namespace Dali
{

namespace TextAbstraction
{

namespace
{
const unsigned int WHITE_SPACE_THRESHOLD  = 0x21; ///< All characters below 0x21 are considered white spaces.
const unsigned int CHAR_LF   = 0x000A; ///< NL Line feed, new line.
const unsigned int CHAR_VT   = 0x000B; ///< Vertical tab.
const unsigned int CHAR_FF   = 0x000C; ///< NP Form feed, new page.
const unsigned int CHAR_CR   = 0x000D; ///< Carriage return, new line.
const unsigned int CHAR_NEL  = 0x0085; ///< Next line.
const unsigned int CHAR_LS   = 0x2028; ///< Line separator.
const unsigned int CHAR_PS   = 0x2029; ///< Paragraph separator

const unsigned int CHAR_ZWS  = 0x200B; ///< Zero width space.
const unsigned int CHAR_ZWNJ = 0x200C; ///< Zero width non joiner.
const unsigned int CHAR_ZWJ  = 0x200D; ///< Zero width joiner.
const unsigned int CHAR_LTRM = 0x200E; ///< Left to Right Mark.
const unsigned int CHAR_RTLM = 0x200F; ///< Right to Left Mark.
const unsigned int CHAR_TS   = 0x2009; ///< Thin Space.
} // namespace

bool IsRightToLeftScript( Script script )
{
  return ( ( ARABIC == script ) ||
           ( HEBREW == script ) );
}

Script GetCharacterScript( Character character )
{
  // Latin script:   It contains punctuation characters and symbols which are not part of the latin script. https://en.wikipedia.org/wiki/Latin_script_in_Unicode
  // 0x0000 - 0x007f C0 Controls and Basic Latin
  //
  //                 ASCII digits (not part of LATIN script):
  //                 0x0030 - 0x0039
  //
  //                 ASCII punctuation and symbols (not part of LATIN script):
  //                 0x0020 - 0x002F
  //                 0x003A - 0x0040
  //                 0x005B - 0x0060
  //                 0x007B - 0x007E
  //
  //                 Controls (not part of LATIN script):
  //                 0x007F
  //
  // 0x0080 - 0x00ff C1 Controls and Latin-1 Supplement
  //
  //                 Controls (not part of LATIN script):
  //                 0x0080 - 0x009F
  //
  //                 Punctuations and symbols (not part of LATIN script):
  //                 0x00A0 - 0x00BF
  //
  //                 Mathematical operators (not part of LATIN script):
  //                 0x00D7
  //                 0x00F7
  //
  // 0x0100 - 0x017f Latin Extended-A
  // 0x0180 - 0x024f Latin Extended-B
  // 0x0250 - 0x02af IPA Extensions
  // 0x02b0 - 0x02ff Spacing Modifier Letters
  //
  //                 Punctuation (not part of LATIN script):
  //                 0x02B9 - 0x02BF
  //
  // 0x1d00 - 0x1d7f Phonetic Extensions
  //
  //                 Uralic Phonetic (not part of LATIN script):
  //                 0x1D26 - 0x1D2B
  //
  //                 Subscripts and superscripts
  //                 0x1D5D - 0x1D61
  //                 0x1D66 - 0x1D6A
  //                 0x1D78
  //
  // 0x1d80 - 0x1dbf Phonetic Extensions Supplement
  //
  //                 0x1DBF (subscript or superscript. Not part of LATIN script )
  //
  // 0x1e00 - 0x1eff Latin Extended Additional
  // 0x2070 - 0x209f Superscripts and Subscripts
  //
  //                 0x2070          (not part of LATIN script)
  //                 0x2074 - 0x207E (not part of LATIN script)
  //
  // 0x2100 - 0x214f Letterlike symbols (not part of LATIN script)
  //
  //                 0x212A - 0x212B (are part of LATIN script)
  //                 0x2132          (are part of LATIN script)
  //                 0x214E          (are part of LATIN script)
  //
  // 0x2150 - 0x2189 Number Forms
  //
  //                 0x2150 - 0x215F Fractions (not part of LATIN script)
  //                 0x2189          Fractions (not part of LATIN script)
  //
  // 0x2c60 - 0x2c7f Latin Extended-C
  // 0xa720 - 0xa7ff Latin Extended-D
  //
  //                 0xA720 - 0xA721 Uralic Phonetic (not part of LATIN script)
  //                 0xA788          (not part of LATIN script)
  //                 0xA789 - 0xA78A Budu (not part of LATIN script)
  //
  // 0xab30 - 0xab6f Latin Extended-E
  //
  // 0xfb00 - 0xfb06 Latin Alphabetic Presentation Forms
  // 0xff00 - 0xffef Halfwidth and Fullwidth Forms
  //
  //                 0xFF00 - 0xFF20 HWFW Symbols (not part of LATIN script)
  //                 0xFF3B - 0xFF40 HWFW Symbols (not part of LATIN script)
  //                 0xFF5B - 0xFFEF HWFW Symbols (not part of LATIN script)

  // Brahmic scripts:
  // 0x0900 - 0x097f Devanagari
  // 0x0980 - 0x09ff Bengali
  // 0x0a00 - 0x0a7f Gurmukhi
  // 0x0a80 - 0x0aff Gujarati
  // 0x0b00 - 0x0b7f Oriya
  // 0x0b80 - 0x0bff Tamil
  // 0x0c00 - 0x0c7f Telugu
  // 0x0c80 - 0x0cff Kannada
  // 0x0d00 - 0x0d7f Malayalam

  // Sinhala script.
  // 0x0d80 - 0x0dff Sinhala

  // Arabic script.
  // 0x0600 - 0x06ff Arabic
  // 0x0750 - 0x077f Arabic Supplement
  // 0x08A0 - 0x08ff Arabic Extended-A
  // 0xfb50 - 0xfdff Arabic Presentation Forms-A
  // 0xfe70 - 0xfeff Arabic Presentation Forms-B
  // 0x1ee00 - 0x1eeff Arabic Mathematical Alphabetic Symbols

  // CJK (Chinese, Japanese and Korean) and Vietnamese script.
  // 0x2e80 - 0x2eff CJK Radicals Supplement
  // 0x2f00 - 0x2fdf Kangxi Radicals
  // 0x3000 - 0x303f CJK Symbols and Punctuation
  // 0x3200 - 0x32ff Enclosed CJK Letters and Months
  // 0x3400 - 0x4dbf CJK Unified Ideographs Extension A
  // 0x4e00 - 0x62ff CJK Unified Ideographs
  // 0x6300 - 0x77ff CJK Unified Ideographs
  // 0x7800 - 0x8cff CJK Unified Ideographs
  // 0x8d00 - 0x9fff CJK Unified Ideographs
  // 0x20000 - 0x215ff CJK Unified Ideographs Extension B
  // 0x21600 - 0x230ff CJK Unified Ideographs Extension B
  // 0x23100 - 0x245ff CJK Unified Ideographs Extension B
  // 0x24600 - 0x260ff CJK Unified Ideographs Extension B
  // 0x26100 - 0x275ff CJK Unified Ideographs Extension B
  // 0x27600 - 0x290ff CJK Unified Ideographs Extension B
  // 0x29100 - 0x2a6df CJK Unified Ideographs Extension B
  // 0x2a700 - 0x2b73f CJK Unified Ideographs Extension C
  // 0x2b740 - 0x2b81f CJK Unified Ideographs Extension D

  // Japanese scripts.
  // 0x3040 - 0x309f Hiragana
  // 0x30a0 - 0x30ff Katakana

  // Hangul script
  // 0x1100 - 0x11ff Hangul jamo
  // 0x3130 - 0x318f Hangul Compatibility Jamo
  // 0xa960 - 0xa97f Hangul Jamo Extended-A
  // 0xac00 - 0xd7af Hangul Syllables
  // 0xd7b0 - 0xd7ff Hangul Jamo Extended-B

  // Bopomofo script
  // 0x3100 - 0x312f Bopomofo
  // 0x31a0 - 0x31bf Bopomofo Extended

  // Khmer script
  // 0x1780 - 0x17ff Khmer
  // 0x19e0 - 0x19ff Khmer Symbols

  // Lao script
  // 0x0e80 - 0x0eff Lao

  // Thai script
  // 0x0e00 - 0x0e7f Thai

  // Burmese script
  // 0x1000 - 0x109f Myanmar

  // Hebrew script
  // 0x0591 - 0x05f4 Hebrew
  // 0xfb1d - 0xfb4f Hebrew subset of Alphabetic Presentation Forms

  // Cyrillic script
  // 0x0400 - 0x04ff Cyrillic
  // 0x0500 - 0x052f Cyrillic suplement
  // 0x2de0 - 0x2dff Cyrillic Extended-A
  // 0xa640 - 0xa69f Cyrillic Extended-B

  // Georgian script
  // 0x10a0 - 0x10ff Georgian
  // 0x2d00 - 0x2d2f Georgian suplement

  // Greek script
  // 0x0370 - 0x03ff Greek & Coptic
  // 0x1f00 - 0x1fff Greek Extended

  // Armenian script
  // 0x0530 - 0x058f Armenian
  // 0xfb13 - 0xfb17 Armenian subset of Alphabetic prefentation forms

  // Javanese script
  // 0xa980 - 0xa9fd Javanese

  // Sundanese script
  // 0x1b80 - 0x1bbf Sundanese
  // 0x1cc0 - 0x1ccf Sundanese supplement

  // Ge'ez script (Ethiopic)
  // 0x1200 - 0x137f Ethiopic
  // 0x1380 - 0x139f Ethiopic supplement
  // 0x2d80 - 0x2ddf Ethiopic Extended
  // 0xab00 - 0xab2f Ethiopic Extended-A

  // Baybayin Script
  // 0x1700 - 0x171f Baybayin

  // Ol Chiki Script
  // 0x1c50 - 0x1c7f Ol Chiki

  // Meitei Script
  // 0xabc0 - 0xabff Meetei Mayek
  // 0xaae0 - 0xaaff Meetei Mayek Extensions

  // The Emoji which map to standardized Unicode characters
  // 1. Emoticons ( 1F601 - 1F64F )
  // 2. Dingbats ( 2700 - 27BF )
  // 3. Transport and map symbols ( 1F680 - 1F6C0 )
  // 4. Enclosed characters ( 24C2 - 1F251 )
  // 5. Uncategorized :-S
  // 6. Additional Emoticons ( 1F600 - 1F636 )
  // 6b. Additional transport and map symbols ( 1F680 - 1F6FF ): http://unicode.org/charts/PDF/U1F680.pdf
  // 6c. Other additional symbols ( 1F30D - 1F567 )
  // 7. Supplemental Symbols and Pictographs ( 1F900–1F9FF ): http://unicode.org/charts/PDF/U1F900.pdf

  // Symbols. Work around for these symbols.
  // 0x25cb
  // 0x25cf
  // 0x25a1
  // 0x25a0
  // 0x2664
  // 0x2661
  // 0x2662
  // 0x2667
  // 0x2606
  // 0x25aa
  // 0x262a

  if( IsCommonScript( character ) )
  {
    return COMMON;
  }

  if( character <= 0x0cff )
  {
    if( character <= 0x09ff )
    {
      if( character <= 0x077f )
      {
        if( ( 0x0030 <= character ) && ( character <= 0x0039 ) )
        {
          return ASCII_DIGITS;
        }
        if( character <= 0x007E )
        {
          if( ( 0x0020 <= character ) && ( character <= 0x002F ) )
          {
            return ASCII_PS;
          }
          if( ( 0x003A <= character ) && ( character <= 0x0040 ) )
          {
            return ASCII_PS;
          }
          if( ( 0x005B <= character ) && ( character <= 0x0060 ) )
          {
            return ASCII_PS;
          }
          if( ( 0x007B <= character ) && ( character <= 0x007E ) )
          {
            return ASCII_PS;
          }
        }
        if( ( 0x007F <= character ) && ( character <= 0x009F ) )
        {
          // 0x007F is actually part of C0 Controls and Basic Latin. However, is the last and only control character of its block
          // and the following characters of the next block are consecutive.
          return C1_CONTROLS;
        }
        if( ( 0x00A0 <= character ) && ( character <= 0x00BF ) )
        {
          if( character == 0x00A9 )
          {
            return EMOJI; // 5. Uncategorized: copyright sign
          }
          if( character == 0x00AE )
          {
            return EMOJI; // 5. Uncategorized: registered sign
          }

          return C1_PS;
        }
        if( character == 0x00D7 )
        {
          return C1_MATH;
        }
        if( character == 0x00F7 )
        {
          return  C1_MATH;
        }
        if( character <= 0x02ff )
        {
          if( ( 0x02B9 <= character ) && ( character <= 0x02BF ) )
          {
            return SML_P;
          }

          return LATIN;
        }
        if( ( 0x0370 <= character ) && ( character <= 0x03ff ) )
        {
          return GREEK;
        }
        if( ( 0x0400 <= character ) && ( character <= 0x04ff ) )
        {
          return CYRILLIC;
        }
        if( ( 0x0500 <= character ) && ( character <= 0x052f ) )
        {
          return CYRILLIC;
        }
        if( ( 0x0530 <= character ) && ( character <= 0x058f ) )
        {
          return ARMENIAN;
        }
        if( ( 0x0591 <= character ) && ( character <= 0x05f4 ) )
        {
          return HEBREW;
        }
        if( ( 0x0600 <= character ) && ( character <= 0x06ff ) )
        {
          return ARABIC;
        }
        if( ( 0x0750 <= character ) && ( character <= 0x077f ) )
        {
          return ARABIC;
        }
      }
      else // > 0x077f
      {
        if( ( 0x08A0 <= character ) && ( character <= 0x08ff ) )
        {
          return ARABIC;
        }
        if( ( 0x0900 <= character ) && ( character <= 0x097f ) )
        {
          return DEVANAGARI;
        }
        if( ( 0x0980 <= character ) && ( character <= 0x09ff ) )
        {
          return BENGALI;
        }
      }
    }
    else // > 0x09ff
    {
      if( character <= 0x0b7f )
      {
        if( ( 0x0a00 <= character ) && ( character <= 0x0a7f ) )
        {
          return GURMUKHI;
        }
        if( ( 0x0a80 <= character ) && ( character <= 0x0aff ) )
        {
          return GUJARATI;
        }
        if( ( 0x0b00 <= character ) && ( character <= 0x0b7f ) )
        {
          return ORIYA;
        }
      }
      else // > 0x0b7f
      {
        if( ( 0x0b80 <= character ) && ( character <= 0x0bff ) )
        {
          return TAMIL;
        }
        if( ( 0x0c00 <= character ) && ( character <= 0x0c7f ) )
        {
          return TELUGU;
        }
        if( ( 0x0c80 <= character ) && ( character <= 0x0cff ) )
        {
          return KANNADA;
        }
      }
    }
  }
  else // > 0x0cff
  {
    if( character <= 0x2c7f )
    {
      if( character <= 0x1eff )
      {
        if( ( 0x0d00 <= character ) && ( character <= 0x0d7f ) )
        {
          return MALAYALAM;
        }
        if( ( 0x0d80 <= character ) && ( character <= 0x0dff ) )
        {
          return SINHALA;
        }
        if( ( 0x0e00 <= character ) && ( character <= 0x0e7f ) )
        {
          return THAI;
        }
        if( ( 0x0e80 <= character ) && ( character <= 0x0eff ) )
        {
          return LAO;
        }
        if( ( 0x1000 <= character ) && ( character <= 0x109f ) )
        {
          return BURMESE;
        }
        if( ( 0x10a0 <= character ) && ( character <= 0x10ff ) )
        {
          return GEORGIAN;
        }
        if( ( 0x1100 <= character ) && ( character <= 0x11ff ) )
        {
          return HANGUL;
        }
        if( ( 0x1200 <= character ) && ( character <= 0x137f ) )
        {
          return GEEZ;
        }
        if( ( 0x1380 <= character ) && ( character <= 0x139f ) )
        {
          return GEEZ;
        }
        if( ( 0x1700 <= character ) && ( character <= 0x171f ) )
        {
          return BAYBAYIN;
        }
        if( ( 0x1780 <= character ) && ( character <= 0x17ff ) )
        {
          return KHMER;
        }
        if( ( 0x19e0 <= character ) && ( character <= 0x19ff ) )
        {
          return KHMER;
        }
        if( ( 0x1b80 <= character ) && ( character <= 0x1bbf ) )
        {
          return SUNDANESE;
        }
        if( ( 0x1c50 <= character ) && ( character <= 0x1c7f ) )
        {
          return OL_CHIKI;
        }
        if( ( 0x1cc0 <= character ) && ( character <= 0x1ccf ) )
        {
          return SUNDANESE;
        }
        if( ( 0x1d00 <= character ) && ( character <= 0x1eff ) )
        {
          if( ( 0x1D26 <= character ) && ( character <= 0x1D2B ) )
          {
            return PHONETIC_U;
          }
          if( ( 0x1D5D <= character ) && ( character <= 0x1D61 ) )
          {
            return PHONETIC_SS;
          }
          if( ( 0x1D66 <= character ) && ( character <= 0x1D6A ) )
          {
            return PHONETIC_SS;
          }
          if( character == 0x1D78 )
          {
            return PHONETIC_SS;
          }
          if( character == 0x1DBF)
          {
            return PHONETIC_SS;
          }

          return LATIN;
        }
      }
      else // > 0x1eff
      {
        if( ( 0x1f00 <= character ) && ( character <= 0x1fff ) )
        {
          return GREEK;
        }
        if( character == 0x203c )
        {
          return EMOJI; // 5. Uncategorized: double exclamation mark
        }
        if( character == 0x2049 )
        {
          return EMOJI; // 5. Uncategorized: exclamation question mark
        }
        if( ( 0x2070 <= character ) && ( character <= 0x209f ) )
        {
          if( character == 0x2070 )
          {
            return NUMERIC_SS;
          }
          if( ( 0x2074 <= character ) && ( character <= 0x207E ) )
          {
            return NUMERIC_SS;
          }

          return LATIN;
        }
        if( character == 0x20e3 )
        {
          return EMOJI; // 5. Uncategorized: combining enclosing keycap
        }
        if( character == 0x2122 )
        {
          return EMOJI; // 5. Uncategorized: trade mark sign
        }
        if( character == 0x2139 )
        {
          return EMOJI; // 5. Uncategorized: information source
        }
        if( ( 0x2100 <= character ) && ( character <= 0x2189 ) )
        {
          if( ( 0x2100 <= character ) && ( character <= 0x214f ) )
          {
            if( ( 0x212A <= character ) && ( character <= 0x212B ) )
            {
              return LATIN;
            }
            if( character == 0x2132 )
            {
              return LATIN;
            }
            if( character == 0x214E )
            {
              return LATIN;
            }

            return LETTER_LIKE;
          }
          if( ( 0x2150 <= character ) && ( character <= 0x215F ) )
          {
            return FRACTIONS_NF;
          }
          if( character == 0x2189 )
          {
            return FRACTIONS_NF;
          }

          return LATIN;
        }

        // Symbols
        if( ( 0x25cb == character ) ||
            ( 0x25cf == character ) ||
            ( 0x25a1 == character ) )
        {
          return SYMBOLS1;
        }

        if( 0x25a0 == character )
        {
          return SYMBOLS2;
        }

        if( ( 0x2664 == character ) ||
            ( 0x2661 == character ) ||
            ( 0x2662 == character ) ||
            ( 0x2667 == character ) )
        {
          return SYMBOLS3;
        }

        if( ( 0x2606 == character ) ||
            ( 0x25aa == character ) )
        {
          return SYMBOLS4;
        }

        if( 0x262a == character )
        {
          return SYMBOLS5;
        }

        // U+2194 5. Uncategorized: left right arrow
        // U+2B55 5. Uncategorized: heavy large circle
        if( ( 0x2194 <= character ) && ( character <= 0x2B55 ) )
        {
          return EMOJI;
        }
        if( ( 0x2c60 <= character ) && ( character <= 0x2c7f ) )
        {
          return LATIN;
        }
      }
    }
    else // > 0x2c7f
    {
      if( character <= 0xfdff )
      {
        if( ( 0x2d00 <= character ) && ( character <= 0x2d2f ) )
        {
          return GEORGIAN;
        }
        if( ( 0x2d80 <= character ) && ( character <= 0x2ddf ) )
        {
          return GEEZ;
        }
        if( ( 0x2de0 <= character ) && ( character <= 0x2dff ) )
        {
          return CYRILLIC;
        }
        if( ( 0x2e80 <= character ) && ( character <= 0x2eff ) )
        {
          return CJK;
        }
        if( ( 0x2f00 <= character ) && ( character <= 0x2fdf ) )
        {
          return CJK;
        }
        if( ( 0x3000 <= character ) && ( character <= 0x303f ) )
        {
          return CJK;
        }
        if( ( 0x3040 <= character ) && ( character <= 0x309f ) )
        {
          return HIRAGANA;
        }
        if( ( 0x30a0 <= character ) && ( character <= 0x30ff ) )
        {
          return KATAKANA;
        }
        if( ( 0x3100 <= character ) && ( character <= 0x312f ) )
        {
          return BOPOMOFO;
        }
        if( ( 0x3130 <= character ) && ( character <= 0x318f ) )
        {
          return HANGUL;
        }
        if( ( 0x31a0 <= character ) && ( character <= 0x31bf ) )
        {
          return BOPOMOFO;
        }
        if( ( 0x3200 <= character ) && ( character <= 0x32ff ) )
        {
          return CJK;
        }
        if( ( 0x3400 <= character ) && ( character <= 0x4dbf ) )
        {
          return CJK;
        }
        if( ( 0x4e00 <= character ) && ( character <= 0x62ff ) )
        {
          return CJK;
        }
        if( ( 0x6300 <= character ) && ( character <= 0x77ff ) )
        {
          return CJK;
        }
        if( ( 0x7800 <= character ) && ( character <= 0x8cff ) )
        {
          return CJK;
        }
        if( ( 0x8d00 <= character ) && ( character <= 0x9fff ) )
        {
          return CJK;
        }
        if( ( 0xa640 <= character ) && ( character <= 0xa69f ) )
        {
          return CYRILLIC;
        }
        if( ( 0xa720 <= character ) && ( character <= 0xa7ff ) )
        {
          if( character == 0xA720 )
          {
            return PHONETIC_U;
          }
          if( character == 0xA721 )
          {
            return PHONETIC_U;
          }
          if( character == 0xA788 )
          {
            return NON_LATIN_LED;
          }
          if( character == 0xA789 )
          {
            return NON_LATIN_LED;
          }
          if( character == 0xA78A )
          {
            return NON_LATIN_LED;
          }

          return LATIN;
        }
        if( ( 0xa960 <= character ) && ( character <= 0xa97f ) )
        {
          return HANGUL;
        }
        if( ( 0xa980 <= character ) && ( character <= 0xa9fd ) )
        {
          return JAVANESE;
        }
        if( ( 0xab00 <= character ) && ( character <= 0xab2f ) )
        {
          return GEEZ;
        }
        if( ( 0xab30 <= character ) && ( character <= 0xab6f ) )
        {
          return LATIN;
        }
        if( ( 0xaae0 <= character ) && ( character <= 0xaaff ) )
        {
          return MEITEI;
        }
        if( ( 0xabc0 <= character ) && ( character <= 0xabff ) )
        {
          return MEITEI;
        }
        if( ( 0xac00 <= character ) && ( character <= 0xd7af ) )
        {
          return HANGUL;
        }
        if( ( 0xd7b0 <= character ) && ( character <= 0xd7ff ) )
        {
          return HANGUL;
        }
        if( ( 0xfb00 <= character ) && ( character <= 0xfb06 ) )
        {
          return LATIN;
        }
        if( ( 0xfb13 <= character ) && ( character <= 0xfb17 ) )
        {
          return ARMENIAN;
        }
        if( ( 0xfb1d <= character ) && ( character <= 0xfb4f ) )
        {
          return HEBREW;
        }
        if( ( 0xfb50 <= character ) && ( character <= 0xfdff ) )
        {
          return ARABIC;
        }
      }
      else // > 0xfdff
      {
        if( ( 0xfe70 <= character ) && ( character <= 0xfeff ) )
        {
          return ARABIC;
        }
        if( ( 0xff00 <= character ) && ( character <= 0xffef ) )
        {
          if( ( 0xFF00 <= character ) && ( character <= 0xFF20 ) )
          {
            return HWFW_S;
          }
          if( ( 0xFF3B <= character ) && ( character <= 0xFF40 ) )
          {
            return HWFW_S;
          }
          if( ( 0xFF5B <= character ) && ( character <= 0xFFEF ) )
          {
            return HWFW_S;
          }

          return LATIN;
        }
        if( ( 0x1ee00 <= character ) && ( character <= 0x1eeff ) )
        {
          return ARABIC;
        }
        // U+1f170 4. Enclosed characters: negative squared latin capital letter A
        // U+1f6ff 6b. Additional transport and map symbols
        if( ( 0x1f170 <= character ) && ( character <= 0x1f6ff ) )
        {
          return EMOJI;
        }
        // 7. Supplemental Symbols and Pictographs
        if( ( 0x1f900 <= character ) && ( character <= 0x1f9ff ) )
        {
          return EMOJI;
        }
        if( ( 0x20000 <= character ) && ( character <= 0x215ff ) )
        {
          return CJK;
        }
        if( ( 0x21600 <= character ) && ( character <= 0x230ff ) )
        {
          return CJK;
        }
        if( ( 0x23100 <= character ) && ( character <= 0x245ff ) )
        {
          return CJK;
        }
        if( ( 0x24600 <= character ) && ( character <= 0x260ff ) )
        {
          return CJK;
        }
        if( ( 0x26100 <= character ) && ( character <= 0x275ff ) )
        {
          return CJK;
        }
        if( ( 0x27600 <= character ) && ( character <= 0x290ff ) )
        {
          return CJK;
        }
        if( ( 0x29100 <= character ) && ( character <= 0x2a6df ) )
        {
          return CJK;
        }
        if( ( 0x2a700 <= character ) && ( character <= 0x2b73f ) )
        {
          return CJK;
        }
        if( ( 0x2b740 <= character ) && ( character <= 0x2b81f ) )
        {
          return CJK;
        }
      }
    }
  }

  return UNKNOWN;
}

bool IsWhiteSpace( Character character )
{
  return character < WHITE_SPACE_THRESHOLD;
}

bool IsNewParagraph( Character character )
{
  return ( ( CHAR_LF == character )  ||
           ( CHAR_VT == character )  ||
           ( CHAR_FF == character )  ||
           ( CHAR_CR == character )  ||
           ( CHAR_NEL == character ) ||
           ( CHAR_LS == character )  ||
           ( CHAR_PS == character ) );
}

bool IsZeroWidthNonJoiner( Character character )
{
  return CHAR_ZWNJ == character;
}

bool IsZeroWidthJoiner( Character character )
{
  return CHAR_ZWJ == character;
}

bool IsZeroWidthSpace( Character character )
{
  return CHAR_ZWS == character;
}

bool IsLeftToRightMark( Character character )
{
  return CHAR_LTRM == character;
}

bool IsRightToLeftMark( Character character )
{
  return CHAR_RTLM == character;
}

bool IsThinSpace( Character character )
{
  return CHAR_TS == character;
}

bool IsCommonScript( Character character )
{
  return ( IsWhiteSpace( character )         ||
           IsZeroWidthNonJoiner( character ) ||
           IsZeroWidthJoiner( character )    ||
           IsZeroWidthSpace( character )     ||
           IsLeftToRightMark( character )    ||
           IsRightToLeftMark( character )    ||
           IsThinSpace( character )          ||
           IsNewParagraph( character ) );
}

bool HasLigatureMustBreak( Script script )
{
  return ( ( LATIN == script ) ||
           ( ARABIC == script ) );
}

} // namespace TextAbstraction

} // namespace Dali
