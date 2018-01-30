/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/text/glyphy/vector-font-cache.h>

// EXTERNAL INCLUDES
#include <vector>
#include <math.h>

// INTERNAL INCLUDES
#include <dali/internal/text/glyphy/glyphy.h>
#include <dali/internal/text/glyphy/glyphy-freetype.h>

using namespace std;

namespace
{

const unsigned int INITIAL_GLYPH_CAPACITY = 50;
const double MIN_FONT_SIZE = 10;

static glyphy_bool_t
accumulate_endpoint( glyphy_arc_endpoint_t*         endpoint,
                     vector<glyphy_arc_endpoint_t>* endpoints )
{
  endpoints->push_back( *endpoint );
  return true;
}

} // unnamed namespace

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

typedef vector<VectorBlob> BlobArray;

struct VectorGlyph
{
  /**
   * @brief Create a vector-based glyph.
   */
  static VectorGlyph* New( FT_Face face,
                           FontId fontId,
                           GlyphIndex index,
                           glyphy_arc_accumulator_t* accumulator )
  {
    VectorGlyph* newGlyph = new VectorGlyph();
    newGlyph->blobData.resize( 1024 * 16 );

    if( FT_Err_Ok != FT_Load_Glyph( face,
                                    index,
                                    FT_LOAD_NO_BITMAP |
                                    FT_LOAD_NO_HINTING |
                                    FT_LOAD_NO_AUTOHINT |
                                    FT_LOAD_NO_SCALE |
                                    FT_LOAD_LINEAR_DESIGN |
                                    FT_LOAD_IGNORE_TRANSFORM))
    {
      DALI_LOG_ERROR( "FT_Load_Glyph failed\n" );
      delete newGlyph;
      return NULL;
    }

    const double upem = static_cast<double>( face->units_per_EM );
    const double tolerance = upem * 1.0f/2048.0f;

    glyphy_arc_accumulator_reset( accumulator);
    glyphy_arc_accumulator_set_tolerance( accumulator, tolerance );

    vector<glyphy_arc_endpoint_t> endpoints;
    glyphy_arc_accumulator_set_callback( accumulator,
                                         reinterpret_cast<glyphy_arc_endpoint_accumulator_callback_t>( accumulate_endpoint ),
                                         &endpoints );

    if( FT_Err_Ok != glyphy_freetype_outline_decompose( &face->glyph->outline, accumulator ) )
    {
      DALI_LOG_ERROR( "glyphy_freetype_outline_decompose failed\n" );
      delete newGlyph;
      return NULL;
    }

    DALI_ASSERT_DEBUG( glyphy_arc_accumulator_get_error(accumulator) <= tolerance && "glyphy_arc_accumulator_get_error > tolerance" );

    if( endpoints.size() )
    {
      glyphy_outline_winding_from_even_odd( &endpoints[0], endpoints.size (), false );
    }

    unsigned int blobLength( 0 );
    double averageFetchAchieved( 0.0 );
    if (!glyphy_arc_list_encode_blob( endpoints.size() ? &endpoints[0] : NULL,
                                      endpoints.size(),
                                      &newGlyph->blobData[0],
                                      newGlyph->blobData.capacity(),
                                      upem / ( MIN_FONT_SIZE * M_SQRT2 ),
                                      4,
                                      &averageFetchAchieved,
                                      &blobLength,
                                      &newGlyph->nominalWidth,
                                      &newGlyph->nominalHeight,
                                      &newGlyph->extents ) )
    {
      DALI_LOG_ERROR( "glyphy_arc_list_encode_blob failed\n" );
      delete newGlyph;
      return NULL;
    }
    newGlyph->blobData.resize( blobLength );

    glyphy_extents_scale( &newGlyph->extents, 1.0/upem, 1.0/upem );

    newGlyph->glyphInfo.fontId = fontId;
    newGlyph->glyphInfo.index  = index;

    if( glyphy_extents_is_empty( &newGlyph->extents ) )
    {
      newGlyph->glyphInfo.width  = 0.0f;
      newGlyph->glyphInfo.height = 0.0f;

      newGlyph->glyphInfo.xBearing = 0.0f;
      newGlyph->glyphInfo.yBearing = 0.0f;
    }
    else
    {
      newGlyph->glyphInfo.width  = (newGlyph->extents.max_x - newGlyph->extents.min_x);
      newGlyph->glyphInfo.height = (newGlyph->extents.max_y - newGlyph->extents.min_y);

      newGlyph->glyphInfo.xBearing = newGlyph->extents.min_x;
      newGlyph->glyphInfo.yBearing = newGlyph->glyphInfo.height + (newGlyph->extents.min_y);
    }

    newGlyph->glyphInfo.advance = face->glyph->metrics.horiAdvance / upem;
    newGlyph->glyphInfo.scaleFactor = 0.0f;

    return newGlyph;
  }

  VectorGlyph()
  : advance( 0.0 ),
    nominalWidth( 0 ),
    nominalHeight( 0 ),
    glyphInfo(),
    blobData()
  {
    glyphy_extents_clear( &extents );
  }

  glyphy_extents_t extents;
  double           advance;
  unsigned int     nominalWidth;
  unsigned int     nominalHeight;
  GlyphInfo        glyphInfo;
  BlobArray        blobData;
};

typedef vector<VectorGlyph*> GlyphCache;

struct VectorFont
{
  VectorFont( FT_Face face )
  : mFace( face ),
    mGlyphCache()
  {
    mGlyphCache.reserve( INITIAL_GLYPH_CAPACITY );
  }

  FT_Face    mFace;
  GlyphCache mGlyphCache;
};

struct VectorFontCache::Impl
{
  Impl( FT_Library freeTypeLibrary )
  : mFreeTypeLibrary( freeTypeLibrary ),
    mIdLookup(),
    mVectorFonts(),
    mAccumulator( NULL )
  {
    mAccumulator = glyphy_arc_accumulator_create();
  }

  ~Impl()
  {
    glyphy_arc_accumulator_destroy( mAccumulator );
  }

private:

  // Declared private and left undefined to avoid copies.
  Impl( const Impl& );
  // Declared private and left undefined to avoid copies.
  Impl& operator=( const Impl& );

public:

  FT_Library mFreeTypeLibrary; ///< A handle to a FreeType library instance.

  vector<string> mIdLookup;

  vector<VectorFont*> mVectorFonts;

  glyphy_arc_accumulator_t* mAccumulator;
};

VectorFontCache::VectorFontCache( FT_Library freeTypeLibrary )
: mImpl( NULL )
{
  mImpl = new Impl( freeTypeLibrary );
}

VectorFontCache::~VectorFontCache()
{
  delete mImpl;
}

FontId VectorFontCache::GetFontId( const std::string& url )
{
  FontId id( 0 );

  if( mImpl )
  {
    if( ! FindFont( url, id ) )
    {
      id = CreateFont( url );
    }
  }

  return id;
}

void VectorFontCache::GetGlyphMetrics( FontId vectorFontId, GlyphInfo& glyphInfo )
{
  if( mImpl )
  {
    if( vectorFontId > 0 &&
        vectorFontId-1 < mImpl->mVectorFonts.size() )
    {
      VectorFont* font = mImpl->mVectorFonts[ vectorFontId-1 ];
      GlyphCache& cache = font->mGlyphCache;

      bool foundGlyph( false );
      unsigned int foundIndex( 0 );
      for( unsigned int i=0; i<cache.size(); ++i )
      {
        VectorGlyph* glyph = cache[i];

        if( glyph->glyphInfo.index == glyphInfo.index )
        {
          foundIndex = i;
          foundGlyph = true;
          break;
        }
      }

      if( foundGlyph )
      {
        VectorGlyph* glyph = cache[foundIndex];
        // Note - this clobbers the original fontId, but helps avoid duplicating identical blobs
        // e.g. if when the same font family is requested in different point-sizes
        glyphInfo = glyph->glyphInfo;
      }
      else
      {
        VectorGlyph* newGlyph = VectorGlyph::New( font->mFace,
                                                  glyphInfo.fontId,
                                                  glyphInfo.index,
                                                  mImpl->mAccumulator );

        if( newGlyph )
        {
          glyphInfo = newGlyph->glyphInfo;

          cache.push_back( newGlyph );
        }
      }
    }
  }
}

void VectorFontCache::GetVectorBlob( FontId vectorFontId,
                                     FontId fontId,
                                     GlyphIndex glyphIndex,
                                     VectorBlob*& blob,
                                     unsigned int& blobLength,
                                     unsigned int& nominalWidth,
                                     unsigned int& nominalHeight )
{
  if( mImpl )
  {
    if( vectorFontId > 0 &&
        vectorFontId-1 < mImpl->mVectorFonts.size() )
    {
      VectorFont* font = mImpl->mVectorFonts[ vectorFontId-1 ];
      GlyphCache& cache = font->mGlyphCache;

      bool foundGlyph( false );
      unsigned int foundIndex( 0 );
      for( unsigned int i=0; i<cache.size(); ++i )
      {
        VectorGlyph* glyph = cache[i];

        if( glyph->glyphInfo.index == glyphIndex )
        {
          foundIndex = i;
          foundGlyph = true;
          break;
        }
      }

      if( foundGlyph )
      {
        VectorGlyph* glyph = cache[foundIndex];

        blob          = &glyph->blobData[0];
        blobLength    = glyph->blobData.size();
        nominalWidth  = glyph->nominalWidth;
        nominalHeight = glyph->nominalHeight;
      }
      else
      {
        VectorGlyph* newGlyph = VectorGlyph::New( font->mFace, fontId, glyphIndex, mImpl->mAccumulator );

        if( newGlyph )
        {
          blob          = &newGlyph->blobData[0];
          blobLength    = newGlyph->blobData.size();
          nominalWidth  = newGlyph->nominalWidth;
          nominalHeight = newGlyph->nominalHeight;

          cache.push_back( newGlyph );
        }
      }
    }
  }
}

bool VectorFontCache::FindFont( const string& url, FontId& vectorFontId ) const
{
  vectorFontId = 0u;

  const vector<string>& idLookup = mImpl->mIdLookup;

  for( unsigned int i=0; i<idLookup.size(); ++i, ++vectorFontId )
  {
    if( url == idLookup[i] )
    {
      ++vectorFontId;
      return true;
    }
  }

  return false;
}

FontId VectorFontCache::CreateFont( const string& url )
{
  FontId id( 0 );

  // Create & cache new font face
  FT_Face face;
  int error = FT_New_Face( mImpl->mFreeTypeLibrary,
                           url.c_str(),
                           0,
                           &face );

  if( FT_Err_Ok == error )
  {
    mImpl->mIdLookup.push_back( url );
    id = mImpl->mIdLookup.size();

    VectorFont* newFont = new VectorFont( face );
    mImpl->mVectorFonts.push_back( newFont );

    DALI_ASSERT_DEBUG( mImpl->mIdLookup.size() == mImpl->mVectorFonts.size() );
  }

  return id;
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
