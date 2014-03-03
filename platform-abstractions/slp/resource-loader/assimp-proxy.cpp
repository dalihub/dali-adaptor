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
#include "assimp-proxy.h"

#include <stdio.h>
#include <dlfcn.h>

#include <dali/integration-api/debug.h>


namespace Dali
{
namespace SlpPlatform
{

// Private typedefs for function pointers returned from dlsym().

typedef const struct aiScene* PFImportFile(const char*, size_t);
typedef void                  PFReleaseImport(const aiScene*);
typedef const char*           PFGetErrorString();

typedef enum aiReturn PFGetMaterialColor(
  const C_STRUCT aiMaterial* pMaterial,
  const char*                pKey,
  unsigned int               type,
  unsigned int               index,
  C_STRUCT aiColor4D*        pOut);

typedef enum aiReturn PFGetMaterialIntegerArray(
  const C_STRUCT aiMaterial* pMaterial,
  const char*                pKey,
  unsigned int               type,
  unsigned int               index,
  int*                       pOut,
  unsigned int*              pMax);

typedef enum aiReturn PFGetMaterialFloatArray(
  const C_STRUCT aiMaterial* pMaterial,
  const char*                pKey,
  unsigned int               type,
  unsigned int               index,
  float*                     pOut,
  unsigned int*              pMax);

typedef enum aiReturn PFGetMaterialString(
  const C_STRUCT aiMaterial* pMaterial,
  const char*                pKey,
  unsigned int               type,
  unsigned int               index,
  C_STRUCT aiString*         pOut);


/**
 * Class to proxy the Assimp importer. This will allow us to dynamically load the assimp
 * library when required.
 */
AssimpProxy::AssimpProxy()
  : mLibHandle(NULL)
{
  mLibHandle = dlopen("libassimp.so", RTLD_LAZY);
  if(!mLibHandle)
  {
    fprintf(stderr, "Cannot load assimp library: %s\n", dlerror());
    mLibHandle = NULL;
  }
  else
  {
    // reset errors
    dlerror();
  }
}

AssimpProxy::~AssimpProxy()
{
  if(mLibHandle != NULL)
  {
    if(dlclose(mLibHandle))
    {
      fprintf(stderr, "Error closing assimp library: %s\n", dlerror());
    }
  }
}


const struct aiScene* AssimpProxy::ImportFile(std::string fileName, unsigned int postProcessFlags)
{
  const struct aiScene* scene = NULL;
  PFImportFile* aiImportFile = (PFImportFile*) dlsym(mLibHandle, "aiImportFile");
  if(aiImportFile == NULL)
  {
    fprintf(stderr, "Cannot get aiImportFile symbol from library: %s\n", dlerror());
  }
  else
  {
    dlerror();
    scene = aiImportFile(fileName.c_str(), postProcessFlags);
  }
  return scene;
}


void AssimpProxy::ReleaseImport(const aiScene* scene)
{
  PFReleaseImport* aiReleaseImport = (PFReleaseImport*) dlsym(mLibHandle, "aiReleaseImport");
  if(aiReleaseImport == NULL)
  {
    fprintf(stderr, "Cannot get aiReleaseImport symbol from library: %s\n", dlerror());
  }
  else
  {
    dlerror();
    aiReleaseImport(scene);
  }
}


const char* AssimpProxy::GetErrorString()
{
  const char* errorString=NULL;

  PFGetErrorString* aiGetErrorString = (PFGetErrorString*) dlsym(mLibHandle, "aiGetErrorString");
  if(aiGetErrorString == NULL)
  {
    fprintf(stderr, "Cannot get aiGetErrorString symbol from library: %s\n", dlerror());
  }
  else
  {
    dlerror();
    errorString = aiGetErrorString();
  }

  return errorString;
}


enum aiReturn AssimpProxy::GetMaterialColor(
  const C_STRUCT aiMaterial* material,
  const char*                key,
  unsigned int               type,
  unsigned int               index,
  C_STRUCT aiColor4D*        data)
{
  enum aiReturn status = aiReturn_FAILURE;

  PFGetMaterialColor* aiGetMaterialColor = (PFGetMaterialColor*) dlsym(mLibHandle, "aiGetMaterialColor");
  if(aiGetMaterialColor == NULL)
  {
    fprintf(stderr, "Cannot get aiGetMaterialColor symbol from library: %s\n", dlerror());
  }
  else
  {
    dlerror();
    status = aiGetMaterialColor(material, key, type, index, data);
  }

  return status;
}


enum aiReturn AssimpProxy::GetMaterialInteger(
  const C_STRUCT aiMaterial* material,
  const char*                key,
  unsigned int               type,
  unsigned int               index,
  int*                       data)
{
  enum aiReturn status = aiReturn_FAILURE;

  PFGetMaterialIntegerArray* aiGetMaterialIntegerArray = (PFGetMaterialIntegerArray*) dlsym(mLibHandle, "aiGetMaterialIntegerArray");
  if(aiGetMaterialIntegerArray == NULL)
  {
    fprintf(stderr, "Cannot get aiGetMaterialIntegerArray symbol from library: %s\n", dlerror());
  }
  else
  {
    dlerror();
    status = aiGetMaterialIntegerArray(material, key, type, index, data, NULL);
  }

  return status;
}

enum aiReturn AssimpProxy::GetMaterialFloat(
  const C_STRUCT aiMaterial* material,
  const char*                key,
  unsigned int               type,
  unsigned int               index,
  float*                     data)
{
  enum aiReturn status = aiReturn_FAILURE;

  PFGetMaterialFloatArray* aiGetMaterialFloatArray = (PFGetMaterialFloatArray*) dlsym(mLibHandle, "aiGetMaterialFloatArray");
  if(aiGetMaterialFloatArray == NULL)
  {
    fprintf(stderr, "Cannot get aiGetMaterialFloatArray symbol from library: %s\n", dlerror());
  }
  else
  {
    dlerror();
    status = aiGetMaterialFloatArray(material, key, type, index, data, NULL);
  }

  return status;
}


enum aiReturn AssimpProxy::GetMaterialString(
  const C_STRUCT aiMaterial* material,
  const char*                key,
  unsigned int               type,
  unsigned int               index,
  C_STRUCT aiString*         data)
{
  enum aiReturn status = aiReturn_FAILURE;

  PFGetMaterialString* aiGetMaterialString = (PFGetMaterialString*) dlsym(mLibHandle, "aiGetMaterialString");
  if(aiGetMaterialString == NULL)
  {
    fprintf(stderr, "Cannot get aiGetMaterialString symbol from library: %s\n", dlerror());
  }
  else
  {
    dlerror();
    status = aiGetMaterialString(material, key, type, index, data);
  }

  return status;
}



AssimpScene::AssimpScene(AssimpProxy* importer,
                         std::string  fileName,
                         unsigned int postProcessFlags)
  : mModelImporter(importer)
{
  mScene = importer->ImportFile(fileName, postProcessFlags);
  if(!mScene)
  {
    DALI_LOG_ERROR("%s", importer->GetErrorString());
  }
}

AssimpScene::~AssimpScene()
{
  // Ensure the imported model data is released
  mModelImporter->ReleaseImport(mScene);
}

const aiScene* AssimpScene::GetScene()
{
  return mScene;
}


}
}

