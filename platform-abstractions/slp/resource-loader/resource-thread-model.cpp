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

#include <iostream>
#include <fstream>

#include "resource-thread-model.h"

#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/dali-core.h>
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/object/base-object.h>

#include "binary-model-builder.h"

#if defined(DALI_PROFILE_MOBILE) || defined(DALI_PROFILE_TV)
#include "assimp-model-builder.h"
#include "assimp-proxy.h"
#else
#include "assimp-stubs.h"
#endif

using namespace std;
using namespace Dali::Integration;
using boost::mutex;
using boost::unique_lock;
using boost::scoped_ptr;

namespace Dali
{

namespace SlpPlatform
{

ResourceThreadModel::ResourceThreadModel(ResourceLoader& resourceLoader)
: ResourceThreadBase(resourceLoader),
  mModelImporter(NULL)
{
}

ResourceThreadModel::~ResourceThreadModel()
{
  TerminateThread();

  if( mModelImporter)
  {
    // Closes dynamic library
    delete mModelImporter;
  }
}

void ResourceThreadModel::Load(const ResourceRequest& request)
{
  DALI_ASSERT_DEBUG(request.GetType()->id == ResourceModel);

  DALI_LOG_INFO(mLogFilter, Debug::Verbose, "%s(%s)\n", __PRETTY_FUNCTION__, request.GetPath().c_str());

  bool success(false);
  ModelData modelData;

  scoped_ptr<ModelBuilder> modelBuilder( CreateModelBuilder(request.GetPath()) );

  if( modelBuilder )
  {
    modelData = ModelData::New(modelBuilder->GetModelName());
    success = modelBuilder->Build(modelData);
  }

  if( success )
  {
    // Construct LoadedResource and ResourcePointer for model data
    LoadedResource resource( request.GetId(), request.GetType()->id, ResourcePointer(&(modelData.GetBaseObject())));

    // Queue the loaded resource
    mResourceLoader.AddLoadedResource(resource);
  }
  else
  {
    // add to the failed queue
    FailedResource resource(request.GetId(), FailureUnknown);
    mResourceLoader.AddFailedLoad(resource);
  }
}

void ResourceThreadModel::Save(const ResourceRequest& request)
{
  DALI_ASSERT_DEBUG(request.GetType()->id == ResourceModel);

  DALI_LOG_INFO(mLogFilter, Debug::Verbose, "%s(%s)\n", __PRETTY_FUNCTION__, request.GetPath().c_str());

  bool success(false);

  BaseObject* baseObject = dynamic_cast<BaseObject*>(request.GetResource().Get());
  if( baseObject != NULL )
  {
    BaseHandle baseHandle(baseObject);
    ModelData modelData = ModelData::DownCast(baseHandle);
    if( modelData )
    {
      scoped_ptr<BinaryModelBuilder> modelBuilder (new BinaryModelBuilder(request.GetPath().c_str()));
      if ( modelBuilder->Write(modelData) )
      {
        success = true;

        // Construct SavedResource
        SavedResource resource( request.GetId(), request.GetType()->id);

        // Queue the loaded resource
        mResourceLoader.AddSavedResource(resource);
      }
    }
  }
  if( ! success )
  {
    // add to the failed queue
    FailedResource resource(request.GetId(), FailureUnknown);
    mResourceLoader.AddFailedSave(resource);
  }
}

ModelBuilder* ResourceThreadModel::CreateModelBuilder(const std::string& modelFileName)
{
  ModelBuilder* modelBuilder = NULL;
  char fileMagic[4];

  // read first 4 bytes of file
  {
    std::filebuf buf;
    buf.open(modelFileName.c_str(), std::ios::in | std::ios::binary);
    std::iostream stream(&buf);
    stream.read(fileMagic, 4);
  }

  // if file starts with a 'DALI' tag/marker create a binary builder
  if( fileMagic[0] == 'D' &&
      fileMagic[1] == 'A' &&
      fileMagic[2] == 'L' &&
      fileMagic[3] == 'I' )
  {
    modelBuilder = new BinaryModelBuilder(modelFileName);
  }
#if defined(DALI_PROFILE_MOBILE) || defined(DALI_PROFILE_TV)
  else
  {
    if(!mModelImporter)
    {
      // Loads assimp library only when needed
      mModelImporter = new AssimpProxy;
    }
    modelBuilder = new AssimpModelBuilder(mModelImporter, modelFileName);
  }
#endif
  return modelBuilder;
}

} // namespace SlpPlatform

} // namespace Dali
