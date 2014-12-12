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

#include <stdio.h>

#include "resource-thread-shader.h"
#include <dali/integration-api/debug.h>
#include <dali/integration-api/shader-data.h>
#include <dali/public-api/common/scoped-pointer.h>

using namespace Dali::Integration;
using boost::mutex;
using boost::unique_lock;

namespace Dali
{

namespace SlpPlatform
{

ResourceThreadShader::ResourceThreadShader(ResourceLoader& resourceLoader)
: ResourceThreadBase(resourceLoader)
{
}

ResourceThreadShader::~ResourceThreadShader()
{
}

void ResourceThreadShader::Load(const ResourceRequest& request)
{
  // fake successfull load as this is dead code
  // Construct LoadedResource and ResourcePointer for shader data
  LoadedResource resource( request.GetId(), request.GetType()->id, ResourcePointer() );

  // Queue the loaded resource
  mResourceLoader.AddLoadedResource(resource);
}

void ResourceThreadShader::Save(const ResourceRequest& request)
{
  DALI_ASSERT_DEBUG(request.GetType()->id == ResourceShader);

  DALI_LOG_INFO(mLogFilter, Debug::Verbose, "ResourceThreadShader::Save(%s)\n", request.GetPath().c_str());

  bool result = false;

  ShaderDataPtr shaderData( reinterpret_cast< ShaderData* >(request.GetResource().Get()) );

  FILE *fp = fopen(request.GetPath().c_str(), "wb");
  if (fp != NULL)
  {
    fwrite( shaderData->GetBufferData(), shaderData->GetBufferSize(), 1, fp );

    fclose(fp);

    result = true;

    DALI_LOG_INFO(mLogFilter, Debug::Verbose, "ResourceThreadShader::Save(%s) - succeeded, wrote %d bytes\n",
                  request.GetPath().c_str(), shaderData->GetBufferSize());
  }
  else
  {
    DALI_LOG_WARNING("**Failed** to save \"%s\"\n", request.GetPath().c_str());
  }


  if (result)
  {
    // Construct LoadedResource and ResourcePointer for image data
    SavedResource resource( request.GetId(), request.GetType()->id );

    // Queue the loaded resource
    mResourceLoader.AddSavedResource(resource);
  }
  else
  {
    // add to the failed queue
    FailedResource resource(request.GetId(), FailureUnknown);
    mResourceLoader.AddFailedSave(resource);
  }
}


} // namespace SlpPlatform

} // namespace Dali
