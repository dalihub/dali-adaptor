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

// CLASS  HEADER
#include "shaping-impl.h"

// INTERNAL INCLUDES
#include <singleton-service-impl.h>
#include <dali/public-api/text-abstraction/font-client.h>
#include <dali/public-api/text-abstraction/glyph-info.h>
#include <dali/integration-api/debug.h>

// EXTERNAL INCLUDES
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include <ft2build.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

const unsigned int HIGH_QUALITY_PIXEL_SIZE = 200u;  // Pixel size sent to FreeType2 FT_Set_Char_Size() for high quality glyphs.
const char*        DEFAULT_LANGUAGE = "en";
const unsigned int DEFAULT_LANGUAGE_LENGTH = 2u;
const float        TO_PIXELS = 64.f;

const hb_script_t SCRIPT_TO_HARFBUZZ[] =
{
  HB_SCRIPT_LATIN,
  HB_SCRIPT_ARABIC,
  HB_SCRIPT_DEVANAGARI,
  HB_SCRIPT_BENGALI,
  HB_SCRIPT_GURMUKHI,
  HB_SCRIPT_GUJARATI,
  HB_SCRIPT_ORIYA,
  HB_SCRIPT_TAMIL,
  HB_SCRIPT_TELUGU,
  HB_SCRIPT_KANNADA,
  HB_SCRIPT_MALAYALAM,
  HB_SCRIPT_SINHALA,
  HB_SCRIPT_HAN,
  HB_SCRIPT_HANGUL,
  HB_SCRIPT_KHMER,
  HB_SCRIPT_LAO,
  HB_SCRIPT_THAI,
  HB_SCRIPT_MYANMAR,
  HB_SCRIPT_UNKNOWN
};

struct Shaping::Plugin
{
  Plugin()
  : mFreeTypeLibrary( NULL ),
    mIndices(),
    mAdvance(),
    mCharacterMap(),
    mFontId( 0u )
  {
  }

  ~Plugin()
  {
    FT_Done_FreeType( mFreeTypeLibrary );
  }

  void Initialize()
  {
    int error = FT_Init_FreeType( &mFreeTypeLibrary );
    if( FT_Err_Ok != error )
    {
      DALI_LOG_ERROR( "FreeType Init error: %d\n", error );
    }
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
    mFontId = fontId;

    TextAbstraction::FontClient fontClient = TextAbstraction::FontClient::Get();

    // Get the font's path file name from the font Id.
    FontDescription fontDescription;
    fontClient.GetDescription( fontId, fontDescription );

    // Create a FreeType font's face.
    FT_Face face;
    FT_Error retVal = FT_New_Face( mFreeTypeLibrary, fontDescription.path.c_str(), 0u, &face );
    if( FT_Err_Ok != retVal )
    {
      DALI_LOG_ERROR( "Failed to open face: %s\n", fontDescription.path.c_str() );
      return 0u;
    }

    FT_Set_Pixel_Sizes( face, HIGH_QUALITY_PIXEL_SIZE, HIGH_QUALITY_PIXEL_SIZE );

    /* Get our harfbuzz font struct */
    hb_font_t* harfBuzzFont;
    harfBuzzFont = hb_ft_font_create( face, NULL );

    /* Create a buffer for harfbuzz to use */
    hb_buffer_t* harfBuzzBuffer = hb_buffer_create();

    const bool rtlDiection = ( ARABIC == script );
    hb_buffer_set_direction( harfBuzzBuffer,
                             rtlDiection ? HB_DIRECTION_RTL : HB_DIRECTION_LTR ); /* or LTR */

    hb_buffer_set_script( harfBuzzBuffer,
                          SCRIPT_TO_HARFBUZZ[ script ] ); /* see hb-unicode.h */

    hb_buffer_set_language( harfBuzzBuffer,
                            hb_language_from_string( DEFAULT_LANGUAGE,
                                                     DEFAULT_LANGUAGE_LENGTH ) );

    /* Layout the text */
    hb_buffer_add_utf32( harfBuzzBuffer, text, numberOfCharacters, 0u, numberOfCharacters );

    hb_shape( harfBuzzFont, harfBuzzBuffer, NULL, 0u );

    /* Get glyph data */
    unsigned int glyphCount;
    hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos( harfBuzzBuffer, &glyphCount );
    hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions( harfBuzzBuffer, &glyphCount );

    const Length lastGlyphIndex = glyphCount - 1u;
    for( Length i = 0u; i < glyphCount; ++i )
    {
      // If the direction is right to left, Harfbuzz retrieves the glyphs in the visual order.
      // The glyphs are needed in the logical order to layout the text in lines.
      const Length index = rtlDiection ? ( lastGlyphIndex - i ) : i;

      mIndices.PushBack( glyphInfo[index].codepoint );
      mAdvance.PushBack( glyphPositions[index].x_advance / TO_PIXELS );
      mCharacterMap.PushBack( glyphInfo[index].cluster );
    }

    /* Cleanup */
    hb_buffer_destroy( harfBuzzBuffer );
    hb_font_destroy( harfBuzzFont );
    FT_Done_Face( face );

    return mIndices.Count();
  }

  void GetGlyphs( GlyphInfo* glyphInfo,
                  CharacterIndex* glyphToCharacterMap )
  {
    Vector<CharacterIndex>::ConstIterator indicesIt = mIndices.Begin();
    Vector<float>::ConstIterator advanceIt = mAdvance.Begin();
    Vector<CharacterIndex>::ConstIterator characterMapIt = mCharacterMap.Begin();

    for( Length index = 0u, size = mIndices.Count(); index < size; ++index )
    {
      GlyphInfo& glyph = *( glyphInfo + index );
      CharacterIndex& glyphToCharacter = *( glyphToCharacterMap + index );

      glyph.fontId = mFontId;
      glyph.index = *( indicesIt + index );
      glyph.advance = *( advanceIt + index );

      glyphToCharacter = *( characterMapIt + index );
    }
  }

  FT_Library             mFreeTypeLibrary;

  Vector<CharacterIndex> mIndices;
  Vector<float>          mAdvance;
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
    mPlugin->Initialize();
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
