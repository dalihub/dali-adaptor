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

// CLASS  HEADER
#include <dali/internal/text/text-abstraction/shaping-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/singleton-service-impl.h>
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/devel-api/text-abstraction/glyph-info.h>
#include <dali/integration-api/debug.h>
#include "font-client-impl.h"

// EXTERNAL INCLUDES
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

namespace
{

#if defined(DEBUG_ENABLED)
Dali::Integration::Log::Filter* gLogFilter = Dali::Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_FONT_CLIENT");
#endif

}

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

const char* const  DEFAULT_LANGUAGE = "en";
const unsigned int DEFAULT_LANGUAGE_LENGTH = 2u;
const float        FROM_266 = 1.0f / 64.0f;

const hb_script_t SCRIPT_TO_HARFBUZZ[] =
{
  HB_SCRIPT_COMMON,

  HB_SCRIPT_COMMON, // ASCII_DIGITS
  HB_SCRIPT_COMMON, // ASCII_PS

  HB_SCRIPT_COMMON, // C1_CONTROLS
  HB_SCRIPT_COMMON, // C1_PS
  HB_SCRIPT_COMMON, // C1_MATH
  HB_SCRIPT_COMMON, // SML_P
  HB_SCRIPT_COMMON, // PHONETIC_U
  HB_SCRIPT_COMMON, // PHONETIC_SS
  HB_SCRIPT_COMMON, // NUMERIC_SS
  HB_SCRIPT_COMMON, // LETTER_LIKE
  HB_SCRIPT_COMMON, // NUMBER_FORMS
  HB_SCRIPT_COMMON, // FRACTIONS_NF
  HB_SCRIPT_COMMON, // NON_LATIN_LED
  HB_SCRIPT_COMMON, // HWFW_S

  HB_SCRIPT_CYRILLIC,
  HB_SCRIPT_GREEK,
  HB_SCRIPT_LATIN,

  HB_SCRIPT_ARABIC,
  HB_SCRIPT_HEBREW,

  HB_SCRIPT_ARMENIAN,
  HB_SCRIPT_GEORGIAN,

  HB_SCRIPT_HAN,
  HB_SCRIPT_HANGUL,
  HB_SCRIPT_HIRAGANA,
  HB_SCRIPT_KATAKANA,
  HB_SCRIPT_BOPOMOFO,

  HB_SCRIPT_BENGALI,
  HB_SCRIPT_MYANMAR,
  HB_SCRIPT_DEVANAGARI,
  HB_SCRIPT_GUJARATI,
  HB_SCRIPT_GURMUKHI,
  HB_SCRIPT_KANNADA,
  HB_SCRIPT_MALAYALAM,
  HB_SCRIPT_ORIYA,
  HB_SCRIPT_SINHALA,
  HB_SCRIPT_TAMIL,
  HB_SCRIPT_TELUGU,

  HB_SCRIPT_LAO,
  HB_SCRIPT_THAI,
  HB_SCRIPT_KHMER,
  HB_SCRIPT_JAVANESE,
  HB_SCRIPT_SUNDANESE,

  HB_SCRIPT_ETHIOPIC,
  HB_SCRIPT_OL_CHIKI,
  HB_SCRIPT_TAGALOG,
  HB_SCRIPT_MEETEI_MAYEK,

  HB_SCRIPT_UNKNOWN, // EMOJI
  HB_SCRIPT_UNKNOWN, // SYMBOLS1
  HB_SCRIPT_UNKNOWN, // SYMBOLS2
  HB_SCRIPT_UNKNOWN, // SYMBOLS3
  HB_SCRIPT_UNKNOWN, // SYMBOLS4
  HB_SCRIPT_UNKNOWN, // SYMBOLS5
  HB_SCRIPT_UNKNOWN
};

struct Shaping::Plugin
{
  Plugin()
  : mIndices(),
    mAdvance(),
    mCharacterMap(),
    mFontId( 0u )
  {
  }

  ~Plugin()
  {
  }

  Length Shape( const Character* const text,
                Length numberOfCharacters,
                FontId fontId,
                Script script )
  {
    // Clear previoursly shaped texts.
    mIndices.Clear();
    mAdvance.Clear();
    mCharacterMap.Clear();
    mOffset.Clear();
    mFontId = fontId;

    TextAbstraction::FontClient fontClient = TextAbstraction::FontClient::Get();
    TextAbstraction::Internal::FontClient& fontClientImpl = TextAbstraction::GetImplementation( fontClient );

    const FontDescription::Type type = fontClientImpl.GetFontType( fontId );

    switch( type )
    {
      case FontDescription::FACE_FONT:
      {
        // Reserve some space to avoid reallocations.
        const Length numberOfGlyphs = static_cast<Length>( 1.3f * static_cast<float>( numberOfCharacters ) );
        mIndices.Reserve( numberOfGlyphs );
        mAdvance.Reserve( numberOfGlyphs );
        mCharacterMap.Reserve( numberOfGlyphs );
        mOffset.Reserve( 2u * numberOfGlyphs );

        // Retrieve a FreeType font's face.
        FT_Face face = fontClientImpl.GetFreetypeFace( fontId );
        if( nullptr == face )
        {
          // Nothing to do if the face is null.
          return 0u;
        }

        unsigned int horizontalDpi = 0u;
        unsigned int verticalDpi = 0u;
        fontClient.GetDpi( horizontalDpi, verticalDpi );

        FT_Set_Char_Size( face,
                          0u,
                          fontClient.GetPointSize( fontId ),
                          horizontalDpi,
                          verticalDpi );

        /* Get our harfbuzz font struct */
        hb_font_t* harfBuzzFont;
        harfBuzzFont = hb_ft_font_create( face, NULL );

        /* Create a buffer for harfbuzz to use */
        hb_buffer_t* harfBuzzBuffer = hb_buffer_create();

        const bool rtlDirection = IsRightToLeftScript( script );
        hb_buffer_set_direction( harfBuzzBuffer,
                                 rtlDirection ? HB_DIRECTION_RTL : HB_DIRECTION_LTR ); /* or LTR */

        hb_buffer_set_script( harfBuzzBuffer,
                              SCRIPT_TO_HARFBUZZ[ script ] ); /* see hb-unicode.h */


        char* currentLocale = setlocale(LC_MESSAGES,NULL);

        std::istringstream stringStream( currentLocale );
        std::string localeString;
        std::getline(stringStream, localeString, '_');
        hb_buffer_set_language( harfBuzzBuffer, hb_language_from_string( localeString.c_str(), localeString.size() ) );

        /* Layout the text */
        hb_buffer_add_utf32( harfBuzzBuffer, text, numberOfCharacters, 0u, numberOfCharacters );

        hb_shape( harfBuzzFont, harfBuzzBuffer, NULL, 0u );

        /* Get glyph data */
        unsigned int glyphCount;
        hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos( harfBuzzBuffer, &glyphCount );
        hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions( harfBuzzBuffer, &glyphCount );
        const GlyphIndex lastGlyphIndex = glyphCount - 1u;

        for( GlyphIndex i = 0u; i < glyphCount; )
        {
          if( rtlDirection )
          {
            // If the direction is right to left, Harfbuzz retrieves the glyphs in the visual order.
            // The glyphs are needed in the logical order to layout the text in lines.
            // Do not change the order of the glyphs if they belong to the same cluster.
            GlyphIndex rtlIndex = lastGlyphIndex - i;

            unsigned int cluster = glyphInfo[rtlIndex].cluster;
            unsigned int previousCluster = cluster;
            Length numberOfGlyphsInCluster = 0u;

            while( ( cluster == previousCluster ) )
            {
              ++numberOfGlyphsInCluster;
              previousCluster = cluster;

              if( rtlIndex > 0u )
              {
                --rtlIndex;

                cluster = glyphInfo[rtlIndex].cluster;
              }
              else
              {
                break;
              }
            }

            rtlIndex = lastGlyphIndex - ( i + ( numberOfGlyphsInCluster - 1u ) );

            for( GlyphIndex j = 0u; j < numberOfGlyphsInCluster; ++j )
            {
              const GlyphIndex index = rtlIndex + j;

              mIndices.PushBack( glyphInfo[index].codepoint );
              mAdvance.PushBack( floor( glyphPositions[index].x_advance * FROM_266 ) );
              mCharacterMap.PushBack( glyphInfo[index].cluster );
              mOffset.PushBack( floor( glyphPositions[index].x_offset * FROM_266 ) );
              mOffset.PushBack( floor( glyphPositions[index].y_offset * FROM_266 ) );
            }

            i += numberOfGlyphsInCluster;
          }
          else
          {
            mIndices.PushBack( glyphInfo[i].codepoint );
            mAdvance.PushBack( floor( glyphPositions[i].x_advance * FROM_266 ) );
            mCharacterMap.PushBack( glyphInfo[i].cluster );
            mOffset.PushBack( floor( glyphPositions[i].x_offset * FROM_266 ) );
            mOffset.PushBack( floor( glyphPositions[i].y_offset * FROM_266 ) );

            ++i;
          }
        }

        /* Cleanup */
        hb_buffer_destroy( harfBuzzBuffer );
        hb_font_destroy( harfBuzzFont );
        break;
      }
      case FontDescription::BITMAP_FONT:
      {
        // Reserve some space to avoid reallocations.
        // The advance and offset tables can be initialized with zeros as it's not needed to get metrics from the bitmaps here.
        mIndices.Resize( numberOfCharacters );
        mAdvance.Resize( numberOfCharacters, 0u );
        mCharacterMap.Reserve( numberOfCharacters );
        mOffset.Resize( 2u * numberOfCharacters, 0.f );

        // The utf32 character can be used as the glyph's index.
        std::copy( text, text + numberOfCharacters, mIndices.Begin() );

        // The glyph to character map is 1 to 1.
        for( unsigned int index = 0u; index < numberOfCharacters; ++index )
        {
          mCharacterMap.PushBack( index );
        }
        break;
      }
      default:
      {
        DALI_LOG_INFO(gLogFilter, Debug::General, "  Invalid type of font\n");
      }
    }

    return mIndices.Count();
  }

  void GetGlyphs( GlyphInfo* glyphInfo,
                  CharacterIndex* glyphToCharacterMap )
  {
    Vector<CharacterIndex>::ConstIterator indicesIt = mIndices.Begin();
    Vector<float>::ConstIterator advanceIt = mAdvance.Begin();
    Vector<float>::ConstIterator offsetIt = mOffset.Begin();
    Vector<CharacterIndex>::ConstIterator characterMapIt = mCharacterMap.Begin();

    for( GlyphIndex index = 0u, size = mIndices.Count(); index < size; ++index )
    {
      GlyphInfo& glyph = *( glyphInfo + index );
      CharacterIndex& glyphToCharacter = *( glyphToCharacterMap + index );

      glyph.fontId = mFontId;
      glyph.index = *( indicesIt + index );
      glyph.advance = *( advanceIt + index );

      const GlyphIndex offsetIndex = 2u * index;
      glyph.xBearing = *( offsetIt + offsetIndex );
      glyph.yBearing = *( offsetIt + offsetIndex + 1u );

      glyphToCharacter = *( characterMapIt + index );
    }
  }

  Vector<CharacterIndex> mIndices;
  Vector<float>          mAdvance;
  Vector<float>          mOffset;
  Vector<CharacterIndex> mCharacterMap;
  FontId                 mFontId;
};

Shaping::Shaping()
: mPlugin( NULL )
{
}

Shaping::~Shaping()
{
  delete mPlugin;
}

TextAbstraction::Shaping Shaping::Get()
{
  TextAbstraction::Shaping shapingHandle;

  SingletonService service( SingletonService::Get() );
  if( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( TextAbstraction::Shaping ) );
    if( handle )
    {
      // If so, downcast the handle
      Shaping* impl = dynamic_cast< Internal::Shaping* >( handle.GetObjectPtr() );
      shapingHandle = TextAbstraction::Shaping( impl );
    }
    else // create and register the object
    {
      shapingHandle = TextAbstraction::Shaping( new Shaping );
      service.Register( typeid( shapingHandle ), shapingHandle );
    }
  }

  return shapingHandle;
}

Length Shaping::Shape( const Character* const text,
                       Length numberOfCharacters,
                       FontId fontId,
                       Script script )
{
  CreatePlugin();

  return mPlugin->Shape( text,
                         numberOfCharacters,
                         fontId,
                         script );
}

void Shaping::GetGlyphs( GlyphInfo* glyphInfo,
                         CharacterIndex* glyphToCharacterMap )
{
  CreatePlugin();

  mPlugin->GetGlyphs( glyphInfo,
                      glyphToCharacterMap );
}

void Shaping::CreatePlugin()
{
  if( !mPlugin )
  {
    mPlugin = new Plugin();
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
