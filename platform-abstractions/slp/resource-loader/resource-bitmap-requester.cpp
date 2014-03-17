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
  mThreadImage  = new ResourceThreadImage( resourceLoader );
  mThreadDistanceField = new ResourceThreadDistanceField( resourceLoader );
}

ResourceBitmapRequester::~ResourceBitmapRequester()
{
  delete mThreadImage;
  delete mThreadDistanceField;
}

void ResourceBitmapRequester::Pause()
{
  mThreadImage->Pause();
  mThreadDistanceField->Pause();
}

void ResourceBitmapRequester::Resume()
{
  mThreadImage->Resume();
  mThreadDistanceField->Resume();
}

void ResourceBitmapRequester::LoadResource( Integration::ResourceRequest& request )
{
  DALI_ASSERT_DEBUG( (0 != dynamic_cast<ImageResourceType*>(request.GetType())) && "Only requsts for bitmap resources can ever be routed to ResourceBitmapRequester.\n");
  ImageResourceType* resType = static_cast<ImageResourceType*>(request.GetType());
  if( resType )
  {
    // Work out if the resource is in memory or a file:
    const ResourceThreadBase::RequestType requestType = request.GetResource().Get() ? ResourceThreadBase::RequestDecode : ResourceThreadBase::RequestLoad;

    // Work out what thread to decode / load the image on:
    ResourceThreadBase* const imageThread = mThreadImage;
    ResourceThreadBase* const distanceFieldThread = mThreadDistanceField ;
    ResourceThreadBase* const workerThread = ( !resType->imageAttributes.IsDistanceField() ) ? imageThread : distanceFieldThread;

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
  mThreadImage->CancelRequest(id);
  mThreadDistanceField->CancelRequest(id);
}

Integration::ResourcePointer ResourceBitmapRequester::LoadResourceSynchronously( const Integration::ResourceType& resourceType, const std::string& resourcePath )
{
  // TODO - Refactor common code out of thread
  return mThreadImage->LoadResourceSynchronously( resourceType, resourcePath );
}

void ResourceBitmapRequester::GetClosestImageSize( const std::string& filename,
                                                   const ImageAttributes& attributes,
                                                   Vector2 &closestSize )
{
  // TODO - Refactor common code out of thread
  mThreadImage->GetClosestImageSize(filename, attributes, closestSize );
}

void ResourceBitmapRequester::GetClosestImageSize( Integration::ResourcePointer resourceBuffer,
                                                   const ImageAttributes& attributes,
                                                   Vector2 &closestSize )
{
  // TODO - Refactor common code out of thread
  mThreadImage->GetClosestImageSize(resourceBuffer, attributes, closestSize );
}

} // SlpPlatform
} // Dali
