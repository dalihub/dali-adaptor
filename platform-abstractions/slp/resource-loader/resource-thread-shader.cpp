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

#include <stdio.h>

#include "resource-thread-shader.h"

#include <dali/integration-api/debug.h>
#include <dali/integration-api/shader-data.h>

using namespace std;
using namespace Dali::Integration;
using boost::mutex;
using boost::unique_lock;
using boost::scoped_ptr;

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
  DALI_ASSERT_DEBUG(request.GetType()->id == ResourceShader);

  DALI_LOG_INFO(mLogFilter, Debug::Verbose, "ResourceThreadShader::Load(%s)\n", request.GetPath().c_str());

  const ShaderResourceType& shaderRequest = dynamic_cast<const ShaderResourceType&>(*request.GetType());
  ShaderDataPtr shaderData( new ShaderData( shaderRequest.vertexShader, shaderRequest.fragmentShader) );

  FILE* fp = fopen(request.GetPath().c_str(), "rb");

  if( fp != NULL )
  {
    if( fseek(fp, 0, SEEK_END) )
    {
      DALI_LOG_ERROR("Error seeking to end of file\n");
    }

    long positionIndicator = ftell(fp);
    unsigned int binaryLength( 0u );
    if( positionIndicator > -1L )
    {
      binaryLength = static_cast<unsigned int>(positionIndicator);
    }

    if( binaryLength > 0u )
    {
      if( fseek(fp, 0, SEEK_SET) )
      {
        DALI_LOG_ERROR("Error seeking to start of file\n");
      }

      shaderData->AllocateBuffer(binaryLength);

      unsigned int length = fread(shaderData->buffer.data(), 1, binaryLength, fp);

      DALI_LOG_INFO(mLogFilter, Debug::Verbose, "ResourceThreadShader::Load() - loaded %d from '%s'\n", length, request.GetPath().c_str());

      DALI_ASSERT_ALWAYS( length == binaryLength && "Shader data read from file does not match file size" );
    }

    fclose(fp);
  }
  else
  {
    DALI_LOG_WARNING("ResourceThreadShader::Load() - failed to load '%s'\n", request.GetPath().c_str());
  }

  // Always returning successful load, The shader will compile the vertex/fragment sources
  // in ShaderData if the binaryBuffer is zero length

  // Construct LoadedResource and ResourcePointer for image data
  LoadedResource resource( request.GetId(), request.GetType()->id, ResourcePointer(shaderData.Get()) );

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
    fwrite( shaderData->buffer.data(), shaderData->buffer.size(), 1, fp );

    fclose(fp);

    result = true;

    DALI_LOG_INFO(mLogFilter, Debug::Verbose, "ResourceThreadShader::Save(%s) - succeeded, wrote %d bytes\n",
                  request.GetPath().c_str(), shaderData->buffer.size());
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
