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

#include "resource-bitmap-requester.h"
#include <dali/integration-api/resource-cache.h>

using namespace Dali::Integration;

namespace Dali
{
namespace SlpPlatform
{

ResourceBitmapRequester::ResourceBitmapRequester( ResourceLoader& resourceLoader )
: ResourceRequesterBase( resourceLoader )
{
  mThreadImageLocal  = new ResourceThreadImage( resourceLoader, false );
  mThreadImageRemote  = new ResourceThreadImage( resourceLoader, true );
}

ResourceBitmapRequester::~ResourceBitmapRequester()
{
  delete mThreadImageLocal;
  delete mThreadImageRemote;
}

void ResourceBitmapRequester::Pause()
{
  mThreadImageLocal->Pause();
  mThreadImageRemote->Pause();
}

void ResourceBitmapRequester::Resume()
{
  mThreadImageLocal->Resume();
  mThreadImageRemote->Resume();
}

void ResourceBitmapRequester::LoadResource( Integration::ResourceRequest& request )
{
  DALI_ASSERT_DEBUG( (0 != dynamic_cast<BitmapResourceType*>(request.GetType())) && "Only requsts for bitmap resources can ever be routed to ResourceBitmapRequester.\n");
  BitmapResourceType* resType = static_cast<BitmapResourceType*>(request.GetType());
  if( resType )
  {
    // Work out what thread to decode / load the image on:
    ResourceThreadBase* const localImageThread = mThreadImageLocal;
    ResourceThreadBase* const remoteImageThread = mThreadImageRemote;
    ResourceThreadBase* workerThread;

    // Work out if the resource is in memory, a file, or in a remote server:
    ResourceThreadBase::RequestType requestType;
    if( request.GetResource().Get() )
    {
      requestType = ResourceThreadBase::RequestDecode;
      workerThread = localImageThread;
    }
    else
    {
      const std::string& resourcePath = request.GetPath();
      if( resourcePath.length() > 7 && strncasecmp( resourcePath.c_str(), "http://", 7 ) == 0 )
      {
        requestType = ResourceThreadBase::RequestDownload;
        workerThread = remoteImageThread;
      }
      else
      {
        requestType = ResourceThreadBase::RequestLoad;
        workerThread = localImageThread;
      }
    }

    // Dispatch the job to the right thread:
    workerThread->AddRequest( request, requestType );
  }
}

Integration::LoadStatus ResourceBitmapRequester::LoadFurtherResources( Integration::ResourceRequest& request, LoadedResource partialResource )
{
  // Nothing to do
  return RESOURCE_COMPLETELY_LOADED;
}

void ResourceBitmapRequester::SaveResource(const Integration::ResourceRequest& request )
{
  // Nothing to do
}

void ResourceBitmapRequester::CancelLoad(Integration::ResourceId id, Integration::ResourceTypeId typeId)
{
  mThreadImageLocal->CancelRequest(id);
  mThreadImageRemote->CancelRequest(id);
}

} // SlpPlatform
} // Dali
