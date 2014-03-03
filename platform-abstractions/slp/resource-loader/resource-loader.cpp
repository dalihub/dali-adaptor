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

// CLASS HEADER
#include "resource-loader.h"

// EXTERNAL HEADERS
#include <boost/thread.hpp>
#include <iostream>
#include <fstream>
#include <set>
#include <queue>

// INTERNAL HEADERS
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/resource-cache.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/math/vector2.h>
#include "resource-requester-base.h"
#include "resource-bitmap-requester.h"
#include "resource-model-requester.h"
#include "resource-shader-requester.h"
#include "resource-text-requester.h"
#include "debug/resource-loader-debug.h"
#include "loader-font.h"
#include "../interfaces/font-controller.h"
#include "../interfaces/data-cache.h"


/**
 * A macro to expand an argument to a compile time constant string literal.
 * Wrapping the stringify in an outer macro, means that any macro passed as
 * "x" will be expanded before being turned into a string.
 * Use this for example to turn the current line number into a string:
 *   puts("The current line number is " DALI_TO_STRING(__LINE__) ".");
 */
#define DALI_TO_STRING_INNER(x) #x
#define DALI_TO_STRING(x) DALI_TO_STRING_INNER(x)

using namespace Dali::Integration;
using boost::mutex;
using boost::unique_lock;
using boost::scoped_ptr;

namespace Dali
{

namespace SlpPlatform
{

namespace
{

const char * const DALI_USER_FONT_CACHE_PATH( DALI_USER_FONT_CACHE_DIR );
const unsigned int MAX_NUMBER_CHARS_TO_CACHE( 60000 );  ///< support up to 60,000 glyphs
const unsigned int DISTANCE_FIELD_SIZE = 64;       // doesn't need to be power of two (the atlas may for performance)
const unsigned int DISTANCE_FIELD_PADDING = 30;    // Number of pixels of padding around the source FreeType bitmap
const unsigned int HIGH_QUALITY_PIXEL_SIZE = 200;  // Pixel size sent to FreeType2 FT_Set_Char_Size() for high quality glyphs
const float ONE_OVER_64 = 1.0f/64.0f;

#ifdef DEBUG_ENABLED
// For DEBUG_ENABLED profiling of distance field glyph generation
double GetTimeMicroseconds()
{
  timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  double seconds = time.tv_sec;
  seconds += 1e-3 * time.tv_nsec;
  return seconds;
}
#endif

} // unnamed namespace


struct ResourceLoader::ResourceLoaderImpl
{
  typedef std::pair<ResourceId, ResourceRequest>  RequestStorePair;
  typedef std::map<ResourceId, ResourceRequest>   RequestStore;
  typedef RequestStore::iterator                  RequestStoreIter;

  typedef std::queue<LoadedResource> LoadedQueue;
  typedef std::queue<SavedResource>  SavedQueue;
  typedef std::queue<FailedResource> FailedQueue;

  typedef std::pair<ResourceTypeId, ResourceRequesterBase*> RequestHandlerPair;
  typedef std::map<ResourceTypeId,  ResourceRequesterBase*> RequestHandlers;
  typedef RequestHandlers::iterator                         RequestHandlersIter;

  boost::mutex mQueueMutex;             ///< used to synchronize access to mLoadedQueue, mSavedQueue and mFailedQueue
  LoadedQueue  mPartiallyLoadedQueue;   ///< Partially complete load requests notifications are stored here until fetched by core
  LoadedQueue  mLoadedQueue;            ///< Completed load requests notifications are stored here until fetched by core
  SavedQueue   mSavedQueue;             ///< Completed save request notifications are stored here until fetched by core
  FailedQueue  mFailedLoads;            ///< Failed load request notifications are stored here until fetched by core
  FailedQueue  mFailedSaves;            ///< Failed save request notifications are stored here until fetched by core

  Dali::Platform::FontController* mFontController;       ///< Interface for accessing font information

  RequestHandlers mRequestHandlers;
  RequestStore mStoredRequests;         ///< Used to store load requests until loading is completed

  ResourceLoaderImpl( ResourceLoader* loader )
  {
    mFontController = Dali::Platform::FontController::New();

    mRequestHandlers.insert(std::make_pair(ResourceBitmap, new ResourceBitmapRequester(*loader)));
    mRequestHandlers.insert(std::make_pair(ResourceShader, new ResourceShaderRequester(*loader)));
    mRequestHandlers.insert(std::make_pair(ResourceModel, new ResourceModelRequester(*loader)));
    mRequestHandlers.insert(std::make_pair(ResourceText, new ResourceTextRequester(*loader)));
  }

  ~ResourceLoaderImpl()
  {
    // Delete resource handlers
    for( RequestHandlersIter it = mRequestHandlers.begin(); it != mRequestHandlers.end(); ++it )
    {
      ResourceRequesterBase* requestBase = it->second;
      delete requestBase;
    }

    delete mFontController;
  }

  void Pause()
  {
    // Pause all the request handlers:
    for( RequestHandlersIter it = mRequestHandlers.begin(), end = mRequestHandlers.end(); it != end;  ++it )
    {
      ResourceRequesterBase * const requester = it->second;
      if( requester )
      {
        requester->Pause();
      }
    }
  }

  void Resume()
  {
    // Wake up all the request handlers:
    for( RequestHandlersIter it = mRequestHandlers.begin(), end = mRequestHandlers.end(); it != end;  ++it )
    {
      ResourceRequesterBase * const requester = it->second;
      if( requester )
      {
        requester->Resume();
      }
    }
  }

  ResourceRequesterBase* GetRequester(ResourceTypeId typeId)
  {
    ResourceRequesterBase* requestHandler = NULL;
    RequestHandlersIter iter = mRequestHandlers.find(typeId);
    if(iter != mRequestHandlers.end())
    {
      requestHandler = iter->second;
    }
    DALI_ASSERT_DEBUG(requestHandler && "All resource types should have a requester defined for them.");
    return requestHandler;
  }

  void LoadResource(const ResourceRequest& request)
  {
    // Store resource request for partial loaders. Will get cleaned up after load complete has finished
    StoreRequest(request);

    ResourceRequesterBase* requester = GetRequester(request.GetType()->id);
    if( requester )
    {
      ResourceRequest* storedRequest = GetRequest(request.GetId());
      if( storedRequest != NULL )
      {
        requester->LoadResource(*storedRequest); // Pass in stored request
      }
    }
    else
    {
      DALI_LOG_ERROR( "Unknown resource type (%u) with path \"%s\" in load request.\n", request.GetType()->id, request.GetPath().c_str() );
      DALI_ASSERT_DEBUG( 0 == "Unknown resource type in load request at " __FILE__ ", line " DALI_TO_STRING(__LINE__) ".\n" );
    }
  }

  void SaveResource(const ResourceRequest& request)
  {
    ResourceRequesterBase* requester = GetRequester( request.GetType()->id );
    if( requester )
    {
      requester->SaveResource( request );
    }
  }

  void CancelLoad(ResourceId id, ResourceTypeId typeId)
  {
    ResourceRequesterBase* requester = GetRequester(typeId);
    if( requester )
    {
      requester->CancelLoad( id, typeId );
    }
    ClearRequest( id );
  }

  LoadStatus LoadFurtherResources( LoadedResource partialResource )
  {
    LoadStatus loadStatus = RESOURCE_LOADING;
    RequestStoreIter iter = mStoredRequests.find(partialResource.id);

    if( mStoredRequests.end() != iter ) // else cancelled. Ignore response
    {
      ResourceRequest& request = iter->second;
      ResourceRequesterBase* requester = GetRequester(request.GetType()->id);
      if( requester )
      {
        loadStatus = requester->LoadFurtherResources( request, partialResource );
      }

      DALI_LOG_INFO(gLoaderFilter, Debug::General, "ResourceLoader::LoadFurtherResources( ID:%u complete: %s)\n",  request.GetId(), loadStatus==RESOURCE_LOADING?"Loading":loadStatus==RESOURCE_PARTIALLY_LOADED?"PARTIAL":"COMPLETE" );
    }

    if( loadStatus == RESOURCE_COMPLETELY_LOADED )
    {
      ClearRequest( partialResource.id );
    }

    return loadStatus;
  }

  bool IsLoading()
  {
    // TODO - not used - remove?
    return true;
  }

  void GetResources(ResourceCache& cache)
  {
    // Fill the resource cache

    unique_lock<mutex> lock(mQueueMutex);

    // iterate through the partially loaded resources
    while (!mPartiallyLoadedQueue.empty())
    {
      LoadedResource loaded( mPartiallyLoadedQueue.front() );
      mPartiallyLoadedQueue.pop();
      LoadStatus loadStatus = LoadFurtherResources( loaded );
      cache.LoadResponse( loaded.id, loaded.type, loaded.resource, loadStatus );
    }

    // iterate through the successfully loaded resources
    while (!mLoadedQueue.empty())
    {
      LoadedResource loaded( mLoadedQueue.front() );
      mLoadedQueue.pop();
      ClearRequest( loaded.id );
      cache.LoadResponse( loaded.id, loaded.type, loaded.resource, RESOURCE_COMPLETELY_LOADED );
    }

    // iterate through the successfully saved resources
    while (!mSavedQueue.empty())
    {
      SavedResource saved(mSavedQueue.front());
      mSavedQueue.pop();
      cache.SaveComplete(saved.id, saved.type);
    }

    // iterate through the resources which failed to load
    while (!mFailedLoads.empty())
    {
      FailedResource failed(mFailedLoads.front());
      mFailedLoads.pop();
      ClearRequest(failed.id);
      cache.LoadFailed(failed.id, failed.failureType);
    }

    // iterate through the resources which failed to save
    while (!mFailedSaves.empty())
    {
      FailedResource failed(mFailedSaves.front());
      mFailedSaves.pop();
      cache.SaveFailed(failed.id, failed.failureType);
    }
  }

  void LoadImageMetadata( const std::string fileName, Vector2 &size )
  {
    ResourceRequesterBase* requester = GetRequester(ResourceBitmap);
    ResourceBitmapRequester* bitmapRequester = dynamic_cast<ResourceBitmapRequester*>(requester);
    if( bitmapRequester != NULL )
    {
      bitmapRequester->LoadImageMetadata( fileName, size );
    }
  }

  void AddPartiallyLoadedResource( LoadedResource& resource)
  {
    // Lock the LoadedQueue to store the loaded resource
    unique_lock<mutex> lock(mQueueMutex);

    mPartiallyLoadedQueue.push( resource );
  }

  void AddLoadedResource(LoadedResource& resource)
  {
    // Lock the LoadedQueue to store the loaded resource
    unique_lock<mutex> lock(mQueueMutex);

    mLoadedQueue.push( resource );
  }

  void AddSavedResource(SavedResource& resource)
  {
    // Lock the SavedQueue to store the loaded resource
    unique_lock<mutex> lock(mQueueMutex);

    mSavedQueue.push(resource);
  }

  void AddFailedLoad(FailedResource& resource)
  {
    // Lock the FailedQueue to store the failed resource information
    unique_lock<mutex> lock(mQueueMutex);

    mFailedLoads.push(resource);
  }

  void AddFailedSave(FailedResource& resource)
  {
    // Lock the FailedQueue to store the failed resource information
    unique_lock<mutex> lock(mQueueMutex);

    mFailedSaves.push(resource);
  }

  void StoreRequest( const ResourceRequest& request )
  {
    DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader: StoreRequest(id:%u)\n", request.GetId());
    mStoredRequests.insert( RequestStorePair( request.GetId(), request ) ); // copy request as value type
  }

  ResourceRequest* GetRequest( ResourceId id )
  {
    ResourceRequest* found(NULL);
    DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader: GetRequest(id:%u)\n", id);
    RequestStoreIter iter = mStoredRequests.find( id );
    if( mStoredRequests.end() != iter )
    {
      found = &iter->second;
    }
    return found;
  }

  void ClearRequest( ResourceId resourceId )
  {
    DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader: ClearRequest(id:%u)\n", resourceId);
    RequestStoreIter iter = mStoredRequests.find( resourceId );
    if( mStoredRequests.end() != iter ) // Can't assert here - cancel load may cross with load failed
    {
      mStoredRequests.erase( iter );
    }
  }
};

/********************************************************************************/
/****************************   RESOURCE LOADER METHODS  ************************/
/********************************************************************************/
ResourceLoader::ResourceLoader()
: mTerminateThread(0)
{
  mImpl = new ResourceLoaderImpl( this );
}

ResourceLoader::~ResourceLoader()
{
  // Flag that the ResourceLoader is exiting
  (void)__sync_or_and_fetch( &mTerminateThread, -1 );

  delete mImpl;
}

void ResourceLoader::Pause()
{
  mImpl->Pause();
}

void ResourceLoader::Resume()
{
  mImpl->Resume();
}

bool ResourceLoader::IsTerminating()
{
  return __sync_fetch_and_or( &mTerminateThread, 0 );
}

void ResourceLoader::GetResources(ResourceCache& cache)
{
  mImpl->GetResources( cache );
}

/********************************************************************************/
/**************************   CALLED FROM LOADER THREADS   **********************/
/********************************************************************************/

void ResourceLoader::AddPartiallyLoadedResource( LoadedResource& resource)
{
  mImpl->AddPartiallyLoadedResource( resource );
}

void ResourceLoader::AddLoadedResource(LoadedResource& resource)
{
  mImpl->AddLoadedResource( resource );
}

void ResourceLoader::AddSavedResource(SavedResource& resource)
{
  mImpl->AddSavedResource( resource );
}

void ResourceLoader::AddFailedLoad(FailedResource& resource)
{
  mImpl->AddFailedLoad( resource );
}

void ResourceLoader::AddFailedSave(FailedResource& resource)
{
  mImpl->AddFailedSave( resource );
}

/********************************************************************************/
/*********************   CALLED FROM PLATFORM ABSTRACTION  **********************/
/********************************************************************************/

void ResourceLoader::LoadResource(const ResourceRequest& request)
{
  mImpl->LoadResource(request);
}

void ResourceLoader::SaveResource(const ResourceRequest& request)
{
  mImpl->SaveResource(request);
}

void ResourceLoader::CancelLoad(ResourceId id, ResourceTypeId typeId)
{
  mImpl->CancelLoad(id, typeId);
}

bool ResourceLoader::IsLoading()
{
  return mImpl->IsLoading();
}

void ResourceLoader::LoadImageMetadata(const std::string fileName, Vector2 &size)
{
  mImpl->LoadImageMetadata( fileName, size );
}

std::string ResourceLoader::GetFontFamilyForChars(const TextArray& charsRequested)
{
  return mImpl->mFontController->GetFontFamilyForChars( charsRequested ).first;
}

bool ResourceLoader::AllGlyphsSupported(const std::string &fontFamily, const std::string &fontStyle, const TextArray& charsRequested)
{
  return mImpl->mFontController->AllGlyphsSupported( Platform::FontController::StyledFontFamily( fontFamily, fontStyle ), charsRequested);

}

bool ResourceLoader::ValidateFontFamilyName(const std::string& fontFamily, const std::string& fontStyle, bool& isDefaultSystemFont, std::string& closestFontFamilyMatch, std::string& closestFontStyleMatch)
{
  Platform::FontController::StyledFontFamily closestMatch;

  bool result = mImpl->mFontController->ValidateFontFamilyName( Platform::FontController::StyledFontFamily( fontFamily, fontStyle ), isDefaultSystemFont, closestMatch);

  closestFontFamilyMatch = closestMatch.first;
  closestFontStyleMatch = closestMatch.second;

  return result;
}

const PixelSize ResourceLoader::GetFontLineHeightFromCapsHeight(const std::string fontFamily, const std::string& fontStyle, const CapsHeight& capsHeight, FT_Library freeType)
{
  PixelSize result(0);

  if (!fontFamily.empty())
  {
    std::string fontFileName = GetFontPath( fontFamily, fontStyle );
    SlpFace* slpFace = LoadFontFace(fontFileName, PixelSize(capsHeight), freeType);

    if (slpFace)
    {
      const float scale = static_cast<float>(capsHeight.value) / ((slpFace->face->ascender / 64.0f) * 0.95f);

      result.value = static_cast<unsigned int>(roundf(scale * (slpFace->face->height / 64.0f)));

      delete slpFace;
      slpFace = NULL;
    }
  }

  return result;
}

std::vector<std::string> ResourceLoader::GetFontList( Dali::Integration::PlatformAbstraction::FontListMode mode )
{
  std::vector<std::string> result;
  std::set<std::string> uniqueFontNames;

  // VCC TODO: A GetStyles() method which returns a list of styles for a given font family is needed.

  Platform::FontController::FontList fontList;
  Platform::FontController::FontListMode listMode;

  switch( mode )
  {
    case Dali::Integration::PlatformAbstraction::LIST_ALL_FONTS:
    {
      listMode =  Platform::FontController::LIST_ALL_FONTS;
      break;
    }
    case Dali::Integration::PlatformAbstraction::LIST_SYSTEM_FONTS:
    {
      listMode =  Platform::FontController::LIST_SYSTEM_FONTS;
      break;
    }
    case Dali::Integration::PlatformAbstraction::LIST_APPLICATION_FONTS:
    {
      listMode =  Platform::FontController::LIST_APPLICATION_FONTS;
      break;
    }
    default:
    {
      DALI_ASSERT_DEBUG(0 && "invalid mode");
      return result;
    }
  }

  fontList = mImpl->mFontController->GetFontList( listMode );

  for( Platform::FontController::FontList::const_iterator it = fontList.begin(), endIt = fontList.end(); it != endIt; ++it )
  {
    uniqueFontNames.insert(it->first);
  }

  // copy into a vector
  std::copy(uniqueFontNames.begin(), uniqueFontNames.end(), std::back_inserter(result));

  return result;
}


/**
 * Note, called from both platform abstraction & from text loader threads
 **/
GlyphSet* ResourceLoader::GetGlyphData (const TextResourceType& textRequest,
                                        FT_Library freeType,
                                        const std::string& fontFamily,
                                        bool getBitmap)
{
  GlyphSet* glyphSet = NULL;

  size_t fontHash = textRequest.mFontHash;

  DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "LoadGlyphSet - requested string is %d characters long\n", textRequest.mCharacterList.size());

  // path holds the font name
  if( !fontFamily.empty() )
  {
    std::string fontFileName = GetFontPath( fontFamily, textRequest.mStyle );

    const bool highQuality(textRequest.mQuality == TextResourceType::TextQualityHigh);
    const unsigned int glyphQuality( highQuality ? GlyphMetrics::HIGH_QUALITY : GlyphMetrics::LOW_QUALITY );

    SlpFace* slpFace = LoadFontFace( fontFileName, PixelSize( HIGH_QUALITY_PIXEL_SIZE), freeType );
    if (slpFace)
    {
      glyphSet = new GlyphSet();
      glyphSet->mFontHash = fontHash;
      glyphSet->SetAtlasResourceId( textRequest.mTextureAtlasId );

      for( TextResourceType::CharacterList::const_iterator it = textRequest.mCharacterList.begin(), endIt = textRequest.mCharacterList.end(); it != endIt; ++it )
      {
        uint32_t charCode( it->character );

        if (!glyphSet->HasCharacter(charCode))        // ignore duplicate glyphs in the request
        {
#ifdef DEBUG_ENABLED
          // DEBUG_ENABLED profiling of distance field glyph generation
          double then( 0.0 );
          if( getBitmap )
          {
            then = GetTimeMicroseconds();
          }
#endif
          scoped_ptr< GlyphSet::Character > character( GetCharacter(slpFace->face, charCode,
                                                                    DISTANCE_FIELD_SIZE, DISTANCE_FIELD_PADDING, textRequest.mMaxGlyphSize,
                                                                    getBitmap, highQuality ) );

#ifdef DEBUG_ENABLED
          // DEBUG_ENABLED profiling of distance field glyph generation
          if( getBitmap )
          {
            double now( GetTimeMicroseconds() );

            DALI_LOG_INFO( gLoaderFilter, Log::Verbose, "Generating (%c) in %s quality took %.3f ms\n", charCode, highQuality ? "high" : "low",  1e-3 * ( now - then ) );
          }
#endif
          if (character)
          {
            GlyphSet::Character& glyphCharacter( *character.get() );

            glyphCharacter.second.quality = glyphQuality;
            glyphCharacter.second.xPosition = it->xPosition;
            glyphCharacter.second.yPosition = it->yPosition;
            // copy character to GlyphSet
            glyphSet->AddCharacter( glyphCharacter );
          }
        }
      }

      delete slpFace;
    }
  }

  return glyphSet;
}

GlyphSet* ResourceLoader::GetCachedGlyphData(const TextResourceType& textRequest, const std::string& fontFamily)
{
  GlyphSet* glyphSet( new GlyphSet() );
  glyphSet->mFontHash = textRequest.mFontHash;
  glyphSet->SetAtlasResourceId(textRequest.mTextureAtlasId);

  std::string cachePath(DALI_USER_FONT_CACHE_PATH);
  cachePath.append(fontFamily + "-" + textRequest.mStyle);
  std::replace(cachePath.begin(), cachePath.end(), ' ', '-');

  DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader::GetCachedGlyphData() - cachefile: %s\n", cachePath.c_str() );

  Platform::DataCache* dataCache = Platform::DataCache::New( Platform::DataCache::READ_ONLY,
                                                             Platform::DataCache::RUN_LENGTH_ENCODING,
                                                             cachePath,
                                                             DISTANCE_FIELD_SIZE * DISTANCE_FIELD_SIZE,
                                                             MAX_NUMBER_CHARS_TO_CACHE);

  Platform::DataCache::KeyVector keyVector;
  Platform::DataCache::DataVector dataVector;

  const TextResourceType::CharacterList& requestedCharacters = textRequest.mCharacterList;
  for( std::size_t i=0, length = requestedCharacters.size(); i < length; ++i )
  {
    keyVector.push_back( requestedCharacters[i].character );
  }

  // load the glyphs from file
  dataCache->Find( keyVector, dataVector );

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
      BitmapPtr bitmapData ( Integration::Bitmap::New(Bitmap::BITMAP_2D_PACKED_PIXELS, true) );
      DALI_ASSERT_ALWAYS( data.length == DISTANCE_FIELD_SIZE * DISTANCE_FIELD_SIZE );

      // assign the data
      bitmapData->GetPackedPixelsProfile()->AssignBuffer( Pixel::A8, data.data, DISTANCE_FIELD_SIZE * DISTANCE_FIELD_SIZE, DISTANCE_FIELD_SIZE, DISTANCE_FIELD_SIZE );

      data.data = NULL;

      // add to the glyphset
      glyphSet->AddCharacter( bitmapData, glyphMetrics );
    }
  }
  DALI_LOG_INFO( gLoaderFilter, Debug::Verbose, "ResourceLoader::GetCachedGlyphData() - requestedGlyphs:%u, cachedGlyphs:%u\n",
                 requestedCharacters.size(), glyphSet->GetCharacterList().size() );

  delete dataCache;

  return glyphSet;
}

void ResourceLoader::GetGlobalMetrics( FT_Library freeType,
                                       const std::string& fontFamily,
                                       const std::string& fontStyle,
                                       GlobalMetrics& globalMetrics )
{
  // path holds the font name
  if( !fontFamily.empty() )
  {
    std::string fontFileName = GetFontPath( fontFamily, fontStyle );

    SlpFace* slpFace = LoadFontFace( fontFileName, PixelSize( HIGH_QUALITY_PIXEL_SIZE), freeType );
    if( slpFace )
    {
      // scale factor for unit scaled glyphs
      const float xScale = 1.0f / (slpFace->face->size->metrics.x_scale / 65536.0f);
      const float yScale = 1.0f / (slpFace->face->size->metrics.y_scale / 65536.0f);

      globalMetrics.lineHeight = slpFace->face->height * ONE_OVER_64;
      globalMetrics.ascender = slpFace->face->ascender * ONE_OVER_64;
      globalMetrics.unitsPerEM = slpFace->face->units_per_EM * ONE_OVER_64;

      globalMetrics.underlinePosition = -4.f;
      globalMetrics.underlineThickness = 5.f * yScale;
      if( 1.f > globalMetrics.underlineThickness )
      {
        globalMetrics.underlineThickness = 1.f;
      }
      globalMetrics.maxWidth = DISTANCE_FIELD_SIZE * xScale;
      globalMetrics.maxHeight = DISTANCE_FIELD_SIZE * yScale;
      globalMetrics.padAdjustX = DISTANCE_FIELD_PADDING * xScale;
      globalMetrics.padAdjustY = DISTANCE_FIELD_PADDING * yScale;

      delete slpFace;
    }
  }
}

void ResourceLoader::SetDpi(unsigned int dpiHor, unsigned int dpiVer)
{
  // Unused
}

bool ResourceLoader::LoadFile( const std::string& filename, std::vector< unsigned char >& buffer ) const
{
  DALI_LOG_TRACE_METHOD(gLoaderFilter);

  DALI_ASSERT_DEBUG( 0 != filename.length());

  bool result;

  std::filebuf buf;
  buf.open(filename.c_str(), std::ios::in | std::ios::binary);
  if( buf.is_open() )
  {
    std::istream stream(&buf);

    // determine data length
    stream.seekg(0, std::ios_base::end);
    unsigned int length = static_cast<unsigned int>( stream.tellg() );
    stream.seekg(0, std::ios_base::beg);

    // allocate a buffer
    buffer.resize(length);
    // read data into buffer
    stream.read(reinterpret_cast<char*>(buffer.data()), length);

    DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader::LoadFile(%s) - loaded %d bytes\n", filename.c_str(), length);

    result = true;
  }
  else
  {
    DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader::LoadFile(%s) - failed to load\n", filename.c_str());
    result = false;
  }

  return result;
}

std::string ResourceLoader::LoadFile(const std::string& filename) const
{
  DALI_LOG_TRACE_METHOD(gLoaderFilter);

  DALI_ASSERT_DEBUG( 0 != filename.length());

  std::string contents;

  std::filebuf buf;
  buf.open(filename.c_str(), std::ios::in);
  if( buf.is_open() )
  {
    std::istream stream(&buf);

    // determine data length
    stream.seekg(0, std::ios_base::end);
    unsigned int length = static_cast<unsigned int>( stream.tellg() );
    stream.seekg(0, std::ios_base::beg);

    // allocate a buffer
    contents.resize(length);
    // read data into buffer
    stream.read(&contents[0], length);

    DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader::LoadFile(%s) - loaded %d bytes\n", filename.c_str(), length);
  }
  else
  {
    DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader::LoadFile(%s) - failed to load\n", filename.c_str());
  }

  return contents;
}

bool ResourceLoader::SaveFile(const std::string& filename, std::vector< unsigned char >& buffer)
{
  DALI_LOG_TRACE_METHOD(gLoaderFilter);

  DALI_ASSERT_DEBUG( 0 != filename.length());

  bool result = false;

  std::filebuf buf;
  buf.open(filename.c_str(), std::ios::out | std::ios_base::trunc | std::ios::binary);
  if( buf.is_open() )
  {
    std::ostream stream(&buf);

    // determine size of buffer
    int length = static_cast<int>(buffer.size());

    // write contents of buffer to the file
    stream.write(reinterpret_cast<char*>(buffer.data()), length);

    if( !stream.bad() )
    {
      DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader::SaveFile(%s) - wrote %d bytes\n", filename.c_str(), length);
      result = true;
    }
  }

#if defined(DEBUG_BUILD)
  if( !result )
  {
    DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader::SaveFile(%s) - failed to load\n", filename.c_str());
  }
#endif

  return result;
}

void ResourceLoader::SetDefaultFontFamily( const std::string& fontFamily, const std::string& fontStyle )
{
  mImpl->mFontController->SetDefaultFontFamily( Platform::FontController::StyledFontFamily( fontFamily, fontStyle ) );
}

std::string ResourceLoader::GetFontPath(const std::string& fontFamily, const std::string& fontStyle)
{
  return mImpl->mFontController->GetFontPath(std::make_pair(fontFamily,fontStyle));
}

} // namespace SlpPlatform

} // namespace Dali
