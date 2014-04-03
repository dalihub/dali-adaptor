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

#include "resource-model-requester.h"

using namespace Dali::Integration;

namespace Dali
{
namespace SlpPlatform
{

ResourceModelRequester::ResourceModelRequester( ResourceLoader& resourceLoader )
: ResourceRequesterBase( resourceLoader ),
  mThreadModel(NULL)
{
}

ResourceModelRequester::~ResourceModelRequester()
{
  delete mThreadModel;
}

void ResourceModelRequester::Pause()
{
  if( mThreadModel )
  {
     mThreadModel->Pause();
  }
}

void ResourceModelRequester::Resume()
{
  if( mThreadModel )
  {
     mThreadModel->Resume();
  }
}

void ResourceModelRequester::LoadResource( Integration::ResourceRequest& request )
{
  if( ! mThreadModel )
  {
    mThreadModel  = new ResourceThreadModel( mResourceLoader );
  }

  mThreadModel->AddRequest(request, ResourceThreadBase::RequestLoad);
}

ResourcePointer ResourceModelRequester::LoadResourceSynchronously( const Integration::ResourceType& type, const std::string& path )
{
  DALI_ASSERT_ALWAYS( 0 && "Cannot load models synchronously" );
  return NULL;
}

LoadStatus ResourceModelRequester::LoadFurtherResources( ResourceRequest& request, LoadedResource partialResource )
{
  // Nothing to do
  return RESOURCE_COMPLETELY_LOADED;
}

void ResourceModelRequester::SaveResource(const ResourceRequest& request )
{
  if( ! mThreadModel )
  {
    mThreadModel  = new ResourceThreadModel( mResourceLoader );
  }

  // Nothing to do
  mThreadModel->AddRequest(request, ResourceThreadBase::RequestSave);
}

void ResourceModelRequester::CancelLoad( ResourceId id, ResourceTypeId typeId)
{
  mThreadModel->CancelRequest(id);
}

} // SlpPlatform
} // Dali
