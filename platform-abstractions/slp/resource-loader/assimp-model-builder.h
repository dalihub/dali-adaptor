#ifndef __DALI_SLP_PLATFORM_ASSIMP_MODEL_BUILDER_H__
#define __DALI_SLP_PLATFORM_ASSIMP_MODEL_BUILDER_H__

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

#include "model-builder.h"

#include <string>
#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>

#include <dali/public-api/modeling/model-data.h>

namespace Dali
{
class Bone;
class Mesh;
class Entity;
class ModelAnimationMap;
class EntityAnimatorMap;

class MeshData;

namespace SlpPlatform
{

class AssimpProxy;

class AssimpModelBuilder : public ModelBuilder
{
public:
  /**
   * Construct a Builder object with the filename of the Model to import
   * @param[in] name - the filename of the model
   */
  AssimpModelBuilder(AssimpProxy* importer, const std::string& name);

  /**
   * Destructor
   */
  virtual ~AssimpModelBuilder();

public: // ModelBuilder Interface

  /**
   * Build a model structure from the Assimp scene
   * @param[inout] Model - the model to build
   */
  virtual bool Build(ModelData model);

  virtual const std::string& GetModelName();

private:
  /**
   * Build this mesh from the assimp model.
   * @param[out]   meshData  - object to which the data will be written into
   * @param[in]    ai_scene  - the Assimp scene with the data
   * @param[in]    meshIndex - the index within the ai_scene of the mesh data
   * @param[in]    model     - the current model
   */
  void BuildMeshData(MeshData& meshData, const aiScene* ai_scene, unsigned int meshIndex, ModelData& model);

  /**
   * Build this entity from the assimp model
   * @pre The internal model should already contain the assimp mesh data
   * @param[in]    ai_scene  - the assimp scene with the data
   * @param[in]    ai_node   - the assimp node to which this entity corresponds
   * @param[in]    model     - the current model
   * @param[in]    transform - the current transform matrix to apply
   * @return An Entity object.
   */
  Entity BuildEntity(const aiScene* ai_scene, aiNode* ai_node, ModelData* model, const Matrix& transform);

  /**
   * Create a key frame animator from the given node.
   * @param[in] node - assimp animation node
   * @param[in] duration - length of animation in seconds
   * @return the animator data for the node
   */
  EntityAnimatorMap CreateAnimator(aiNodeAnim* aiNode, float duration, const std::string& name, Entity animatedEntity);

  /**
   * Build this animation from the assimp model
   * @pre The internal model should already contain all the entities of the scene, and they
   * should have an entity id set (so that the animation can be referenced back to them)
   * @param[in]    ai_anim   - the assimp animation
   * @param[in]    model     - the current internal model
   * @return The animation
   */
  ModelAnimationMap BuildAnimation(const aiAnimation *ai_anim, ModelData *model);

  /**
   * Build this light from the assimp model
   * @param[in]    ai_scene    - The assimp scene with the data
   * @param[in]    lightIndex - The index of the light within the assimp scene
   * @return A valid Dali::Light handle or false on failure
   */
  Light BuildLight(const aiScene* ai_scene, const unsigned int lightIndex);

  /**
   * Build this material from the assimp model, and load all appropriate textures
   * @param[in] ai_material A pointer to an <a href="http://assimp.sourceforge.net/lib_html/structai_material.html">Assimp Material</a>.
   * @param[in] sBasePath   The base path for any texture filenames referenced by the material
   * @return                A pointer to a Material or false.
   */
  Material BuildMaterial(const aiMaterial* ai_material, const std::string& sBasePath);

  /**
   * Match up the bone names to entities. Goes through each mesh, and assigns the correct
   * entity onto each bone.
   * @model - the model to update
   */
  void MatchBonesToEntities(ModelData* model);


private: // Attributes
  const std::string mFilename;
  std::string       mModelName;
  char*             mTmpFilename;
  AssimpProxy*      mModelImporter;
};

}//SlpPlatform
}//Dali

#endif // __DALI_SLP_PLATFORM_ASSIMP_MODEL_BUILDER_H__
