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
#include "resource-loader.h"

// EXTERNAL HEADERS
#include <iostream>
#include <fstream>
#include <queue>
#include <cstring>
#include <dali/devel-api/common/map-wrapper.h>
#include <dali/devel-api/threading/mutex.h>

// INTERNAL HEADERS
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/devel-api/common/set-wrapper.h>
#include <dali/public-api/math/vector2.h>
#include "resource-requester-base.h"
#include "resource-bitmap-requester.h"
#include "debug/resource-loader-debug.h"

using namespace Dali::Integration;

namespace Dali
{

namespace TizenPlatform
{

struct ResourceLoader::ResourceLoaderImpl
{
  typedef std::queue<LoadedResource> LoadedQueue;
  typedef std::queue<FailedResource> FailedQueue;

  typedef std::pair<ResourceTypeId, ResourceRequesterBase*> RequestHandlerPair;
  typedef std::map<ResourceTypeId,  ResourceRequesterBase*> RequestHandlers;
  typedef RequestHandlers::iterator                         RequestHandlersIter;

  Dali::Mutex  mQueueMutex;             ///< used to synchronize access to mLoadedQueue and mFailedQueue
  LoadedQueue  mLoadedQueue;            ///< Completed load requests notifications are stored here until fetched by core
  FailedQueue  mFailedLoads;            ///< Failed load request notifications are stored here until fetched by core

  RequestHandlers mRequestHandlers;

  ResourceLoaderImpl( ResourceLoader* loader )
  {
    mRequestHandlers.insert(std::make_pair(ResourceBitmap, new ResourceBitmapRequester(*loader)));
  }

  ~ResourceLoaderImpl()
  {
    // Delete resource handlers
    for( RequestHandlersIter it = mRequestHandlers.begin(); it != mRequestHandlers.end(); ++it )
    {
      ResourceRequesterBase* requestBase = it->second;
      delete requestBase;
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

  void AddLoadedResource(LoadedResource& resource)
  {
    // Lock the LoadedQueue to store the loaded resource
    Mutex::ScopedLock lock( mQueueMutex );

    mLoadedQueue.push( resource );
  }

  void AddFailedLoad(FailedResource& resource)
  {
    // Lock the FailedQueue to store the failed resource information
    Mutex::ScopedLock lock( mQueueMutex );

    mFailedLoads.push(resource);
  }


private:

  ResourceLoaderImpl( const ResourceLoaderImpl& ); ///< Undefined
  ResourceLoaderImpl& operator=( const ResourceLoaderImpl& ); ///< Undefined
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

bool ResourceLoader::IsTerminating()
{
  return __sync_fetch_and_or( &mTerminateThread, 0 );
}

/********************************************************************************/
/**************************   CALLED FROM LOADER THREADS   **********************/
/********************************************************************************/

void ResourceLoader::AddLoadedResource(LoadedResource& resource)
{
  mImpl->AddLoadedResource( resource );
}

void ResourceLoader::AddFailedLoad(FailedResource& resource)
{
  mImpl->AddFailedLoad( resource );
}

/********************************************************************************/
/*********************   CALLED FROM PLATFORM ABSTRACTION  **********************/
/********************************************************************************/


bool ResourceLoader::LoadFile( const std::string& filename, std::vector< unsigned char >& buffer ) const
{
  Dali::Vector<unsigned char> daliVec;
  const bool result = LoadFile( filename, daliVec );
  buffer.resize( daliVec.Size() );
  memcpy( &buffer[0], &daliVec[0], daliVec.Size() );
  return result;
}

bool ResourceLoader::LoadFile( const std::string& filename, Dali::Vector< unsigned char >& buffer ) const
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
    buffer.Resize(length);
    // read data into buffer
    stream.read(reinterpret_cast<char*>(buffer.Begin()), length);

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

bool ResourceLoader::SaveFile(const std::string& filename, const unsigned char * buffer, unsigned int numBytes )
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
    int length = static_cast<int>(numBytes);

    // write contents of buffer to the file
    stream.write(reinterpret_cast<const char*>(buffer), length);

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

} // namespace TizenPlatform

} // namespace Dali
