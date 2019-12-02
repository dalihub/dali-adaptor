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

// CLASS HEADER
#include <dali/internal/text/text-abstraction/font-client-plugin-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/font-list.h>

#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/platform-abstraction.h>
#include <dali/internal/text/text-abstraction/font-client-helper.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>

// EXTERNAL INCLUDES
#include <fontconfig/fontconfig.h>

namespace
{

#if defined(DEBUG_ENABLED)
Dali::Integration::Log::Filter* gLogFilter = Dali::Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_FONT_CLIENT");
#endif

/**
 * Conversion from Fractional26.6 to float
 */
const float FROM_266 = 1.0f / 64.0f;
const float POINTS_PER_INCH = 72.f;

const std::string DEFAULT_FONT_FAMILY_NAME( "Tizen" );
const int DEFAULT_FONT_WIDTH  = 100; // normal
const int DEFAULT_FONT_WEIGHT =  80; // normal
const int DEFAULT_FONT_SLANT  =   0; // normal

const uint32_t ELLIPSIS_CHARACTER = 0x2026;

// http://www.freedesktop.org/software/fontconfig/fontconfig-user.html

// NONE            -1  --> DEFAULT_FONT_WIDTH (NORMAL) will be used.
// ULTRA_CONDENSED 50
// EXTRA_CONDENSED 63
// CONDENSED       75
// SEMI_CONDENSED  87
// NORMAL         100
// SEMI_EXPANDED  113
// EXPANDED       125
// EXTRA_EXPANDED 150
// ULTRA_EXPANDED 200
const int FONT_WIDTH_TYPE_TO_INT[] = { -1, 50, 63, 75, 87, 100, 113, 125, 150, 200 };
const unsigned int NUM_FONT_WIDTH_TYPE = sizeof( FONT_WIDTH_TYPE_TO_INT ) / sizeof( int );

// NONE                       -1  --> DEFAULT_FONT_WEIGHT (NORMAL) will be used.
// THIN                        0
// ULTRA_LIGHT, EXTRA_LIGHT   40
// LIGHT                      50
// DEMI_LIGHT, SEMI_LIGHT     55
// BOOK                       75
// NORMAL, REGULAR            80
// MEDIUM                    100
// DEMI_BOLD, SEMI_BOLD      180
// BOLD                      200
// ULTRA_BOLD, EXTRA_BOLD    205
// BLACK, HEAVY, EXTRA_BLACK 210
const int FONT_WEIGHT_TYPE_TO_INT[] = { -1, 0, 40, 50, 55, 75, 80, 100, 180, 200, 205, 210 };
const unsigned int NUM_FONT_WEIGHT_TYPE = sizeof( FONT_WEIGHT_TYPE_TO_INT ) / sizeof( int );

// NONE             -1 --> DEFAULT_FONT_SLANT (NORMAL) will be used.
// NORMAL, ROMAN     0
// ITALIC          100
// OBLIQUE         110
const int FONT_SLANT_TYPE_TO_INT[] = { -1, 0, 100, 110 };
const unsigned int NUM_FONT_SLANT_TYPE = sizeof( FONT_SLANT_TYPE_TO_INT ) / sizeof( int );

} // namespace

using Dali::Vector;
using namespace std;

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

/**
 * @brief Returns the FontWidth's enum index for the given width value.
 *
 * @param[in] width The width value.
 *
 * @return The FontWidth's enum index.
 */
FontWidth::Type IntToWidthType( int width )
{
  return static_cast<FontWidth::Type>( ValueToIndex( width, FONT_WIDTH_TYPE_TO_INT, NUM_FONT_WIDTH_TYPE - 1u ) );
}

/**
 * @brief Returns the FontWeight's enum index for the given weight value.
 *
 * @param[in] weight The weight value.
 *
 * @return The FontWeight's enum index.
 */
FontWeight::Type IntToWeightType( int weight )
{
  return static_cast<FontWeight::Type>( ValueToIndex( weight, FONT_WEIGHT_TYPE_TO_INT, NUM_FONT_WEIGHT_TYPE - 1u ) );
}

/**
 * @brief Returns the FontSlant's enum index for the given slant value.
 *
 * @param[in] slant The slant value.
 *
 * @return The FontSlant's enum index.
 */
FontSlant::Type IntToSlantType( int slant )
{
  return static_cast<FontSlant::Type>( ValueToIndex( slant, FONT_SLANT_TYPE_TO_INT, NUM_FONT_SLANT_TYPE - 1u ) );
}

/**
 * @brief Free the resources allocated by the FcCharSet objects.
 *
 * @param[in] characterSets The vector of character sets.
 */
void DestroyCharacterSets( CharacterSetList& characterSets )
{
  for( auto& item : characterSets )
  {
    if( item )
    {
      FcCharSetDestroy( item );
    }
  }
}

FontClient::Plugin::FallbackCacheItem::FallbackCacheItem( FontDescription&& font, FontList* fallbackFonts, CharacterSetList* characterSets )
: fontDescription{ std::move( font ) },
  fallbackFonts{ fallbackFonts },
  characterSets{ characterSets }
{
}

FontClient::Plugin::FontDescriptionCacheItem::FontDescriptionCacheItem( const FontDescription& fontDescription,
                                                                        FontDescriptionId index )
: fontDescription{ fontDescription },
  index{ index }
{
}

FontClient::Plugin::FontDescriptionCacheItem::FontDescriptionCacheItem( FontDescription&& fontDescription,
                                                                        FontDescriptionId index )
: fontDescription{ std::move( fontDescription ) },
  index{ index }
{
}

FontClient::Plugin::FontDescriptionSizeCacheItem::FontDescriptionSizeCacheItem( FontDescriptionId validatedFontId,
                                                                                PointSize26Dot6 requestedPointSize,
                                                                                FontId fontId )
: validatedFontId( validatedFontId ),
  requestedPointSize( requestedPointSize ),
  fontId( fontId )
{
}

FontClient::Plugin::FontFaceCacheItem::FontFaceCacheItem( FT_Face ftFace,
                                                          const FontPath& path,
                                                          PointSize26Dot6 requestedPointSize,
                                                          FaceIndex face,
                                                          const FontMetrics& metrics )
: mFreeTypeFace( ftFace ),
  mPath( path ),
  mRequestedPointSize( requestedPointSize ),
  mFaceIndex( face ),
  mMetrics( metrics ),
  mCharacterSet( nullptr ),
  mFixedSizeIndex( 0 ),
  mFixedWidthPixels( 0.f ),
  mFixedHeightPixels( 0.f ),
  mVectorFontId( 0u ),
  mFontId( 0u ),
  mIsFixedSizeBitmap( false ),
  mHasColorTables( false )
{
}

FontClient::Plugin::FontFaceCacheItem::FontFaceCacheItem( FT_Face ftFace,
                                                          const FontPath& path,
                                                          PointSize26Dot6 requestedPointSize,
                                                          FaceIndex face,
                                                          const FontMetrics& metrics,
                                                          int fixedSizeIndex,
                                                          float fixedWidth,
                                                          float fixedHeight,
                                                          bool hasColorTables )
: mFreeTypeFace( ftFace ),
  mPath( path ),
  mRequestedPointSize( requestedPointSize ),
  mFaceIndex( face ),
  mMetrics( metrics ),
  mCharacterSet( nullptr ),
  mFixedSizeIndex( fixedSizeIndex ),
  mFixedWidthPixels( fixedWidth ),
  mFixedHeightPixels( fixedHeight ),
  mVectorFontId( 0u ),
  mFontId( 0u ),
  mIsFixedSizeBitmap( true ),
  mHasColorTables( hasColorTables )
{
}

FontClient::Plugin::Plugin( unsigned int horizontalDpi,
                            unsigned int verticalDpi )
: mFreeTypeLibrary( nullptr ),
  mDpiHorizontal( horizontalDpi ),
  mDpiVertical( verticalDpi ),
  mDefaultFontDescription(),
  mSystemFonts(),
  mDefaultFonts(),
  mFontIdCache(),
  mFontFaceCache(),
  mValidatedFontCache(),
  mFontDescriptionCache(),
  mCharacterSetCache(),
  mFontDescriptionSizeCache(),
  mVectorFontCache( nullptr ),
  mEllipsisCache(),
  mEmbeddedItemCache(),
  mDefaultFontDescriptionCached( false )
{
  int error = FT_Init_FreeType( &mFreeTypeLibrary );
  if( FT_Err_Ok != error )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "FreeType Init error: %d\n", error );
  }

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  mVectorFontCache = new VectorFontCache( mFreeTypeLibrary );
#endif
}

FontClient::Plugin::~Plugin()
{
  ClearFallbackCache( mFallbackCache );

  // Free the resources allocated by the FcCharSet objects.
  DestroyCharacterSets( mDefaultFontCharacterSets );
  DestroyCharacterSets( mCharacterSetCache );
  ClearCharacterSetFromFontFaceCache();

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  delete mVectorFontCache;
#endif
  FT_Done_FreeType( mFreeTypeLibrary );
}

void FontClient::Plugin::ClearCache()
{
  mDefaultFontDescription = FontDescription();

  mSystemFonts.clear();
  mDefaultFonts.clear();

  DestroyCharacterSets( mDefaultFontCharacterSets );
  mDefaultFontCharacterSets.Clear();

  ClearFallbackCache( mFallbackCache );
  mFallbackCache.clear();

  mFontIdCache.Clear();

  ClearCharacterSetFromFontFaceCache();
  mFontFaceCache.clear();

  mValidatedFontCache.clear();
  mFontDescriptionCache.clear();

  DestroyCharacterSets( mCharacterSetCache );
  mCharacterSetCache.Clear();

  mFontDescriptionSizeCache.clear();

  mEllipsisCache.Clear();
  mPixelBufferCache.clear();
  mEmbeddedItemCache.Clear();
  mBitmapFontCache.clear();

  mDefaultFontDescriptionCached = false;
}

void FontClient::Plugin::SetDpi( unsigned int horizontalDpi,
                                 unsigned int verticalDpi )
{
  mDpiHorizontal = horizontalDpi;
  mDpiVertical = verticalDpi;
}

void FontClient::Plugin::ResetSystemDefaults()
{
  mDefaultFontDescriptionCached = false;
}

void FontClient::Plugin::SetFontList( const FontDescription& fontDescription, FontList& fontList, CharacterSetList& characterSetList )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::SetFontList\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  description; family : [%s]\n", fontDescription.family.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[fontDescription.width] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[fontDescription.weight] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[fontDescription.slant] );

  fontList.clear();

  FcPattern* fontFamilyPattern = CreateFontFamilyPattern( fontDescription ); // Creates a pattern that needs to be destroyed by calling FcPatternDestroy.

  FcResult result = FcResultMatch;

  // Match the pattern.
  FcFontSet* fontSet = FcFontSort( nullptr /* use default configure */,
                                   fontFamilyPattern,
                                   false /* don't trim */,
                                   nullptr,
                                   &result ); // FcFontSort creates a font set that needs to be destroyed by calling FcFontSetDestroy.

  if( nullptr != fontSet )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "  number of fonts found : [%d]\n", fontSet->nfont );
    // Reserve some space to avoid reallocations.
    fontList.reserve( fontSet->nfont );

    for( int i = 0u; i < fontSet->nfont; ++i )
    {
      FcPattern* fontPattern = fontSet->fonts[i];

      FontPath path;

      // Skip fonts with no path
      if( GetFcString( fontPattern, FC_FILE, path ) )
      {
        // Retrieve the character set. Need to call FcCharSetDestroy to free the resources.
        FcCharSet* characterSet = nullptr;
        FcPatternGetCharSet( fontPattern, FC_CHARSET, 0u, &characterSet );

        // Increase the reference counter of the character set.
        characterSetList.PushBack( FcCharSetCopy( characterSet ) );

        fontList.push_back( FontDescription() );
        FontDescription& newFontDescription = fontList.back();

        newFontDescription.path = std::move( path );

        int width = 0;
        int weight = 0;
        int slant = 0;
        GetFcString( fontPattern, FC_FAMILY, newFontDescription.family );
        GetFcInt( fontPattern, FC_WIDTH, width );
        GetFcInt( fontPattern, FC_WEIGHT, weight );
        GetFcInt( fontPattern, FC_SLANT, slant );
        newFontDescription.width = IntToWidthType( width );
        newFontDescription.weight = IntToWeightType( weight );
        newFontDescription.slant = IntToSlantType( slant );

        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  description; family : [%s]\n", newFontDescription.family.c_str() );
        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", newFontDescription.path.c_str() );
        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[newFontDescription.width] );
        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[newFontDescription.weight] );
        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[newFontDescription.slant] );
      }
    }

    // Destroys the font set created by FcFontSort.
    FcFontSetDestroy( fontSet );
  }
  else
  {
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  No fonts found.\n" );
  }

  // Destroys the pattern created by FcPatternCreate in CreateFontFamilyPattern.
  FcPatternDestroy( fontFamilyPattern );

  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::SetFontList\n" );
}

void FontClient::Plugin::GetDefaultFonts( FontList& defaultFonts )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::GetDefaultFonts\n" );

  if( mDefaultFonts.empty() )
  {
    FontDescription fontDescription;
    fontDescription.family = DEFAULT_FONT_FAMILY_NAME;  // todo This could be set to the Platform font
    fontDescription.width = IntToWidthType( DEFAULT_FONT_WIDTH );
    fontDescription.weight = IntToWeightType( DEFAULT_FONT_WEIGHT );
    fontDescription.slant = IntToSlantType( DEFAULT_FONT_SLANT );
    SetFontList( fontDescription, mDefaultFonts, mDefaultFontCharacterSets );
  }

  defaultFonts = mDefaultFonts;

  DALI_LOG_INFO( gLogFilter, Debug::General, "  number of default fonts : [%d]\n", mDefaultFonts.size() );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetDefaultFonts\n" );
}

void FontClient::Plugin::GetDefaultPlatformFontDescription( FontDescription& fontDescription )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::GetDefaultPlatformFontDescription\n");

  if( !mDefaultFontDescriptionCached )
  {
    // Clear any font config stored info in the caches.

    // Decrease the reference counter and eventually free the resources allocated by FcCharSet objects.
    DestroyCharacterSets( mDefaultFontCharacterSets );
    DestroyCharacterSets( mCharacterSetCache );
    mDefaultFontCharacterSets.Clear();
    mCharacterSetCache.Clear();

    for( auto& item : mFallbackCache )
    {
      // Decrease the reference counter and eventually free the resources allocated by FcCharSet objects.
      DestroyCharacterSets( *item.characterSets );

      delete item.characterSets;
      item.characterSets = nullptr;
    }

    // Set the character set pointer as null. Will be created again the next time IsCharacterSupportedByFont()
    ClearCharacterSetFromFontFaceCache();

    // FcInitBringUptoDate did not seem to reload config file as was still getting old default font.
    FcInitReinitialize();

    FcPattern* matchPattern = FcPatternCreate(); // Creates a pattern that needs to be destroyed by calling FcPatternDestroy.

    if( nullptr != matchPattern )
    {
      FcConfigSubstitute( nullptr, matchPattern, FcMatchPattern );
      FcDefaultSubstitute( matchPattern );

      FcCharSet* characterSet = nullptr;
      MatchFontDescriptionToPattern( matchPattern, mDefaultFontDescription, &characterSet );
      // Decrease the reference counter of the character set as it's not stored.
      FcCharSetDestroy( characterSet );

      // Destroys the pattern created.
      FcPatternDestroy( matchPattern );
    }

    // Create again the character sets as they are not valid after FcInitReinitialize()

    for( const auto& description : mDefaultFonts )
    {
      mDefaultFontCharacterSets.PushBack( FcCharSetCopy( CreateCharacterSetFromDescription( description ) ) );
    }

    for( const auto& description : mFontDescriptionCache )
    {
      mCharacterSetCache.PushBack( FcCharSetCopy( CreateCharacterSetFromDescription( description ) ) );
    }

    for( auto& item : mFallbackCache )
    {
      if( nullptr != item.fallbackFonts )
      {
        if( nullptr == item.characterSets )
        {
          item.characterSets = new CharacterSetList;
        }

        for( const auto& description : *( item.fallbackFonts ) )
        {
          item.characterSets->PushBack( FcCharSetCopy( CreateCharacterSetFromDescription( description ) ) );
        }
      }
    }

    mDefaultFontDescriptionCached = true;
  }

  fontDescription.path   = mDefaultFontDescription.path;
  fontDescription.family = mDefaultFontDescription.family;
  fontDescription.width  = mDefaultFontDescription.width;
  fontDescription.weight = mDefaultFontDescription.weight;
  fontDescription.slant  = mDefaultFontDescription.slant;

  DALI_LOG_INFO( gLogFilter, Debug::General, "  description; family : [%s]\n", fontDescription.family.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", fontDescription.path.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[fontDescription.width] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[fontDescription.weight] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[fontDescription.slant] );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetDefaultPlatformFontDescription\n");
}

void FontClient::Plugin::GetSystemFonts( FontList& systemFonts )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::GetSystemFonts\n");

  if( mSystemFonts.empty() )
  {
    InitSystemFonts();
  }

  systemFonts = mSystemFonts;
  DALI_LOG_INFO( gLogFilter, Debug::General, "  number of system fonts : [%d]\n", mSystemFonts.size() );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetSystemFonts\n");
}

void FontClient::Plugin::GetDescription( FontId id,
                                         FontDescription& fontDescription ) const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::GetDescription\n");
  DALI_LOG_INFO( gLogFilter, Debug::General, "  font id : %d\n", id );
  const FontId index = id - 1u;

  if( ( id > 0u ) && ( index < mFontIdCache.Count() ) )
  {
    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];
    switch( fontIdCacheItem.type )
    {
      case FontDescription::FACE_FONT:
      {
        for( const auto& item : mFontDescriptionSizeCache )
        {
          if( item.fontId == fontIdCacheItem.id )
          {
            fontDescription = *( mFontDescriptionCache.begin() + item.validatedFontId - 1u );

            DALI_LOG_INFO( gLogFilter, Debug::General, "  description; family : [%s]\n", fontDescription.family.c_str() );
            DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", fontDescription.path.c_str() );
            DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[fontDescription.width] );
            DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[fontDescription.weight] );
            DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[fontDescription.slant] );
            DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetDescription\n");
            return;
          }
        }
        break;
      }
      case FontDescription::BITMAP_FONT:
      {
        fontDescription.type = FontDescription::BITMAP_FONT;
        fontDescription.family = mBitmapFontCache[fontIdCacheItem.id].font.name;
        break;
      }
      default:
      {
        DALI_LOG_INFO(gLogFilter, Debug::General, "  Invalid type of font\n");
        fontDescription.type = FontDescription::INVALID;
        fontDescription.family.clear();
      }
    }
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  No description found for the font ID %d\n", id );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetDescription\n");
}

PointSize26Dot6 FontClient::Plugin::GetPointSize( FontId id )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::GetPointSize\n");
  DALI_LOG_INFO( gLogFilter, Debug::General, "  font id : %d\n", id );
  const FontId index = id - 1u;

  if( ( id > 0u ) &&
      ( index < mFontIdCache.Count() ) )
  {
    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];

    switch( fontIdCacheItem.type )
    {
      case FontDescription::FACE_FONT:
      {
        DALI_LOG_INFO( gLogFilter, Debug::General, "  point size : %d\n", ( *( mFontFaceCache.begin() + fontIdCacheItem.id ) ).mRequestedPointSize );
        DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetPointSize\n");
        return ( *( mFontFaceCache.begin() + fontIdCacheItem.id ) ).mRequestedPointSize;
      }
      case FontDescription::BITMAP_FONT:
      {
        return TextAbstraction::FontClient::DEFAULT_POINT_SIZE;
      }
      default:
      {
        DALI_LOG_INFO(gLogFilter, Debug::General, "  Invalid type of font\n");
      }
    }
  }
  else
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "  Invalid font ID %d\n", id );
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  default point size : %d\n", TextAbstraction::FontClient::DEFAULT_POINT_SIZE );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetPointSize\n");
  return TextAbstraction::FontClient::DEFAULT_POINT_SIZE;
}

bool FontClient::Plugin::IsCharacterSupportedByFont( FontId fontId, Character character )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::IsCharacterSupportedByFont\n");
  DALI_LOG_INFO( gLogFilter, Debug::General, "    font id : %d\n", fontId );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  character : %p\n", character );

  if( ( fontId < 1u ) || ( fontId > mFontIdCache.Count() ) )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "  Invalid font id. Number of items in the cache: %d\n",mFontIdCache.Count());
    DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::IsCharacterSupportedByFont\n");
    return false;
  }

  --fontId;

  bool isSupported = false;

  const FontIdCacheItem& fontIdCacheItem = mFontIdCache[fontId];

  switch( fontIdCacheItem.type )
  {
    case FontDescription::FACE_FONT:
    {
      if( fontIdCacheItem.id < mFontFaceCache.size() )
      {
        FontFaceCacheItem& cacheItem = mFontFaceCache[fontIdCacheItem.id];

        if( nullptr == cacheItem.mCharacterSet )
        {
          // Create again the character set.
          // It can be null if the ResetSystemDefaults() method has been called.

          FontDescription description;
          description.path = cacheItem.mPath;
          description.family = std::move( FontFamily( cacheItem.mFreeTypeFace->family_name ) );
          description.weight = FontWeight::NONE;
          description.width = FontWidth::NONE;
          description.slant = FontSlant::NONE;

          // Note FreeType doesn't give too much info to build a proper font style.
          if( cacheItem.mFreeTypeFace->style_flags & FT_STYLE_FLAG_ITALIC )
          {
            description.slant = FontSlant::ITALIC;
          }
          if( cacheItem.mFreeTypeFace->style_flags & FT_STYLE_FLAG_BOLD )
          {
            description.weight = FontWeight::BOLD;
          }

          cacheItem.mCharacterSet = FcCharSetCopy( CreateCharacterSetFromDescription( description ) );
        }

        isSupported = FcCharSetHasChar( cacheItem.mCharacterSet, character );
      }
      break;
    }
    case FontDescription::BITMAP_FONT:
    {
      const BitmapFont& bitmapFont = mBitmapFontCache[fontIdCacheItem.id].font;

      for( const auto& glyph : bitmapFont.glyphs )
      {
        if( glyph.utf32 == character )
        {
          isSupported = true;
          break;
        }
      }
      break;
    }
    default:
    {
      DALI_LOG_INFO(gLogFilter, Debug::General, "  Invalid type of font\n");
    }
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  is supported : %s\n", (isSupported ? "true" : "false") );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::IsCharacterSupportedByFont\n");
  return isSupported;
}

FontId FontClient::Plugin::FindFontForCharacter( const FontList& fontList,
                                                 const CharacterSetList& characterSetList,
                                                 Character character,
                                                 PointSize26Dot6 requestedPointSize,
                                                 bool preferColor )
{
  DALI_ASSERT_DEBUG( ( fontList.size() == characterSetList.Count() ) && "FontClient::Plugin::FindFontForCharacter. Different number of fonts and character sets." );

  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::FindFontForCharacter\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "           character : %p\n", character );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize );
  DALI_LOG_INFO( gLogFilter, Debug::General, "         preferColor : %s\n", ( preferColor ? "true" : "false" ) );

  FontId fontId = 0u;
  bool foundColor = false;

  DALI_LOG_INFO( gLogFilter, Debug::General, "  number of fonts : %d\n", fontList.size() );

  // Traverse the list of fonts.
  // Check for each font if supports the character.
  for( unsigned int index = 0u, numberOfFonts = fontList.size(); index < numberOfFonts; ++index )
  {
    const FontDescription& description = fontList[index];
    const FcCharSet* const characterSet = characterSetList[index];

    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  description; family : [%s]\n", description.family.c_str() );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", description.path.c_str() );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[description.width] );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[description.weight] );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[description.slant] );

    bool foundInRanges = false;
    if( nullptr != characterSet )
    {
      foundInRanges = FcCharSetHasChar( characterSet, character );
    }

    if( foundInRanges )
    {
      fontId = GetFontId( description,
                          requestedPointSize,
                          0u );

      DALI_LOG_INFO( gLogFilter, Debug::Verbose, "     font id : %d\n", fontId );

      if( preferColor )
      {
        if( ( fontId > 0 ) &&
            ( fontId - 1u < mFontIdCache.Count() ) )
        {
          const FontFaceCacheItem& item = mFontFaceCache[mFontIdCache[fontId - 1u].id];

          foundColor = item.mHasColorTables;
        }

        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  foundColor : %s\n", ( foundColor ? "true" : "false" ) );
      }

      // Keep going unless we prefer a different (color) font.
      if( !preferColor || foundColor )
      {
        break;
      }
    }
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  font id : %d\n", fontId );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindFontForCharacter\n" );
  return fontId;
}

FontId FontClient::Plugin::FindDefaultFont( Character charcode,
                                            PointSize26Dot6 requestedPointSize,
                                            bool preferColor )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::FindDefaultFont\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "           character : %p\n", charcode );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize );
  DALI_LOG_INFO( gLogFilter, Debug::General, "         preferColor : %s\n", ( preferColor ? "true" : "false" ) );

  FontId fontId(0);

  // Create the list of default fonts if it has not been created.
  if( mDefaultFonts.empty() )
  {
    FontDescription fontDescription;
    fontDescription.family = DEFAULT_FONT_FAMILY_NAME;
    fontDescription.width = IntToWidthType( DEFAULT_FONT_WIDTH );
    fontDescription.weight = IntToWeightType( DEFAULT_FONT_WEIGHT );
    fontDescription.slant = IntToSlantType( DEFAULT_FONT_SLANT );

    SetFontList( fontDescription, mDefaultFonts, mDefaultFontCharacterSets );
  }
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  number of default fonts : %d\n", mDefaultFonts.size() );


  // Traverse the list of default fonts.
  // Check for each default font if supports the character.
  fontId = FindFontForCharacter( mDefaultFonts, mDefaultFontCharacterSets, charcode, requestedPointSize, preferColor );

  DALI_LOG_INFO( gLogFilter, Debug::General, "  font id : %d\n", fontId );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindDefaultFont\n" );

  return fontId;
}

FontId FontClient::Plugin::FindFallbackFont( Character charcode,
                                             const FontDescription& preferredFontDescription,
                                             PointSize26Dot6 requestedPointSize,
                                             bool preferColor )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::FindFallbackFont\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "           character : %p\n", charcode );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize );
  DALI_LOG_INFO( gLogFilter, Debug::General, "         preferColor : %s\n", ( preferColor ? "true" : "false" ) );

  // The font id to be returned.
  FontId fontId = 0u;

  FontDescription fontDescription;

  // Fill the font description with the preferred font description and complete with the defaults.
  fontDescription.family = preferredFontDescription.family.empty() ? DEFAULT_FONT_FAMILY_NAME : preferredFontDescription.family;
  fontDescription.weight = ( ( FontWeight::NONE == preferredFontDescription.weight ) ? IntToWeightType( DEFAULT_FONT_WEIGHT ) : preferredFontDescription.weight );
  fontDescription.width = ( ( FontWidth::NONE == preferredFontDescription.width ) ? IntToWidthType( DEFAULT_FONT_WIDTH ) : preferredFontDescription.width );
  fontDescription.slant = ( ( FontSlant::NONE == preferredFontDescription.slant ) ? IntToSlantType( DEFAULT_FONT_SLANT ) : preferredFontDescription.slant );

  DALI_LOG_INFO( gLogFilter, Debug::General, "  preferredFontDescription --> fontDescription\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  [%s] --> [%s]\n", preferredFontDescription.family.c_str(), fontDescription.family.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  [%s] --> [%s]\n", FontWeight::Name[preferredFontDescription.weight], FontWeight::Name[fontDescription.weight] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  [%s] --> [%s]\n", FontWidth::Name[preferredFontDescription.width], FontWidth::Name[fontDescription.width] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  [%s] --> [%s]\n", FontSlant::Name[preferredFontDescription.slant], FontSlant::Name[fontDescription.slant] );

  // Check first if the font's description has been queried before.
  FontList* fontList = nullptr;
  CharacterSetList* characterSetList = nullptr;

  if( !FindFallbackFontList( fontDescription, fontList, characterSetList ) )
  {
    fontList = new FontList;
    characterSetList = new CharacterSetList;

    SetFontList( fontDescription, *fontList, *characterSetList );

    // Add the font-list to the cache.
    mFallbackCache.push_back( std::move( FallbackCacheItem( std::move( fontDescription ), fontList, characterSetList ) ) );
  }

  if( fontList && characterSetList )
  {
    fontId = FindFontForCharacter( *fontList, *characterSetList, charcode, requestedPointSize, preferColor );
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  font id : %d\n", fontId );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindFallbackFont\n");
  return fontId;
}

FontId FontClient::Plugin::GetFontId( const FontPath& path,
                                      PointSize26Dot6 requestedPointSize,
                                      FaceIndex faceIndex,
                                      bool cacheDescription )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::GetFontId\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "                path : [%s]\n", path.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize );

  FontId id = 0u;

  if( nullptr != mFreeTypeLibrary )
  {
    FontId foundId = 0u;
    if( FindFont( path, requestedPointSize, faceIndex, foundId ) )
    {
      id = foundId;
    }
    else
    {
      id = CreateFont( path, requestedPointSize, faceIndex, cacheDescription );
    }
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  font id : %d\n", id );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetFontId\n" );

  return id;
}

FontId FontClient::Plugin::GetFontId( const FontDescription& fontDescription,
                                      PointSize26Dot6 requestedPointSize,
                                      FaceIndex faceIndex )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::GetFontId\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  description; family : [%s]\n", fontDescription.family.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", fontDescription.path.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[fontDescription.width] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[fontDescription.weight] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[fontDescription.slant] );
  DALI_LOG_INFO( gLogFilter, Debug::General, "   requestedPointSize : %d\n", requestedPointSize );

  // This method uses three vectors which caches:
  // * The bitmap font cache
  // * Pairs of non validated font descriptions and an index to a vector with paths to font file names.
  // * The path to font file names.
  // * The font ids of pairs 'font point size, index to the vector with paths to font file names'.

  // 1) Checks if the font description matches with a previously loaded bitmap font.
  //    Returns if a font is found.
  // 2) Checks in the cache if the font's description has been validated before.
  //    If it was it gets an index to the vector with paths to font file names. Otherwise,
  //    retrieves using font config a path to a font file name which matches with the
  //    font's description. The path is stored in the cache.
  //
  // 3) Checks in the cache if the pair 'font point size, index to the vector with paths to
  //    font file names' exists. If exists, it gets the font id. If it doesn't it calls
  //    the GetFontId() method with the path to the font file name and the point size to
  //    get the font id.

  // The font id to be returned.
  FontId fontId = 0u;

  // Check first if the font description matches with a previously loaded bitmap font.
  if( FindBitmapFont( fontDescription.family, fontId ) )
  {
    return fontId;
  }

  // Check if the font's description have been validated before.
  FontDescriptionId validatedFontId = 0u;

  if( !FindValidatedFont( fontDescription,
                          validatedFontId ) )
  {
    // Use font config to validate the font's description.
    ValidateFont( fontDescription,
                  validatedFontId );
  }

  FontId fontFaceId = 0u;
  // Check if exists a pair 'validatedFontId, requestedPointSize' in the cache.
  if( !FindFont( validatedFontId, requestedPointSize, fontFaceId ) )
  {
    // Retrieve the font file name path.
    const FontDescription& description = *( mFontDescriptionCache.begin() + validatedFontId - 1u );

    // Retrieve the font id. Do not cache the description as it has been already cached.
    fontId = GetFontId( description.path,
                        requestedPointSize,
                        faceIndex,
                        false );

    fontFaceId = mFontIdCache[fontId-1u].id;
    mFontFaceCache[fontFaceId].mCharacterSet = FcCharSetCopy( mCharacterSetCache[validatedFontId - 1u] );

    // Cache the pair 'validatedFontId, requestedPointSize' to improve the following queries.
    mFontDescriptionSizeCache.push_back( FontDescriptionSizeCacheItem( validatedFontId,
                                                                       requestedPointSize,
                                                                       fontFaceId ) );
  }
  else
  {
    fontId = mFontFaceCache[fontFaceId].mFontId + 1u;
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  font id : %d\n", fontId );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetFontId\n" );

  return fontId;
}

FontId FontClient::Plugin::GetFontId( const BitmapFont& bitmapFont )
{
  for( const auto& item : mBitmapFontCache )
  {
    if( bitmapFont.name == item.font.name )
    {
      return item.id + 1u;
    }
  }

  BitmapFontCacheItem bitmapFontCacheItem;
  bitmapFontCacheItem.font = bitmapFont;
  bitmapFontCacheItem.id = mFontIdCache.Count();

  // Resize the vector with the pixel buffers.
  bitmapFontCacheItem.pixelBuffers.resize( bitmapFont.glyphs.size() );

  // Traverse all the glyphs and load the pixel buffer of those with ascender and descender equal to zero.
  unsigned int index = 0u;
  for( auto& glyph : bitmapFontCacheItem.font.glyphs )
  {
    Devel::PixelBuffer& pixelBuffer = bitmapFontCacheItem.pixelBuffers[index];

    if( EqualsZero( glyph.ascender ) && EqualsZero( glyph.descender ) )
    {
      // Load the glyph.
      pixelBuffer = LoadImageFromFile( glyph.url );

      if( pixelBuffer )
      {
        glyph.ascender = static_cast<float>(pixelBuffer.GetHeight());
      }
    }

    bitmapFontCacheItem.font.ascender = std::max( glyph.ascender, bitmapFontCacheItem.font.ascender );
    bitmapFontCacheItem.font.descender = std::min( glyph.descender, bitmapFontCacheItem.font.descender );

    ++index;
  }

  FontIdCacheItem fontIdCacheItem;
  fontIdCacheItem.type = FontDescription::BITMAP_FONT;
  fontIdCacheItem.id = mBitmapFontCache.size();

  mBitmapFontCache.push_back( std::move( bitmapFontCacheItem ) );
  mFontIdCache.PushBack( fontIdCacheItem );

  return bitmapFontCacheItem.id + 1u;
}

void FontClient::Plugin::ValidateFont( const FontDescription& fontDescription,
                                       FontDescriptionId& validatedFontId )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::ValidateFont\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  description; family : [%s]\n", fontDescription.family.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", fontDescription.path.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[fontDescription.width] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[fontDescription.weight] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[fontDescription.slant] );

  // Create a font pattern.
  FcPattern* fontFamilyPattern = CreateFontFamilyPattern( fontDescription );

  FontDescription description;

  FcCharSet* characterSet = nullptr;
  bool matched = MatchFontDescriptionToPattern( fontFamilyPattern, description, &characterSet );
  FcPatternDestroy( fontFamilyPattern );

  if( matched && ( nullptr != characterSet ) )
  {
    // Add the path to the cache.
    description.type = FontDescription::FACE_FONT;
    mFontDescriptionCache.push_back( description );

    // Set the index to the vector of paths to font file names.
    validatedFontId = mFontDescriptionCache.size();

    DALI_LOG_INFO( gLogFilter, Debug::General, "  matched description; family : [%s]\n", description.family.c_str() );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                         path : [%s]\n", description.path.c_str() );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                        width : [%s]\n", FontWidth::Name[description.width] );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                       weight : [%s]\n", FontWeight::Name[description.weight] );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                        slant : [%s]\n\n", FontSlant::Name[description.slant] );
    DALI_LOG_INFO( gLogFilter, Debug::General, "  validatedFontId : %d\n", validatedFontId );

    // The reference counter of the character set has already been increased in MatchFontDescriptionToPattern.
    mCharacterSetCache.PushBack( characterSet );

    // Cache the index and the matched font's description.
    FontDescriptionCacheItem item( description,
                                   validatedFontId );

    mValidatedFontCache.push_back( std::move( item ) );

    if( ( fontDescription.family != description.family ) ||
        ( fontDescription.width != description.width )   ||
        ( fontDescription.weight != description.weight ) ||
        ( fontDescription.slant != description.slant ) )
    {
      // Cache the given font's description if it's different than the matched.
      FontDescriptionCacheItem item( fontDescription,
                                     validatedFontId );

      mValidatedFontCache.push_back( std::move( item ) );
    }
  }
  else
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "  font validation failed for font [%s]\n", fontDescription.family.c_str() );
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::ValidateFont\n" );
}

void FontClient::Plugin::GetFontMetrics( FontId fontId,
                                         FontMetrics& metrics )
{
  const FontId index = fontId - 1u;

  if( ( fontId > 0 ) &&
      ( index < mFontIdCache.Count() ) )
  {
    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];

    switch( fontIdCacheItem.type )
    {
      case FontDescription::FACE_FONT:
      {
        const FontFaceCacheItem& font = mFontFaceCache[fontIdCacheItem.id];

        metrics = font.mMetrics;

        // Adjust the metrics if the fixed-size font should be down-scaled
        if( font.mIsFixedSizeBitmap )
        {
          const float desiredFixedSize =  static_cast<float>( font.mRequestedPointSize ) * FROM_266 / POINTS_PER_INCH * mDpiVertical;

          if( desiredFixedSize > 0.f )
          {
            const float scaleFactor = desiredFixedSize / font.mFixedHeightPixels;

            metrics.ascender = metrics.ascender * scaleFactor;
            metrics.descender = metrics.descender * scaleFactor;
            metrics.height = metrics.height * scaleFactor;
            metrics.underlinePosition = metrics.underlinePosition * scaleFactor;
            metrics.underlineThickness = metrics.underlineThickness * scaleFactor;
          }
        }
        break;
      }
      case FontDescription::BITMAP_FONT:
      {
        const BitmapFontCacheItem& bitmapFontCacheItem = mBitmapFontCache[fontIdCacheItem.id];

        metrics.ascender = bitmapFontCacheItem.font.ascender;
        metrics.descender = bitmapFontCacheItem.font.descender;
        metrics.height = metrics.ascender - metrics.descender;
        metrics.underlinePosition = bitmapFontCacheItem.font.underlinePosition;
        metrics.underlineThickness = bitmapFontCacheItem.font.underlineThickness;
        break;
      }
      default:
      {
        DALI_LOG_INFO(gLogFilter, Debug::General, "  Invalid type of font\n");
      }
    }
  }
  else
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::GetFontMetrics. Invalid font id : %d\n", fontId );
  }
}

GlyphIndex FontClient::Plugin::GetGlyphIndex( FontId fontId,
                                              Character charcode )
{
  GlyphIndex glyphIndex = 0u;
  const FontId index = fontId - 1u;

  if( ( fontId > 0u ) &&
      ( index < mFontIdCache.Count() ) )
  {
    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];

    if( FontDescription::FACE_FONT == fontIdCacheItem.type )
    {
      FT_Face ftFace = mFontFaceCache[fontIdCacheItem.id].mFreeTypeFace;

      glyphIndex = FT_Get_Char_Index( ftFace, charcode );
    }
  }

  return glyphIndex;
}

bool FontClient::Plugin::GetGlyphMetrics( GlyphInfo* array,
                                          uint32_t size,
                                          GlyphType type,
                                          bool horizontal )
{
  if( VECTOR_GLYPH == type )
  {
    return GetVectorMetrics( array, size, horizontal );
  }

  return GetBitmapMetrics( array, size, horizontal );
}

bool FontClient::Plugin::GetBitmapMetrics( GlyphInfo* array,
                                           uint32_t size,
                                           bool horizontal )
{
  bool success( true );

  for( unsigned int i=0; i<size; ++i )
  {
    GlyphInfo& glyph = array[i];

    FontId index = glyph.fontId - 1u;

    if( ( glyph.fontId > 0u ) &&
        ( index < mFontIdCache.Count() ) )
    {
      const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];

      switch( fontIdCacheItem.type )
      {
        case FontDescription::FACE_FONT:
        {
          const FontFaceCacheItem& font = mFontFaceCache[fontIdCacheItem.id];

          FT_Face ftFace = font.mFreeTypeFace;

#ifdef FREETYPE_BITMAP_SUPPORT
          // Check to see if we should be loading a Fixed Size bitmap?
          if( font.mIsFixedSizeBitmap )
          {
            FT_Select_Size( ftFace, font.mFixedSizeIndex ); ///< @todo: needs to be investigated why it's needed to select the size again.
            int error = FT_Load_Glyph( ftFace, glyph.index, FT_LOAD_COLOR );
            if ( FT_Err_Ok == error )
            {
              glyph.width = font.mFixedWidthPixels;
              glyph.height = font.mFixedHeightPixels;
              glyph.advance = font.mFixedWidthPixels;
              glyph.xBearing = 0.0f;
              glyph.yBearing = font.mFixedHeightPixels;

              // Adjust the metrics if the fixed-size font should be down-scaled
              const float desiredFixedSize =  static_cast<float>( font.mRequestedPointSize ) * FROM_266 / POINTS_PER_INCH * mDpiVertical;

              if( desiredFixedSize > 0.f )
              {
                const float scaleFactor = desiredFixedSize / font.mFixedHeightPixels;

                glyph.width = glyph.width * scaleFactor ;
                glyph.height = glyph.height * scaleFactor;
                glyph.advance = glyph.advance * scaleFactor;
                glyph.xBearing = glyph.xBearing * scaleFactor;
                glyph.yBearing = glyph.yBearing * scaleFactor;

                glyph.scaleFactor = scaleFactor;
              }
            }
            else
            {
              DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::GetBitmapMetrics. FreeType Bitmap Load_Glyph error %d\n", error );
              success = false;
            }
          }
          else
#endif
          {
            // FT_LOAD_DEFAULT causes some issues in the alignment of the glyph inside the bitmap.
            // i.e. with the SNum-3R font.
            // @todo: add an option to use the FT_LOAD_DEFAULT if required?
            int error = FT_Load_Glyph( ftFace, glyph.index, FT_LOAD_NO_AUTOHINT );

            // Keep the width of the glyph before doing the software emboldening.
            // It will be used to calculate a scale factor to be applied to the
            // advance as Harfbuzz doesn't apply any SW emboldening to calculate
            // the advance of the glyph.
            const float width = static_cast< float >( ftFace->glyph->metrics.width ) * FROM_266;

            if( FT_Err_Ok == error )
            {
              const bool isEmboldeningRequired = glyph.isBoldRequired && !( ftFace->style_flags & FT_STYLE_FLAG_BOLD );
              if( isEmboldeningRequired )
              {
                // Does the software bold.
                FT_GlyphSlot_Embolden( ftFace->glyph );
              }

              glyph.width  = static_cast< float >( ftFace->glyph->metrics.width ) * FROM_266;
              glyph.height = static_cast< float >( ftFace->glyph->metrics.height ) * FROM_266;
              if( horizontal )
              {
                glyph.xBearing += static_cast< float >( ftFace->glyph->metrics.horiBearingX ) * FROM_266;
                glyph.yBearing += static_cast< float >( ftFace->glyph->metrics.horiBearingY ) * FROM_266;
              }
              else
              {
                glyph.xBearing += static_cast< float >( ftFace->glyph->metrics.vertBearingX ) * FROM_266;
                glyph.yBearing += static_cast< float >( ftFace->glyph->metrics.vertBearingY ) * FROM_266;
              }

              if( isEmboldeningRequired && !Dali::EqualsZero( width ) )
              {
                // If the glyph is emboldened by software, the advance is multiplied by a
                // scale factor to make it slightly bigger.
                glyph.advance *= ( glyph.width / width );
              }

              // Use the bounding box of the bitmap to correct the metrics.
              // For some fonts i.e the SNum-3R the metrics need to be corrected,
              // otherwise the glyphs 'dance' up and down depending on the
              // font's point size.

              FT_Glyph ftGlyph;
              error = FT_Get_Glyph( ftFace->glyph, &ftGlyph );

              FT_BBox bbox;
              FT_Glyph_Get_CBox( ftGlyph, FT_GLYPH_BBOX_GRIDFIT, &bbox );

              const float descender = glyph.height - glyph.yBearing;
              glyph.height = ( bbox.yMax -  bbox.yMin) * FROM_266;
              glyph.yBearing = glyph.height - round( descender );

              // Created FT_Glyph object must be released with FT_Done_Glyph
              FT_Done_Glyph( ftGlyph );
            }
            else
            {
              success = false;
            }
          }
          break;
        }
        case FontDescription::BITMAP_FONT:
        {
          BitmapFontCacheItem& bitmapFontCacheItem = mBitmapFontCache[fontIdCacheItem.id];

          unsigned int index = 0u;
          for( auto& item : bitmapFontCacheItem.font.glyphs )
          {
            if( item.utf32 == glyph.index )
            {
              Devel::PixelBuffer& pixelBuffer = bitmapFontCacheItem.pixelBuffers[index];
              if( !pixelBuffer )
              {
                pixelBuffer = LoadImageFromFile( item.url );
              }

              glyph.width  = static_cast< float >( pixelBuffer.GetWidth() );
              glyph.height = static_cast< float >( pixelBuffer.GetHeight() );
              glyph.xBearing = 0.f;
              glyph.yBearing = glyph.height + item.descender;
              glyph.advance = glyph.width;
              glyph.scaleFactor = 1.f;
              break;
            }
            ++index;
          }

          success = true;
          break;
        }
        default:
        {
          DALI_LOG_INFO(gLogFilter, Debug::General, "  Invalid type of font\n");
        }
      }
    }
    else
    {
      // Check if it's an embedded image.
      if( ( 0u == glyph.fontId ) && ( 0u != glyph.index ) && ( glyph.index <= mEmbeddedItemCache.Count() ) )
      {
        const EmbeddedItem& item = mEmbeddedItemCache[glyph.index - 1u];

        glyph.width = static_cast<float>( item.width );
        glyph.height = static_cast<float>( item.height );
        glyph.xBearing = 0.f;
        glyph.yBearing = glyph.height;
        glyph.advance = glyph.width;
        glyph.scaleFactor = 1.f;
      }
      else
      {
        success = false;
      }
    }
  }

  return success;
}

bool FontClient::Plugin::GetVectorMetrics( GlyphInfo* array,
                                           uint32_t size,
                                           bool horizontal )
{
#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  bool success( true );

  for( unsigned int i = 0u; i < size; ++i )
  {
    FontId fontId = array[i].fontId;

    if( ( fontId > 0u ) &&
        ( fontId - 1u ) < mFontIdCache.Count() )
    {
      FontFaceCacheItem& font = mFontFaceCache[mFontIdCache[fontId - 1u].id];

      if( ! font.mVectorFontId )
      {
        font.mVectorFontId = mVectorFontCache->GetFontId( font.mPath );
      }

      mVectorFontCache->GetGlyphMetrics( font.mVectorFontId, array[i] );

      // Vector metrics are in EMs, convert to pixels
      const float scale = ( static_cast<float>( font.mRequestedPointSize ) * FROM_266 ) * static_cast<float>( mDpiVertical ) / POINTS_PER_INCH;
      array[i].width    *= scale;
      array[i].height   *= scale;
      array[i].xBearing *= scale;
      array[i].yBearing *= scale;
      array[i].advance  *= scale;
    }
    else
    {
      success = false;
    }
  }

  return success;
#else
  return false;
#endif
}

void FontClient::Plugin::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, Dali::TextAbstraction::FontClient::GlyphBufferData& data, int outlineWidth )
{
  const FontId index = fontId - 1u;

  if( ( fontId > 0u ) &&
      ( index < mFontIdCache.Count() ) )
  {
    data.isColorBitmap = false;
    data.isColorEmoji = false;

    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];

    switch( fontIdCacheItem.type )
    {
      case FontDescription::FACE_FONT:
      {
        // For the software italics.
        bool isShearRequired = false;

        const FontFaceCacheItem& fontFaceCacheItem = mFontFaceCache[fontIdCacheItem.id];
        FT_Face ftFace = fontFaceCacheItem.mFreeTypeFace;

        FT_Error error;

#ifdef FREETYPE_BITMAP_SUPPORT
        // Check to see if this is fixed size bitmap
        if( fontFaceCacheItem.mIsFixedSizeBitmap )
        {
          error = FT_Load_Glyph( ftFace, glyphIndex, FT_LOAD_COLOR );
        }
        else
#endif
        {
          // FT_LOAD_DEFAULT causes some issues in the alignment of the glyph inside the bitmap.
          // i.e. with the SNum-3R font.
          // @todo: add an option to use the FT_LOAD_DEFAULT if required?
          error = FT_Load_Glyph( ftFace, glyphIndex, FT_LOAD_NO_AUTOHINT );
        }
        if( FT_Err_Ok == error )
        {
          if( isBoldRequired && !( ftFace->style_flags & FT_STYLE_FLAG_BOLD ) )
          {
            // Does the software bold.
            FT_GlyphSlot_Embolden( ftFace->glyph );
          }

          if( isItalicRequired && !( ftFace->style_flags & FT_STYLE_FLAG_ITALIC ) )
          {
            // Will do the software italic.
            isShearRequired = true;
          }

          FT_Glyph glyph;
          error = FT_Get_Glyph( ftFace->glyph, &glyph );

          // Convert to bitmap if necessary
          if( FT_Err_Ok == error )
          {
            if( glyph->format != FT_GLYPH_FORMAT_BITMAP )
            {
              int offsetX = 0, offsetY = 0;
              bool isOutlineGlyph = ( glyph->format == FT_GLYPH_FORMAT_OUTLINE && outlineWidth > 0 );

              // Create a bitmap for the outline
              if( isOutlineGlyph )
              {
                // Retrieve the horizontal and vertical distance from the current pen position to the
                // left and top border of the glyph bitmap for a normal glyph before applying the outline.
                if( FT_Err_Ok == error )
                {
                  FT_Glyph normalGlyph;
                  error = FT_Get_Glyph( ftFace->glyph, &normalGlyph );

                  error = FT_Glyph_To_Bitmap( &normalGlyph, FT_RENDER_MODE_NORMAL, 0, 1 );
                  if( FT_Err_Ok == error )
                  {
                    FT_BitmapGlyph bitmapGlyph = reinterpret_cast< FT_BitmapGlyph >( normalGlyph );

                    offsetX = bitmapGlyph->left;
                    offsetY = bitmapGlyph->top;
                  }

                  // Created FT_Glyph object must be released with FT_Done_Glyph
                  FT_Done_Glyph( normalGlyph );
                }

                // Now apply the outline

                // Set up a stroker
                FT_Stroker stroker;
                error = FT_Stroker_New( mFreeTypeLibrary, &stroker );

                if( FT_Err_Ok == error )
                {
                  FT_Stroker_Set( stroker, outlineWidth * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0 );
                  error = FT_Glyph_StrokeBorder( &glyph, stroker, 0, 1 );

                  if( FT_Err_Ok == error )
                  {
                    FT_Stroker_Done( stroker );
                  }
                  else
                  {
                    DALI_LOG_ERROR( "FT_Glyph_StrokeBorder Failed with error: %d\n", error );
                  }
                }
                else
                {
                  DALI_LOG_ERROR( "FT_Stroker_New Failed with error: %d\n", error );
                }
              }

              error = FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1 );
              if( FT_Err_Ok == error )
              {
                FT_BitmapGlyph bitmapGlyph = reinterpret_cast< FT_BitmapGlyph >( glyph );

                if( isOutlineGlyph )
                {
                  // Calculate the additional horizontal and vertical offsets needed for the position of the outline glyph
                  data.outlineOffsetX = offsetX - bitmapGlyph->left - outlineWidth;
                  data.outlineOffsetY = bitmapGlyph->top - offsetY - outlineWidth;
                }

                ConvertBitmap( data, bitmapGlyph->bitmap, isShearRequired );
              }
              else
              {
                DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::CreateBitmap. FT_Get_Glyph Failed with error: %d\n", error );
              }
            }
            else
            {
              ConvertBitmap( data, ftFace->glyph->bitmap, isShearRequired );
            }

            data.isColorEmoji = fontFaceCacheItem.mIsFixedSizeBitmap;

            // Created FT_Glyph object must be released with FT_Done_Glyph
            FT_Done_Glyph( glyph );
          }
        }
        else
        {
          DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::CreateBitmap. FT_Load_Glyph Failed with error: %d\n", error );
        }
        break;
      }
      case FontDescription::BITMAP_FONT:
      {
        BitmapFontCacheItem& bitmapFontCacheItem = mBitmapFontCache[fontIdCacheItem.id];

        unsigned int index = 0u;
        for( auto& item : bitmapFontCacheItem.font.glyphs )
        {
          if( item.utf32 == glyphIndex )
          {
            Devel::PixelBuffer& pixelBuffer = bitmapFontCacheItem.pixelBuffers[index];
            if( !pixelBuffer )
            {
              pixelBuffer = LoadImageFromFile( item.url );
            }

            data.width = pixelBuffer.GetWidth();
            data.height = pixelBuffer.GetHeight();

            data.isColorBitmap = bitmapFontCacheItem.font.isColorFont;

            ConvertBitmap( data, data.width, data.height, pixelBuffer.GetBuffer() );

            // Sets the pixel format.
            data.format = pixelBuffer.GetPixelFormat();
            break;
          }
          ++index;
        }
        break;
      }
      default:
      {
        DALI_LOG_INFO(gLogFilter, Debug::General, "  Invalid type of font\n");
      }
    }
  }
  else
  {
    if( ( 0u != glyphIndex ) && ( glyphIndex <= mEmbeddedItemCache.Count() ) )
    {
      // It's an embedded item.
      const EmbeddedItem& item = mEmbeddedItemCache[glyphIndex - 1u];

      data.width = item.width;
      data.height = item.height;
      if( 0u != item.pixelBufferId )
      {
        Devel::PixelBuffer pixelBuffer = mPixelBufferCache[item.pixelBufferId-1u].pixelBuffer;
        if( pixelBuffer )
        {
          ConvertBitmap( data, pixelBuffer.GetWidth(), pixelBuffer.GetHeight(), pixelBuffer.GetBuffer() );

          // Sets the pixel format.
          data.format = pixelBuffer.GetPixelFormat();
        }
      }
      else
      {
        // Creates the output buffer
        const unsigned int bufferSize = data.width * data.height * 4u;
        data.buffer = new unsigned char[bufferSize]; // @note The caller is responsible for deallocating the bitmap data using delete[].

        memset( data.buffer, 0u, bufferSize );

        // Just creates a void buffer. Doesn't matter what pixel format is set as is the application code the responsible of filling it.
      }
    }
  }
}

PixelData FontClient::Plugin::CreateBitmap( FontId fontId, GlyphIndex glyphIndex, int outlineWidth )
{
  TextAbstraction::FontClient::GlyphBufferData data;

  CreateBitmap( fontId, glyphIndex, false, false, data, outlineWidth );

  return PixelData::New( data.buffer,
                         data.width * data.height * Pixel::GetBytesPerPixel( data.format ),
                         data.width,
                         data.height,
                         data.format,
                         PixelData::DELETE_ARRAY );
}

void FontClient::Plugin::CreateVectorBlob( FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight )
{
  blob = nullptr;
  blobLength = 0;

#ifdef ENABLE_VECTOR_BASED_TEXT_RENDERING
  if( ( fontId > 0u ) &&
      ( fontId - 1u < mFontIdCache.Count() ) )
  {
    const FontId fontFaceId = mFontIdCache[fontId - 1u].id;
    FontFaceCacheItem& font = mFontFaceCache[fontFaceId];

    if( ! font.mVectorFontId )
    {
      font.mVectorFontId = mVectorFontCache->GetFontId( font.mPath );
    }

    mVectorFontCache->GetVectorBlob( font.mVectorFontId, fontFaceId, glyphIndex, blob, blobLength, nominalWidth, nominalHeight );
  }
#endif
}

const GlyphInfo& FontClient::Plugin::GetEllipsisGlyph( PointSize26Dot6 requestedPointSize )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::GetEllipsisGlyph\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  requestedPointSize %d.\n", requestedPointSize );

  // First look into the cache if there is an ellipsis glyph for the requested point size.
  for( const auto& item : mEllipsisCache )
  {
    if( item.requestedPointSize == requestedPointSize )
    {
      // Use the glyph in the cache.

      DALI_LOG_INFO( gLogFilter, Debug::General, "  glyph id %d found in the cache.\n", item.glyph.index );
      DALI_LOG_INFO( gLogFilter, Debug::General, "      font %d.\n", item.glyph.fontId );
      DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetEllipsisGlyph\n" );

      return item.glyph;
    }
  }

  // No glyph has been found. Create one.
  mEllipsisCache.PushBack( EllipsisItem() );
  EllipsisItem& item = *( mEllipsisCache.End() - 1u );

  item.requestedPointSize = requestedPointSize;

  // Find a font for the ellipsis glyph.
  item.glyph.fontId = FindDefaultFont( ELLIPSIS_CHARACTER,
                                       requestedPointSize,
                                       false );

  // Set the character index to access the glyph inside the font.
  item.glyph.index = FT_Get_Char_Index( mFontFaceCache[mFontIdCache[item.glyph.fontId-1u].id].mFreeTypeFace,
                                        ELLIPSIS_CHARACTER );

  GetBitmapMetrics( &item.glyph, 1u, true );

  DALI_LOG_INFO( gLogFilter, Debug::General, "  glyph id %d found in the cache.\n", item.glyph.index );
  DALI_LOG_INFO( gLogFilter, Debug::General, "      font %d.\n", item.glyph.fontId );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::GetEllipsisGlyph\n" );

  return item.glyph;
}

bool FontClient::Plugin::IsColorGlyph( FontId fontId, GlyphIndex glyphIndex )
{
  FT_Error error = -1;

  const FontId index = fontId - 1u;

  if( ( fontId > 0u ) &&
      ( index < mFontIdCache.Count() ) )
  {
    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];

    switch( fontIdCacheItem.type )
    {
      case FontDescription::FACE_FONT:
      {
#ifdef FREETYPE_BITMAP_SUPPORT
        const FontFaceCacheItem& item = mFontFaceCache[fontIdCacheItem.id];
        FT_Face ftFace = item.mFreeTypeFace;

        // Check to see if this is fixed size bitmap
        if( item.mHasColorTables )
        {
          error = FT_Load_Glyph( ftFace, glyphIndex, FT_LOAD_COLOR );
        }
#endif
        break;
      }
      case FontDescription::BITMAP_FONT:
      {
        error = FT_Err_Ok; // Will return true;
        break;
      }
      default:
      {
        DALI_LOG_INFO(gLogFilter, Debug::General, "  Invalid type of font\n");
      }
    }
  }

  return FT_Err_Ok == error;
}

FT_FaceRec_* FontClient::Plugin::GetFreetypeFace( FontId fontId )
{
  FT_Face fontFace = nullptr;

  const FontId index = fontId - 1u;
  if( ( fontId > 0u ) &&
      ( index < mFontIdCache.Count() ) )
  {
    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];

    if( FontDescription::FACE_FONT == fontIdCacheItem.type )
    {
      fontFace = mFontFaceCache[fontIdCacheItem.id].mFreeTypeFace;
    }
  }
  return fontFace;
}

FontDescription::Type FontClient::Plugin::GetFontType( FontId fontId )
{
  const FontId index = fontId - 1u;
  if( ( fontId > 0u ) &&
      ( index < mFontIdCache.Count() ) )
  {
    return mFontIdCache[index].type;
  }
  return FontDescription::INVALID;
}

bool FontClient::Plugin::AddCustomFontDirectory( const FontPath& path )
{
  // nullptr as first parameter means the current configuration is used.
  return FcConfigAppFontAddDir( nullptr, reinterpret_cast<const FcChar8 *>( path.c_str() ) );
}

GlyphIndex FontClient::Plugin::CreateEmbeddedItem( const TextAbstraction::FontClient::EmbeddedItemDescription& description, Pixel::Format& pixelFormat )
{
  EmbeddedItem embeddedItem;

  embeddedItem.pixelBufferId = 0u;
  embeddedItem.width = description.width;
  embeddedItem.height = description.height;

  pixelFormat = Pixel::A8;

  if( !description.url.empty() )
  {
    // Check if the url is in the cache.
    PixelBufferId index = 0u;

    for( const auto& cacheItem : mPixelBufferCache )
    {
      ++index;
      if( cacheItem.url == description.url )
      {
        // The url is in the pixel buffer cache.
        // Set the index +1 to the vector.
        embeddedItem.pixelBufferId = index;
        break;
      }
    }

    Devel::PixelBuffer pixelBuffer;
    if( 0u == embeddedItem.pixelBufferId )
    {
      // The pixel buffer is not in the cache. Create one and cache it.

      // Load the image from the url.
      pixelBuffer = LoadImageFromFile( description.url );

      // Create the cache item.
      PixelBufferCacheItem pixelBufferCacheItem;
      pixelBufferCacheItem.pixelBuffer = pixelBuffer;
      pixelBufferCacheItem.url = description.url;

      // Store the cache item in the cache.
      mPixelBufferCache.push_back( std::move( pixelBufferCacheItem ) );

      // Set the pixel buffer id to the embedded item.
      embeddedItem.pixelBufferId = mPixelBufferCache.size();
    }
    else
    {
      // Retrieve the pixel buffer from the cache to set the pixel format.
      pixelBuffer = mPixelBufferCache[embeddedItem.pixelBufferId-1u].pixelBuffer;
    }

    if( pixelBuffer )
    {
      // Set the size of the embedded item if it has not been set.
      if( 0u == embeddedItem.width )
      {
        embeddedItem.width = static_cast<unsigned int>( pixelBuffer.GetWidth() );
      }

      if( 0u == embeddedItem.height )
      {
        embeddedItem.height = static_cast<unsigned int>( pixelBuffer.GetHeight() );
      }

      // Set the pixel format.
      pixelFormat = pixelBuffer.GetPixelFormat();
    }
  }

  // Find if the same embeddedItem has already been created.
  unsigned int index = 0u;
  for( const auto& item : mEmbeddedItemCache )
  {
    ++index;
    if( ( item.pixelBufferId == embeddedItem.pixelBufferId ) &&
        ( item.width == embeddedItem.width ) &&
        ( item.height == embeddedItem.height ) )
    {
      return index;
    }
  }

  // Cache the embedded item.
  mEmbeddedItemCache.PushBack( embeddedItem );

  return mEmbeddedItemCache.Count();
}

void FontClient::Plugin::InitSystemFonts()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::InitSystemFonts\n" );

  FcFontSet* fontSet = GetFcFontSet(); // Creates a FcFontSet that needs to be destroyed by calling FcFontSetDestroy.

  if( fontSet )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "  number of system fonts : %d\n", fontSet->nfont );

    // Reserve some space to avoid reallocations.
    mSystemFonts.reserve( fontSet->nfont );

    for( int i = 0u; i < fontSet->nfont; ++i )
    {
      FcPattern* fontPattern = fontSet->fonts[i];

      FontPath path;

      // Skip fonts with no path
      if( GetFcString( fontPattern, FC_FILE, path ) )
      {
        mSystemFonts.push_back( FontDescription() );
        FontDescription& fontDescription = mSystemFonts.back();

        fontDescription.path = std::move( path );

        int width = 0;
        int weight = 0;
        int slant = 0;
        GetFcString( fontPattern, FC_FAMILY, fontDescription.family );
        GetFcInt( fontPattern, FC_WIDTH, width );
        GetFcInt( fontPattern, FC_WEIGHT, weight );
        GetFcInt( fontPattern, FC_SLANT, slant );
        fontDescription.width = IntToWidthType( width );
        fontDescription.weight = IntToWeightType( weight );
        fontDescription.slant = IntToSlantType( slant );

        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  description; family : [%s]\n", fontDescription.family.c_str() );
        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", fontDescription.path.c_str() );
        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[fontDescription.width] );
        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[fontDescription.weight] );
        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[fontDescription.slant] );
      }
    }

    // Destroys the font set created.
    FcFontSetDestroy( fontSet );
  }
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::InitSystemFonts\n" );
}

bool FontClient::Plugin::MatchFontDescriptionToPattern( FcPattern* pattern, Dali::TextAbstraction::FontDescription& fontDescription, FcCharSet** characterSet )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::MatchFontDescriptionToPattern\n" );

  FcResult result = FcResultMatch;
  FcPattern* match = FcFontMatch( nullptr /* use default configure */, pattern, &result ); // Creates a new font pattern that needs to be destroyed by calling FcPatternDestroy.

  const bool matched = nullptr != match;
  DALI_LOG_INFO( gLogFilter, Debug::General, "  pattern matched : %s\n", ( matched ? "true" : "false" ) );

  if( matched )
  {
    int width = 0;
    int weight = 0;
    int slant = 0;
    GetFcString( match, FC_FILE, fontDescription.path );
    GetFcString( match, FC_FAMILY, fontDescription.family );
    GetFcInt( match, FC_WIDTH, width );
    GetFcInt( match, FC_WEIGHT, weight );
    GetFcInt( match, FC_SLANT, slant );
    fontDescription.width = IntToWidthType( width );
    fontDescription.weight = IntToWeightType( weight );
    fontDescription.slant = IntToSlantType( slant );

    // Retrieve the character set and increase the reference counter.
    FcPatternGetCharSet( match, FC_CHARSET, 0u, characterSet );
    *characterSet = FcCharSetCopy( *characterSet );

    // destroyed the matched pattern
    FcPatternDestroy( match );

    DALI_LOG_INFO( gLogFilter, Debug::General, "  description; family : [%s]\n", fontDescription.family.c_str() );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", fontDescription.path.c_str() );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[fontDescription.width] );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[fontDescription.weight] );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[fontDescription.slant] );
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::MatchFontDescriptionToPattern\n" );
  return matched;
}

FcPattern* FontClient::Plugin::CreateFontFamilyPattern( const FontDescription& fontDescription ) const
{
  // create the cached font family lookup pattern
  // a pattern holds a set of names, each name refers to a property of the font
  FcPattern* fontFamilyPattern = FcPatternCreate(); // FcPatternCreate creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

  if( !fontFamilyPattern )
  {
    return nullptr;
  }

  // add a property to the pattern for the font family
  FcPatternAddString( fontFamilyPattern, FC_FAMILY, reinterpret_cast<const FcChar8*>( fontDescription.family.c_str() ) );

  // add a property to the pattern for local setting.
  const char* locale = setlocale( LC_MESSAGES, nullptr );
  if( locale != nullptr)
  {
    FcPatternAddString( fontFamilyPattern, FC_LANG, reinterpret_cast<const FcChar8*>( locale ) );
  }

  int width = FONT_WIDTH_TYPE_TO_INT[fontDescription.width];
  if( width < 0 )
  {
    // Use default.
    width = DEFAULT_FONT_WIDTH;
  }

  int weight = FONT_WEIGHT_TYPE_TO_INT[fontDescription.weight];
  if( weight < 0 )
  {
    // Use default.
    weight = DEFAULT_FONT_WEIGHT;
  }

  int slant = FONT_SLANT_TYPE_TO_INT[fontDescription.slant];
  if( slant < 0 )
  {
    // Use default.
    slant = DEFAULT_FONT_SLANT;
  }

  FcPatternAddInteger( fontFamilyPattern, FC_WIDTH, width );
  FcPatternAddInteger( fontFamilyPattern, FC_WEIGHT, weight );
  FcPatternAddInteger( fontFamilyPattern, FC_SLANT, slant );

  // modify the config, with the mFontFamilyPatterm
  FcConfigSubstitute( nullptr /* use default configure */, fontFamilyPattern, FcMatchPattern );

  // provide default values for unspecified properties in the font pattern
  // e.g. patterns without a specified style or weight are set to Medium
  FcDefaultSubstitute( fontFamilyPattern );

  return fontFamilyPattern;
}

_FcFontSet* FontClient::Plugin::GetFcFontSet() const
{
  FcFontSet* fontset = nullptr;

  // create a new pattern.
  // a pattern holds a set of names, each name refers to a property of the font
  FcPattern* pattern = FcPatternCreate();

  if( nullptr != pattern )
  {
    // create an object set used to define which properties are to be returned in the patterns from FcFontList.
    FcObjectSet* objectSet = FcObjectSetCreate();

    if( nullptr != objectSet )
    {
      // build an object set from a list of property names
      FcObjectSetAdd( objectSet, FC_FILE );
      FcObjectSetAdd( objectSet, FC_FAMILY );
      FcObjectSetAdd( objectSet, FC_WIDTH );
      FcObjectSetAdd( objectSet, FC_WEIGHT );
      FcObjectSetAdd( objectSet, FC_SLANT );

      // get a list of fonts
      // creates patterns from those fonts containing only the objects in objectSet and returns the set of unique such patterns
      fontset = FcFontList( nullptr /* the default configuration is checked to be up to date, and used */, pattern, objectSet ); // Creates a FcFontSet that needs to be destroyed by calling FcFontSetDestroy.

      // clear up the object set
      FcObjectSetDestroy( objectSet );
    }

    // clear up the pattern
    FcPatternDestroy( pattern );
  }

  return fontset;
}

bool FontClient::Plugin::GetFcString( const FcPattern* const pattern,
                                      const char* const n,
                                      std::string& string )
{
  FcChar8* file = nullptr;
  const FcResult retVal = FcPatternGetString( pattern, n, 0u, &file );

  if( FcResultMatch == retVal )
  {
    // Have to use reinterpret_cast because FcChar8 is unsigned char*, not a const char*.
    string.assign( reinterpret_cast<const char*>( file ) );

    return true;
  }

  return false;
}

bool FontClient::Plugin::GetFcInt( const _FcPattern* const pattern, const char* const n, int& intVal )
{
  const FcResult retVal = FcPatternGetInteger( pattern, n, 0u, &intVal );

  if( FcResultMatch == retVal )
  {
    return true;
  }

  return false;
}

FontId FontClient::Plugin::CreateFont( const FontPath& path,
                                       PointSize26Dot6 requestedPointSize,
                                       FaceIndex faceIndex,
                                       bool cacheDescription )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::CreateFont\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "                path : [%s]\n", path.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize );

  FontId id = 0u;

  // Create & cache new font face
  FT_Face ftFace;
  int error = FT_New_Face( mFreeTypeLibrary,
                           path.c_str(),
                           0,
                           &ftFace );

  if( FT_Err_Ok == error )
  {
    // Check if a font is scalable.
    const bool isScalable = ( 0 != ( ftFace->face_flags & FT_FACE_FLAG_SCALABLE ) );
    const bool hasFixedSizedBitmaps = ( 0 != ( ftFace->face_flags & FT_FACE_FLAG_FIXED_SIZES ) ) && ( 0 != ftFace->num_fixed_sizes );
    const bool hasColorTables = ( 0 != ( ftFace->face_flags & FT_FACE_FLAG_COLOR ) );
    FontId fontFaceId = 0u;

    DALI_LOG_INFO( gLogFilter, Debug::General, "            isScalable : [%s]\n", ( isScalable ? "true" : "false" ) );
    DALI_LOG_INFO( gLogFilter, Debug::General, "  hasFixedSizedBitmaps : [%s]\n", ( hasFixedSizedBitmaps ? "true" : "false" ) );
    DALI_LOG_INFO( gLogFilter, Debug::General, "        hasColorTables : [%s]\n", ( hasColorTables ? "true" : "false" ) );

    // Check to see if the font contains fixed sizes?
    if( !isScalable && hasFixedSizedBitmaps )
    {
      PointSize26Dot6 actualPointSize = 0u;
      int fixedSizeIndex = 0;
      for( ; fixedSizeIndex < ftFace->num_fixed_sizes; ++fixedSizeIndex )
      {
        const PointSize26Dot6 fixedSize = ftFace->available_sizes[fixedSizeIndex].size;
        DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  size index : %d, size : %d\n", fixedSizeIndex, fixedSize );

        if( fixedSize >= requestedPointSize )
        {
          actualPointSize = fixedSize;
          break;
        }
      }

      if( 0u == actualPointSize )
      {
        // The requested point size is bigger than the bigest fixed size.
        fixedSizeIndex = ftFace->num_fixed_sizes - 1;
        actualPointSize = ftFace->available_sizes[fixedSizeIndex].size;
      }

      DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  size index : %d, actual size : %d\n", fixedSizeIndex, actualPointSize );

      // Tell Freetype to use this size
      error = FT_Select_Size( ftFace, fixedSizeIndex );
      if ( FT_Err_Ok != error )
      {
        DALI_LOG_INFO( gLogFilter, Debug::General, "FreeType Select_Size error: %d\n", error );
      }
      else
      {
        const float fixedWidth  = static_cast<float>( ftFace->available_sizes[ fixedSizeIndex ].width );
        const float fixedHeight = static_cast<float>( ftFace->available_sizes[ fixedSizeIndex ].height );

        // Indicate that the font is a fixed sized bitmap
        FontMetrics metrics( fixedHeight, // The ascender in pixels.
                             0.0f,
                             fixedHeight, // The height in pixels.
                             0.0f,
                             0.0f );

        // Create the FreeType font face item to cache.
        FontFaceCacheItem fontFaceCacheItem( ftFace, path, requestedPointSize, faceIndex, metrics, fixedSizeIndex, fixedWidth, fixedHeight, hasColorTables );

        // Set the index to the font's id cache.
        fontFaceCacheItem.mFontId = mFontIdCache.Count();

        // Create the font id item to cache.
        FontIdCacheItem fontIdCacheItem;
        fontIdCacheItem.type = FontDescription::FACE_FONT;

        // Set the index to the FreeType font face cache.
        fontIdCacheItem.id = mFontFaceCache.size();
        fontFaceId = fontIdCacheItem.id + 1u;

        // Cache the items.
        mFontFaceCache.push_back( fontFaceCacheItem );
        mFontIdCache.PushBack( fontIdCacheItem );

        // Set the font id to be returned.
        id = mFontIdCache.Count();
      }
    }
    else
    {
      error = FT_Set_Char_Size( ftFace,
                                0,
                                requestedPointSize,
                                mDpiHorizontal,
                                mDpiVertical );

      if( FT_Err_Ok == error )
      {

        FT_Size_Metrics& ftMetrics = ftFace->size->metrics;

        FontMetrics metrics( static_cast< float >( ftMetrics.ascender  ) * FROM_266,
                             static_cast< float >( ftMetrics.descender ) * FROM_266,
                             static_cast< float >( ftMetrics.height    ) * FROM_266,
                             static_cast< float >( ftFace->underline_position ) * FROM_266,
                             static_cast< float >( ftFace->underline_thickness ) * FROM_266 );

        // Create the FreeType font face item to cache.
        FontFaceCacheItem fontFaceCacheItem( ftFace, path, requestedPointSize, faceIndex, metrics );

        // Set the index to the font's id cache.
        fontFaceCacheItem.mFontId = mFontIdCache.Count();

        // Create the font id item to cache.
        FontIdCacheItem fontIdCacheItem;
        fontIdCacheItem.type = FontDescription::FACE_FONT;

        // Set the index to the FreeType font face cache.
        fontIdCacheItem.id = mFontFaceCache.size();
        fontFaceId = fontIdCacheItem.id + 1u;

        // Cache the items.
        mFontFaceCache.push_back( fontFaceCacheItem );
        mFontIdCache.PushBack( fontIdCacheItem );

        // Set the font id to be returned.
        id = mFontIdCache.Count();
      }
      else
      {
        DALI_LOG_INFO( gLogFilter, Debug::General, "  FreeType Set_Char_Size error: %d for pointSize %d\n", error, requestedPointSize );
      }
    }

    if( 0u != fontFaceId )
    {
      if( cacheDescription )
      {
        CacheFontPath( ftFace, fontFaceId, requestedPointSize, path );
      }
    }
  }
  else
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "  FreeType New_Face error: %d for [%s]\n", error, path.c_str() );
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  font id : %d\n", id );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::CreateFont\n" );

  return id;
}

void FontClient::Plugin::ConvertBitmap( TextAbstraction::FontClient::GlyphBufferData& data, unsigned int srcWidth, unsigned int srcHeight, const unsigned char* const srcBuffer )
{
  // Set the input dimensions.
  const ImageDimensions inputDimensions( srcWidth, srcHeight );

  // Set the output dimensions.
  // If the output dimension is not given, the input dimension is set
  // and won't be downscaling.
  data.width = ( data.width == 0 ) ? srcWidth : data.width;
  data.height = ( data.height == 0 ) ? srcHeight : data.height;
  const ImageDimensions desiredDimensions( data.width, data.height );

  // Creates the output buffer
  const unsigned int bufferSize = data.width * data.height * 4u;
  data.buffer = new unsigned char[bufferSize]; // @note The caller is responsible for deallocating the bitmap data using delete[].

  if( inputDimensions == desiredDimensions )
  {
    // There isn't downscaling.
    memcpy( data.buffer, srcBuffer, bufferSize );
  }
  else
  {
    Dali::Internal::Platform::LanczosSample4BPP( srcBuffer,
                                                 inputDimensions,
                                                 data.buffer,
                                                 desiredDimensions );
  }
}

void FontClient::Plugin::ConvertBitmap( TextAbstraction::FontClient::GlyphBufferData& data, FT_Bitmap srcBitmap, bool isShearRequired )
{
  if( srcBitmap.width*srcBitmap.rows > 0 )
  {
    switch( srcBitmap.pixel_mode )
    {
      case FT_PIXEL_MODE_GRAY:
      {
        if( srcBitmap.pitch == static_cast<int>( srcBitmap.width ) )
        {
          uint8_t* pixelsIn = srcBitmap.buffer;
          unsigned int width = srcBitmap.width;
          unsigned height = srcBitmap.rows;

          std::unique_ptr<uint8_t, void(*)(void*)> pixelsOutPtr( nullptr, free );

          if( isShearRequired )
          {
            /**
             * Glyphs' bitmaps with no slant retrieved from FreeType:
             * __________     ____
             * |XXXXXXXX|     |XX|
             * |   XX   |     |XX|
             * |   XX   |     |XX|
             * |   XX   |     |XX|
             * |   XX   |     |XX|
             * |   XX   |     |XX|
             * ----------     ----
             *
             * Expected glyphs' bitmaps with italic slant:
             * ____________   ______
             * |  XXXXXXXX|   |  XX|
             * |     XX   |   |  XX|
             * |    XX    |   | XX |
             * |    XX    |   | XX |
             * |   XX     |   |XX  |
             * |   XX     |   |XX  |
             * ------------   ------
             *
             * Glyphs' bitmaps with software italic slant retrieved from FreeType:
             * __________     ______
             * |XXXXXXXX|     |  XX|
             * |   XX   |     |  XX|
             * |  XX    |     | XX |
             * |  XX    |     | XX |
             * | XX     |     |XX  |
             * | XX     |     |XX  |
             * ----------     ------
             *
             * This difference in some bitmaps' width causes an overlap of some glyphs. This is the reason why a shear operation is done here instead of relying on the experimental FT_GlyphSlot_Oblique() implementation.
             */
            unsigned int widthOut = 0u;
            unsigned int heightOut = 0u;
            uint8_t* pixelsOut = nullptr;

            Dali::Internal::Platform::HorizontalShear( pixelsIn,
                                                       width,
                                                       height,
                                                       1u,
                                                       -TextAbstraction::FontClient::DEFAULT_ITALIC_ANGLE,
                                                       pixelsOut,
                                                       widthOut,
                                                       heightOut );

            width = widthOut;
            height = heightOut;
            pixelsIn = pixelsOut;
            pixelsOutPtr.reset( pixelsOut );
          }

          const unsigned int bufferSize = width * height;
          data.buffer = new unsigned char[bufferSize]; // @note The caller is responsible for deallocating the bitmap data using delete[].
          data.width = width;
          data.height = height;
          data.format = Pixel::L8; // Sets the pixel format.
          memcpy( data.buffer, pixelsIn, bufferSize );
        }
        break;
      }

#ifdef FREETYPE_BITMAP_SUPPORT
      case FT_PIXEL_MODE_BGRA:
      {
        if( srcBitmap.pitch == static_cast<int>( srcBitmap.width << 2u ) )
        {
          ConvertBitmap( data, srcBitmap.width, srcBitmap.rows, srcBitmap.buffer );

          // Sets the pixel format.
          data.format = Pixel::BGRA8888;
        }
        break;
      }
#endif
      default:
      {
        DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::ConvertBitmap. FontClient Unable to create Bitmap of this PixelType\n" );
        break;
      }
    }
  }
}

bool FontClient::Plugin::FindFont( const FontPath& path,
                                   PointSize26Dot6 requestedPointSize,
                                   FaceIndex faceIndex,
                                   FontId& fontId ) const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::FindFont\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "                path : [%s]\n", path.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  number of fonts in the cache : %d\n", mFontFaceCache.size() );

  fontId = 0u;
  for( const auto& cacheItem : mFontFaceCache )
  {
    if( cacheItem.mRequestedPointSize == requestedPointSize &&
        cacheItem.mFaceIndex == faceIndex &&
        cacheItem.mPath == path )
    {
      fontId = cacheItem.mFontId + 1u;

      DALI_LOG_INFO( gLogFilter, Debug::General, "  font found, id : %d\n", fontId );
      DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindFont\n" );

      return true;
    }
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  font not found\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindFont\n" );

  return false;
}

bool FontClient::Plugin::FindValidatedFont( const FontDescription& fontDescription,
                                            FontDescriptionId& validatedFontId )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::FindValidatedFont\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  description; family : [%s]\n", fontDescription.family.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", fontDescription.path.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[fontDescription.width] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[fontDescription.weight] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[fontDescription.slant] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  number of validated fonts in the cache : %d\n", mValidatedFontCache.size() );

  validatedFontId = 0u;

  for( const auto& item : mValidatedFontCache )
  {
    if( !fontDescription.family.empty() &&
        ( fontDescription.family == item.fontDescription.family ) &&
        ( fontDescription.width == item.fontDescription.width ) &&
        ( fontDescription.weight == item.fontDescription.weight ) &&
        ( fontDescription.slant == item.fontDescription.slant ) )
    {
      validatedFontId = item.index;

      DALI_LOG_INFO( gLogFilter, Debug::General, "  validated font found, id : %d\n", validatedFontId );
      DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindValidatedFont\n" );
      return true;
    }
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  validated font not found\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindValidatedFont\n" );
  return false;
}

bool FontClient::Plugin::FindFallbackFontList( const FontDescription& fontDescription,
                                               FontList*& fontList,
                                               CharacterSetList*& characterSetList )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::FindFallbackFontList\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  description; family : [%s]\n", fontDescription.family.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                 path : [%s]\n", fontDescription.path.c_str() );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                width : [%s]\n", FontWidth::Name[fontDescription.width] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "               weight : [%s]\n", FontWeight::Name[fontDescription.weight] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "                slant : [%s]\n\n", FontSlant::Name[fontDescription.slant] );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "  number of fallback font lists in the cache : %d\n", mFallbackCache.size() );

  fontList = nullptr;

  for( const auto& item : mFallbackCache )
  {
    if( !fontDescription.family.empty() &&
        ( fontDescription.family == item.fontDescription.family ) &&
        ( fontDescription.width == item.fontDescription.width ) &&
        ( fontDescription.weight == item.fontDescription.weight ) &&
        ( fontDescription.slant == item.fontDescription.slant ) )
    {
      fontList = item.fallbackFonts;
      characterSetList = item.characterSets;

      DALI_LOG_INFO( gLogFilter, Debug::General, "  fallback font list found.\n" );
      DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindFallbackFontList\n" );
      return true;
    }
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  fallback font list not found.\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindFallbackFontList\n" );
  return false;
}

bool FontClient::Plugin::FindFont( FontDescriptionId validatedFontId,
                                   PointSize26Dot6 requestedPointSize,
                                   FontId& fontId )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "-->FontClient::Plugin::FindFont\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "    validatedFontId  : %d\n", validatedFontId );
  DALI_LOG_INFO( gLogFilter, Debug::General, "  requestedPointSize : %d\n", requestedPointSize );

  fontId = 0u;

  for( const auto& item : mFontDescriptionSizeCache )
  {
    if( ( validatedFontId == item.validatedFontId ) &&
        ( requestedPointSize == item.requestedPointSize ) )
    {
      fontId = item.fontId;

      DALI_LOG_INFO( gLogFilter, Debug::General, "  font found, id : %d\n", fontId );
      DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindFont\n" );
      return true;
    }
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "  font not found.\n" );
  DALI_LOG_INFO( gLogFilter, Debug::General, "<--FontClient::Plugin::FindFont\n" );
  return false;
}

bool FontClient::Plugin::FindBitmapFont( const FontFamily& bitmapFont, FontId& fontId ) const
{
  fontId = 0u;

  for( const auto& item : mBitmapFontCache )
  {
    if( bitmapFont == item.font.name )
    {
      fontId = item.id + 1u;
      return true;
    }
  }

  return false;
}

bool FontClient::Plugin::IsScalable( const FontPath& path )
{
  bool isScalable = false;

  FT_Face ftFace;
  int error = FT_New_Face( mFreeTypeLibrary,
                           path.c_str(),
                           0,
                           &ftFace );
  if( FT_Err_Ok != error )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::IsScalable. FreeType Cannot check font: %s\n", path.c_str() );
  }
  else
  {
    isScalable = ftFace->face_flags & FT_FACE_FLAG_SCALABLE;
  }

  return isScalable;
}

bool FontClient::Plugin::IsScalable( const FontDescription& fontDescription )
{
  // Create a font pattern.
  FcPattern* fontFamilyPattern = CreateFontFamilyPattern( fontDescription ); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.

  FcResult result = FcResultMatch;

  // match the pattern
  FcPattern* match = FcFontMatch( nullptr /* use default configure */, fontFamilyPattern, &result ); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.
  bool isScalable = false;

  if( match )
  {
    // Get the path to the font file name.
    FontPath path;
    GetFcString( match, FC_FILE, path );
    isScalable = IsScalable( path );
  }
  else
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::IsScalable. FreeType Cannot check font: [%s]\n", fontDescription.family.c_str() );
  }

  // Destroys the created patterns.
  FcPatternDestroy( match );
  FcPatternDestroy( fontFamilyPattern );

  return isScalable;
}

void FontClient::Plugin::GetFixedSizes( const FontPath& path, Vector< PointSize26Dot6 >& sizes )
{
  // Empty the caller container
  sizes.Clear();

  FT_Face ftFace;
  int error = FT_New_Face( mFreeTypeLibrary,
                           path.c_str(),
                           0,
                           &ftFace );
  if( FT_Err_Ok != error )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::GetFixedSizes. FreeType Cannot check font path : [%s]\n", path.c_str() );
  }

  // Fetch the number of fixed sizes available
  if ( ftFace->num_fixed_sizes && ftFace->available_sizes )
  {
    for ( int i = 0; i < ftFace->num_fixed_sizes; ++i )
    {
      sizes.PushBack( ftFace->available_sizes[ i ].size );
    }
  }
}

void FontClient::Plugin::GetFixedSizes( const FontDescription& fontDescription,
                                        Vector< PointSize26Dot6 >& sizes )
{
  // Create a font pattern.
  FcPattern* fontFamilyPattern = CreateFontFamilyPattern( fontDescription ); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.

  FcResult result = FcResultMatch;

  // match the pattern
  FcPattern* match = FcFontMatch( nullptr /* use default configure */, fontFamilyPattern, &result ); // Creates a font pattern that needs to be destroyed by calling FcPatternDestroy.

  if( match )
  {
    // Get the path to the font file name.
    FontPath path;
    GetFcString( match, FC_FILE, path );
    GetFixedSizes( path, sizes );
  }
  else
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::GetFixedSizes. FreeType Cannot check font: [%s]\n", fontDescription.family.c_str() );
  }

  // Destroys the created patterns.
  FcPatternDestroy( match );
  FcPatternDestroy( fontFamilyPattern );
}

bool FontClient::Plugin::HasItalicStyle( FontId fontId ) const
{
  bool hasItalicStyle = false;

  const FontId index = fontId - 1u;

  if( ( fontId > 0 ) &&
      ( index < mFontIdCache.Count() ) )
  {
    const FontIdCacheItem& fontIdCacheItem = mFontIdCache[index];

    if( FontDescription::FACE_FONT == fontIdCacheItem.type )
    {
      const FontFaceCacheItem& font = mFontFaceCache[fontIdCacheItem.id];

      hasItalicStyle = 0u != ( font.mFreeTypeFace->style_flags & FT_STYLE_FLAG_ITALIC );
    }
  }
  else
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "FontClient::Plugin::GetFontMetrics. Invalid font id : %d\n", fontId );
  }

  return hasItalicStyle;
}

void FontClient::Plugin::CacheFontPath( FT_Face ftFace, FontId id, PointSize26Dot6 requestedPointSize,  const FontPath& path )
{
  FontDescription description;
  description.path = path;
  description.family = std::move( FontFamily( ftFace->family_name ) );
  description.weight = FontWeight::NONE;
  description.width = FontWidth::NONE;
  description.slant = FontSlant::NONE;

  // Note FreeType doesn't give too much info to build a proper font style.
  if( ftFace->style_flags & FT_STYLE_FLAG_ITALIC )
  {
    description.slant = FontSlant::ITALIC;
  }
  if( ftFace->style_flags & FT_STYLE_FLAG_BOLD )
  {
    description.weight = FontWeight::BOLD;
  }

  FontDescriptionId validatedFontId = 0u;
  if( !FindValidatedFont( description,
                          validatedFontId ) )
  {
    FcPattern* pattern = CreateFontFamilyPattern( description ); // Creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

    FcResult result = FcResultMatch;
    FcPattern* match = FcFontMatch( nullptr, pattern, &result ); // FcFontMatch creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

    FcCharSet* characterSet = nullptr;
    FcPatternGetCharSet( match, FC_CHARSET, 0u, &characterSet );

    const FontId fontFaceId = id - 1u;
    mFontFaceCache[fontFaceId].mCharacterSet = FcCharSetCopy( characterSet ); // Increases the reference counter.

    // Destroys the created patterns.
    FcPatternDestroy( match );
    FcPatternDestroy( pattern );

    // Add the path to the cache.
    description.type = FontDescription::FACE_FONT;
    mFontDescriptionCache.push_back( description );

    // Set the index to the vector of paths to font file names.
    validatedFontId = mFontDescriptionCache.size();

    // Increase the reference counter and add the character set to the cache.
    mCharacterSetCache.PushBack( FcCharSetCopy( characterSet ) );

    // Cache the index and the font's description.
    mValidatedFontCache.push_back( std::move( FontDescriptionCacheItem( std::move( description ),
                                                                        validatedFontId) ) );

    // Cache the pair 'validatedFontId, requestedPointSize' to improve the following queries.
    mFontDescriptionSizeCache.push_back( FontDescriptionSizeCacheItem( validatedFontId,
                                                                       requestedPointSize,
                                                                       fontFaceId ) );
  }
}

FcCharSet* FontClient::Plugin::CreateCharacterSetFromDescription( const FontDescription& description )
{
  FcCharSet* characterSet = nullptr;

  FcPattern* pattern = CreateFontFamilyPattern( description ); // Creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

  if( nullptr != pattern )
  {
    FcResult result = FcResultMatch;
    FcPattern* match = FcFontMatch( nullptr, pattern, &result ); // FcFontMatch creates a new pattern that needs to be destroyed by calling FcPatternDestroy.

    FcPatternGetCharSet( match, FC_CHARSET, 0u, &characterSet );

    // Destroys the created patterns.
    FcPatternDestroy( match );
    FcPatternDestroy( pattern );
  }

  return characterSet;
}

void FontClient::Plugin::ClearFallbackCache( std::vector<FallbackCacheItem>& fallbackCache )
{
  for( auto& item : fallbackCache )
  {
    if( nullptr != item.fallbackFonts )
    {
      delete item.fallbackFonts;
    }

    if( nullptr != item.characterSets )
    {
      // Free the resources allocated by the FcCharSet objects in the 'characterSets' vector.
      DestroyCharacterSets( *item.characterSets );
      delete item.characterSets;
    }
  }
}

void FontClient::Plugin::ClearCharacterSetFromFontFaceCache()
{
  for( auto& item : mFontFaceCache )
  {
    FcCharSetDestroy( item.mCharacterSet );
    item.mCharacterSet = nullptr;
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
