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

#include "resource-shader-requester.h"

using namespace Dali::Integration;

namespace Dali
{
namespace SlpPlatform
{

ResourceShaderRequester::ResourceShaderRequester( ResourceLoader& resourceLoader )
: ResourceRequesterBase( resourceLoader )
{
    mThreadShader  = new ResourceThreadShader( resourceLoader );
}

ResourceShaderRequester::~ResourceShaderRequester()
{
    delete mThreadShader;
}

void ResourceShaderRequester::Pause()
{
  mThreadShader->Pause();
}

void ResourceShaderRequester::Resume()
{
  mThreadShader->Resume();
}

void ResourceShaderRequester::LoadResource( Integration::ResourceRequest& request )
{
  mThreadShader->AddRequest(request, ResourceThreadBase::RequestLoad);
}

ResourcePointer ResourceShaderRequester::LoadResourceSynchronously( const Integration::ResourceType& type, const std::string& path )
{
  DALI_ASSERT_ALWAYS( 0 && "Cannot load shaders synchronously" );
  return NULL;
}

Integration::LoadStatus ResourceShaderRequester::LoadFurtherResources( Integration::ResourceRequest& request, LoadedResource partialResource )
{
  // Nothing to do
  return RESOURCE_COMPLETELY_LOADED;
}

void ResourceShaderRequester::SaveResource(const Integration::ResourceRequest& request )
{
  mThreadShader->AddRequest(request, ResourceThreadBase::RequestSave);
}

void ResourceShaderRequester::CancelLoad(Integration::ResourceId id, Integration::ResourceTypeId typeId)
{
  mThreadShader->CancelRequest(id);
}

} // SlpPlatform
} // Dali
