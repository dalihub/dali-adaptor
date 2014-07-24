#ifndef __DALI_SLP_PLATFORM_ASSIMP_PROXY_H__
#define __DALI_SLP_PLATFORM_ASSIMP_PROXY_H__

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

#include <assimp/assimp.h>

struct aiScene;
struct aiMaterial;
struct aiColor4D;
struct aiString;

namespace Dali
{
namespace SlpPlatform
{

/**
 * Class to proxy the Assimp importer. This will allow us to dynamically load the assimp
 * library when required.
 */
class AssimpProxy
{
public:
  /**
   * Constructor. Dynamically loads the assimp library
   */
  AssimpProxy();

  /**
   * Destructor. Closes the loaded assimp library.
   */
  ~AssimpProxy();

  /**
   * Import a model file into a scene object. Should call ReleaseImport when
   * finished with the scene.
   * @param[in] fileName Filename of the model
   * @param[in] postProcessFlags How Assimp will process the scene after it has loaded.
   * @return aiScene The scene data, or NULL if it has failed to load.
   */
  const aiScene* ImportFile(std::string fileName, unsigned int postProcessFlags);

  /**
   * Release the scene data. Destroys the resources loaded with ImportFile.
   * @param[in] scene The scene data
   */
  void ReleaseImport(const aiScene* scene);

  /**
   * Get the error string. Useful if ImportFile fails.
   * @return the error string
   */
  const char* GetErrorString();

  /**
   * Get the color from the material for the given query.
   * @param[in]  material The material structure to query
   * @param[in]  key The query key
   * @param[in]  type The query type
   * @param[in]  index The query index
   * @param[out] data The output data
   * @return the return status.
   */
  enum aiReturn GetMaterialColor(
    const C_STRUCT aiMaterial* material,
    const char*                key,
    unsigned int               type,
    unsigned int               index,
    C_STRUCT aiColor4D*        data);

  /**
   * Get an integer from the material for the given query.
   * @param[in]  material The material structure to query
   * @param[in]  key The query key
   * @param[in]  type The query type
   * @param[in]  index The query index
   * @param[out] data The output integer
   * @return the return status.
   */
  enum aiReturn GetMaterialInteger(
    const C_STRUCT aiMaterial* material,
    const char*                key,
    unsigned int               type,
    unsigned int               index,
    int*                       data);

  /**
   * Get a float from the material for the given query.
   * @param[in]  material The material structure to query
   * @param[in]  key The query key
   * @param[in]  type The query type
   * @param[in]  index The query index
   * @param[out] data The output float
   * @return the return status.
   */
  enum aiReturn GetMaterialFloat(
    const C_STRUCT aiMaterial* material,
    const char*                key,
    unsigned int               type,
    unsigned int               index,
    float*                     data);

  /**
   * Get a string from the material for the given query.
   * @param[in]  material The material structure to query
   * @param[in]  key The query key
   * @param[in]  type The query type
   * @param[in]  index The query index
   * @param[out] data The output data
   * @return the return status.
   */
  enum aiReturn GetMaterialString(
    const C_STRUCT aiMaterial* material,
    const char*                key,
    unsigned int               type,
    unsigned int               index,
    C_STRUCT aiString*         data);


private:
  void* mLibHandle; // The dynamically loaded libary handle
};


/**
 * Class that loads and releases scene data.
 */
class AssimpScene
{
public:
  /**
   * Create the scene from the model
   */
  AssimpScene(AssimpProxy* importer, std::string fileName, unsigned int postProcessFlags);

  /**
   * Destroy the scene and release the imported data
   */
  ~AssimpScene();

  /**
   * Get the scene data
   */
  const aiScene* GetScene();

private:
  const aiScene* mScene;       ///< the scene data
  AssimpProxy* mModelImporter; ///< the model importer
};

}
}

#endif // __DALI_SLP_PLATFORM_ASSIMP_PROXY_H__