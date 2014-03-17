#ifndef __DALI_SLP_PLATFORM_LOADER_FONT_H__
#define __DALI_SLP_PLATFORM_LOADER_FONT_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <string>

#include <ft2build.h>
#include <dali/integration-api/glyph-set.h>
#include <dali/public-api/text/font.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

namespace Dali
{
namespace Integration
{
class GlyphSet;
}

namespace SlpPlatform
{
struct SlpFace
{
  SlpFace()
  {
    face = NULL;
    buffer = NULL;
  }

  ~SlpFace()
  {
    if (face)
    {
      FT_Done_Face(face);
      face = NULL;
    }
    if (buffer)
    {
      delete [] buffer;
      buffer = NULL;
    }
  }

  FT_Face   face;       ///< FT_Face object
  FT_Byte*  buffer;     ///< A buffer used by the face.
};

/**
 * Loads a freetype font face
 * @param [in]    fileName  Font file name
 * @param [in]    pointSize Point size for freetype source glyph scaling
 * @param [in]    dpiHor    Horizontal dpi
 * @param [in]    dpiVer    Vertical dpi
 * @param [inout] freeType  A handle to the FreeType library resource
 * @return A SlpFace pointer on success, NULL on failure
 */
SlpFace* LoadFontFace(const std::string &fileName,
                      const PointSize pointSize,
                      const unsigned int dpiHor, const unsigned int dpiVer,
                      FT_Library freeType);
/**
 * Loads a freetype font face
 * @param [in]    fileName  Font file name
 * @param [in]    pixelSize Pixel size for freetype source glyph scaling
 * @param [inout] freeType  A handle to the FreeType library resource
 * @return A SlpFace pointer on success, NULL on failure
 */
SlpFace* LoadFontFace(const std::string &fileName,
                      const PixelSize pixelSize,
                      FT_Library freeType);

/**
 * Gets a glyph (character) from a font face.
 * @param [in] face       Freetype face object
 * @param [in] charcode   Character code (unicode)
 * @param [in] load_flags How the glyph should be loaded
 * @return FT_Glyph object on success, NULL on failure
 */
FT_Glyph GetGlyph(FT_Face face, FT_ULong charcode, FT_Int32 load_flags);

/**
 * Gets a glyph (character) from a font face.
 * @param [in] face          Freetype face object
 * @param [in] charcode      UCS4 character code (UTF32)
 * @param [in] fieldSize     size (w and h) of generated distance field cells
 * @param [in] fieldPadding  number of border pixels to add around source glyph bitmap
 * @param [in] maxGlyphCell  The maximum pixel area of a single glyph (in font units)
 * @param [in] renderBitmap  whether to load/render bitmap data as well
 * @return Character pointer on success, NULL on failure
 */
Integration::GlyphSet::Character* GetCharacter(FT_Face face, const FT_ULong charcode,
                                               const unsigned int fieldSize, const unsigned int fieldPadding,
                                               const Vector2& maxGlyphCell,
                                               const bool renderBitmap, const bool highQuality );

/**
 * Retrieves a glyph's image from the given Freetype face object.
 *
 * @param[in] face Freetype face object.
 * @param[in] charCode UCS4 character code (UTF32).
 *
 * @return The glyph's image for the given character.
 */
Integration::ImageDataPtr GetGlyphBitmap( FT_Face face, FT_ULong charCode );

} // namespace SlpPlatform

} // namespace Dali

#endif // __DALI_SLP_PLATFORM_LOADER_FONT_H__
