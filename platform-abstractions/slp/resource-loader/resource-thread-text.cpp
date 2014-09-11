/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "resource-thread-text.h"

// INTERNAL INCLUDES
#include "loader-font.h"
#include "../interfaces/data-cache.h"
#include "../data-cache/tests/data-cache-debug.h"

// EXTERNAL INCLUDES
#include <boost/functional/hash.hpp>
#include <ft2build.h>
#include <iostream>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <dali/integration-api/glyph-set.h>
#include <dali/integration-api/platform-abstraction.h>


using namespace Dali::Integration;

namespace Dali
{
namespace SlpPlatform
{

namespace
{

const char * const DALI_USER_FONT_CACHE_PATH( DALI_USER_FONT_CACHE_DIR );
const unsigned int DISTANCE_FIELD_WIDTH( 64 );
const unsigned int DISTANCE_FIELD_HEIGHT( 64 );
const unsigned int DISTANCE_FIELD_SIZE(DISTANCE_FIELD_WIDTH * DISTANCE_FIELD_HEIGHT);
const unsigned int MAX_NUMBER_CHARS_TO_CACHE( 60000 );  ///< support up to 60,000 glyphs

boost::mutex mFileCheckMutex;             ///< used to ensure only one thread checks the files for corruption on startup


}; // unnamed namespace

ResourceThreadText::ResourceThreadText(ResourceLoader& resourceLoader, const TextResourceType::TextQuality quality)
: ResourceThreadBase(resourceLoader),
  mQuality(quality),
  mFreeTypeHandle( NULL )
{

#ifdef DATA_CACHE_DEBUG
  DataCacheIo::ThreadedStressTest();
#endif
}

ResourceThreadText::~ResourceThreadText()
{
  TerminateThread();

  DeleteDataCaches();

  if( mFreeTypeHandle )
  {
    FT_Done_FreeType(mFreeTypeHandle);
  }
}

void ResourceThreadText::Load(const ResourceRequest& request)
{
  // 1: Check if glyphs are in the data cache
  // 2: For any missing glyphs, load from FreeType and save to cache file.
  //
  // Important Note: FreeType will never fail to load a character, if the character code
  // is not part of the font it will render a 'default glyph'.
  //
  // This means we never call mResourceLoader.AddFailedLoad( ...)

  const TextResourceType& textRequest = dynamic_cast<const TextResourceType&>(*request.GetType());
  TextResourceType::CharacterList text(textRequest.mCharacterList);

  std::string cachePath(DALI_USER_FONT_CACHE_PATH);
  cachePath.append(request.GetPath() + "-" + textRequest.mStyle);
  std::replace(cachePath.begin(), cachePath.end(), ' ', '-');

  Platform::DataCache* dataCache = GetDataCache( cachePath );

  GlyphSet *glyphSet( new GlyphSet() );
  glyphSet->mFontHash = textRequest.mFontHash;
  glyphSet->SetAtlasResourceId(textRequest.mTextureAtlasId);

  if( textRequest.mCache == TextResourceType::GLYPH_CACHE_READ )
  {
    LoadCharactersFromCache( *dataCache, *glyphSet, text );
  }
  else
  {
    LoadCharacters( *dataCache, *glyphSet, textRequest, request.GetPath(), text );
  }

  IntrusivePtr<GlyphSet> glyphResource( glyphSet );
  LoadedResource resource( request.GetId(), request.GetType()->id, glyphResource );
  mResourceLoader.AddPartiallyLoadedResource(resource);
}

void ResourceThreadText::Save(const ResourceRequest& request)
{
}

void ResourceThreadText::LoadCharactersFromCache(
  Platform::DataCache& dataCache,
  GlyphSet& glyphSet,
  const TextResourceType::CharacterList& requestedCharacters )
{
  Platform::DataCache::KeyVector keyVector;
  for( std::size_t i=0, length=requestedCharacters.size(); i<length; i++ )
  {
    keyVector.push_back(requestedCharacters[i].character);
  }
  Platform::DataCache::DataVector dataVector;

  // load the glyphs from file
  dataCache.Find( keyVector, dataVector );


  // for each glyph found, add to the glyph set
  for( std::size_t n = 0, arraySize = keyVector.size(); n < arraySize ; n++ )
  {
    Platform::DataCache::Data& data( dataVector[n]);

    if( data.exists )
    {
      GlyphMetrics glyphMetrics;
      glyphMetrics.code = keyVector[ n ];
      glyphMetrics.quality = GlyphMetrics::HIGH_QUALITY;
      glyphMetrics.xPosition = requestedCharacters[n].xPosition;
      glyphMetrics.yPosition = requestedCharacters[n].yPosition;

      // create a new bitmap, and copy in the data
      BitmapPtr bitmapData ( Integration::Bitmap::New(Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD) );
      DALI_ASSERT_ALWAYS( data.length == DISTANCE_FIELD_SIZE );

      // assign the data
      bitmapData->GetPackedPixelsProfile()->AssignBuffer( Pixel::A8, data.data, DISTANCE_FIELD_SIZE, DISTANCE_FIELD_WIDTH, DISTANCE_FIELD_HEIGHT );

      data.data = NULL;

      // add to the glyphset
      glyphSet.AddCharacter( bitmapData, glyphMetrics );
    }
  }
}

void ResourceThreadText::SaveCharacters( Platform::DataCache& dataCache, const GlyphSet& glyphSet )
{
  const GlyphSet::CharacterList& chars = glyphSet.GetCharacterList();

  Platform::DataCache::KeyVector keyVector;
  Platform::DataCache::DataVector dataVector;

  keyVector.reserve( chars.size() );
  dataVector.reserve( chars.size() );

  for( GlyphSet::CharacterList::const_iterator it = chars.begin(), endIt = chars.end(); it != endIt; ++it )
  {
    const BitmapPtr& bitmap( it->first );
    if ( bitmap )
    {
      const GlyphMetrics& metrics( it->second );

      Platform::DataCache::DataKey key( metrics.code );
      Platform::DataCache::Data data;
      data.SetData( bitmap->GetBuffer(), DISTANCE_FIELD_SIZE );

      keyVector.push_back( key );
      dataVector.push_back( data );
    }
  }

  // this will save the distance fields to file
  dataCache.Add( keyVector, dataVector );

}

void ResourceThreadText::LoadCharacters( Platform::DataCache& dataCache,
                                         GlyphSet& glyphSet,
                                         const TextResourceType& textRequest,
                                         const std::string& path,
                                         const TextResourceType::CharacterList& charCodes )
{
  // we need to use freetype, ensure the library has been initialised
  LoadFreeType();

  // baseline and other general info still queried from FreeType even if all characters were found in cache
  GlyphSet* missingGlyphSet = mResourceLoader.GetGlyphData(textRequest, mFreeTypeHandle, path, true);

  const GlyphSet::CharacterList& cachedChars = missingGlyphSet->GetCharacterList();
  for( GlyphSet::CharacterList::const_iterator it = cachedChars.begin(), endIt = cachedChars.end(); it != endIt; ++it )
  {
    glyphSet.AddCharacter( *it );
  }

  // save to cache if high quality
  if( ( mQuality == TextResourceType::TextQualityHigh ) && ( TextResourceType::GLYPH_CACHE_WRITE == textRequest.mCache ) )
  {
    SaveCharacters( dataCache, glyphSet );
  }

  // all the information in missingGlyphSet has been copied, so delete it.
  delete missingGlyphSet;
}

void ResourceThreadText::LoadFreeType()
{
  if( mFreeTypeHandle )
  {
    return;
  }

  if( FT_Init_FreeType( &mFreeTypeHandle ) != 0 )
  {
    DALI_ASSERT_ALWAYS( 0 && "FT_Init_FreeType failed\n");
  }
}

Platform::DataCache* ResourceThreadText::CreateDataCache( const std::string& fileName )
{
  // low quality text thread just reads from the cache
  Platform::DataCache::ReadWriteMode readWriteMode( Platform::DataCache::READ_ONLY);

  // allow high quality distance fields to be written to the cache
  if( mQuality == TextResourceType::TextQualityHigh )
  {
    readWriteMode = Platform::DataCache::READ_WRITE;
  }

  return Platform::DataCache::New( readWriteMode,
                                          Platform::DataCache::RUN_LENGTH_ENCODING,
                                          fileName,
                                          DISTANCE_FIELD_SIZE,
                                          MAX_NUMBER_CHARS_TO_CACHE);
}

Platform::DataCache* ResourceThreadText::GetDataCache( const std::string& fileName )
{
  boost::hash<const std::string> hasher;
  std::size_t hashValue = hasher( fileName );

  // most applications use under 3 fonts, so a simple vector to do the lookup is fine
  for( unsigned int i = 0; i< mDataCaches.size() ; i++ )
  {
    if( mDataCaches[i].second == hashValue)
    {
      return mDataCaches[i].first;
    }
  }
  // not found in lookup, create a new data cache

  Platform::DataCache* newCache = CreateDataCache( fileName );
  mDataCaches.push_back( TDataCachePair( newCache, hashValue ) );
  return newCache;
}

void ResourceThreadText::DeleteDataCaches()
{
  for( unsigned int i = 0; i< mDataCaches.size() ; i++ )
  {
    delete mDataCaches[i].first;
  }
  mDataCaches.clear();
}

} // namespace SlpPlatform
} // namespace Dali
