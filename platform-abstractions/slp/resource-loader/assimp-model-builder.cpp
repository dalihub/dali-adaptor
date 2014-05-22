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

// Class header
#include "assimp-model-builder.h"

// External headers
#include <libgen.h>
#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h> // Post processing flags

// Core headers
#include <dali/public-api/common/light.h>
#include <dali/public-api/math/matrix.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/geometry/mesh-data.h>
#include <dali/public-api/modeling/bone.h>
#include <dali/public-api/modeling/entity.h>
#include <dali/public-api/modeling/entity-animator-map.h>
#include <dali/public-api/modeling/model-data.h>
#include <dali/public-api/modeling/model-animation-map.h>

#include <dali/public-api/animation/key-frames.h>

#include <dali/integration-api/debug.h>

// Local headers
#include "assimp-proxy.h"
#include "left-hand-convertor.h"

namespace Dali
{
namespace SlpPlatform
{

struct BoneWeight
{
  size_t boneIdx;
  float  weight;
  BoneWeight(size_t aBoneIdx, float aWeight) : boneIdx(aBoneIdx), weight(aWeight) {}
};

typedef std::vector<std::vector<BoneWeight> > VertexWeights;

/********************************************************************************
 *
 */
AssimpModelBuilder::AssimpModelBuilder(AssimpProxy* importer, const std::string& name)
  : mFilename(name), mModelName(), mTmpFilename(NULL), mModelImporter(importer)
{
}

/********************************************************************************
 *
 */
AssimpModelBuilder::~AssimpModelBuilder()
{
  if(mTmpFilename != NULL)
  {
    free(mTmpFilename);
  }
}


/********************************************************************************
 * Return the base name of the model filename
 */
const std::string& AssimpModelBuilder::GetModelName()
{
  // TODO - check for Unix path handling in STL/Boost

  const char* filename = mFilename.c_str();
  mTmpFilename = (char*) malloc(strlen(filename)+1);
  strcpy(mTmpFilename, filename);
  mModelName = basename(mTmpFilename); // Note, this modifies mFilename
  return mModelName;
}

/********************************************************************************
 * Build the internal model
 */
bool AssimpModelBuilder::Build(ModelData model)
{
  bool built = false;

  if (mFilename.empty())
  {
    DALI_LOG_ERROR("empty filename");
    return built;
  }

  unsigned int ppSteps =
    aiProcess_FlipUVs                  | // Flip all UV coordinates along the y-axis
    aiProcess_Triangulate              | // Make triangular faces
    aiProcess_JoinIdenticalVertices    | // Join identical vertices / optimize indexing
    aiProcess_FixInfacingNormals       | // Inverts all in-facing normals.
    aiProcess_ValidateDataStructure    | // perform a full validation of the loader's output
    aiProcess_ImproveCacheLocality     | // improve the cache locality of the output vertices
    aiProcess_RemoveRedundantMaterials | // remove redundant materials
    aiProcess_FindInvalidData          | // detect invalid model data, such as invalid normal vectors
    aiProcess_GenUVCoords              | // convert spherical, cylindrical, box and planar mapping to proper UVs
    aiProcess_TransformUVCoords        | // preprocess UV transformations (scaling, translation ...)
    aiProcess_FindInstances            | // search for instanced meshes and remove them by references to one master
    aiProcess_LimitBoneWeights         | // limit bone weights to 4 per vertex
    aiProcess_OptimizeMeshes           | // join small meshes, if possible;
    aiProcess_SortByPType              |
    0;

  //------------- Load/Parse model file -------------//

  AssimpScene scene(mModelImporter, mFilename, ppSteps);
  const aiScene* ai_scene = scene.GetScene();
  if (!ai_scene)
  {
    return false;
  }

  // Translate from X right, Y up, Z forward to X right, Y down, Z foward
  LeftHandConvertor lhConvertor(ai_scene);
  lhConvertor.ProcessScene();

  //------------- Load Materials - before meshes -------------//
  if (ai_scene->HasMaterials())
  {
    for( unsigned int materialIdx = 0; materialIdx < ai_scene->mNumMaterials; ++materialIdx)
    {
      std::string sBasePath;
      std::string::size_type position = mFilename.rfind('/', mFilename.length() - 1);

      if( position != std::string::npos)
      {
        sBasePath = mFilename.substr(0, position + 1);
      }

      Material material( BuildMaterial(ai_scene->mMaterials[materialIdx], sBasePath) );
      if (material)
      {
        model.AddMaterial(material);
      }
    }
  }

  //------------- Load Meshes - (Requires materials) -------------//
  if (ai_scene->HasMeshes())
  {
    for( unsigned int i = 0; i < ai_scene->mNumMeshes; ++i)
    {
      MeshData meshData;
      BuildMeshData(meshData, ai_scene, i, model);
      model.AddMesh(meshData);
    }
  }

  //------------- Create entities from nodes in the scene -------------//
  Matrix identityMatrix;
  identityMatrix.SetIdentity();

  Entity rootEntity = BuildEntity(ai_scene, ai_scene->mRootNode, &model, identityMatrix);
  model.SetRootEntity(rootEntity);

  //------------- Load Animations. (Requiries entities) -------------//
  if(ai_scene->HasAnimations())
  {
    ModelAnimationMapContainer& mapContainer( model.GetAnimationMapContainer() );
    mapContainer.reserve( ai_scene->mNumAnimations );
    for(unsigned int i = 0; i < ai_scene->mNumAnimations; ++i)
    {
      const aiAnimation *aiAnim = ai_scene->mAnimations[i];

      ModelAnimationMap animationMap(BuildAnimation(aiAnim, &model));
      mapContainer.push_back(animationMap);
    }
  }

  //------------- Load lights -------------//
  if (ai_scene->HasLights())
  {
    for( unsigned int i = 0; i < ai_scene->mNumLights; ++i)
    {
      Light light = BuildLight(ai_scene, i);
      model.AddLight(light);

      Entity entity = rootEntity.Find(light.GetName());

      // TODO: Could also save the entity handle in the light.
      if (entity)
      {
        entity.SetType(Entity::LIGHT);
      }
    }
  }

  return true;
}


/********************************************************************************
 * Build the mesh
 */
void AssimpModelBuilder::BuildMeshData(MeshData& meshData, const aiScene* ai_scene, unsigned int meshIndex, ModelData& model)
{
  if (NULL != ai_scene && meshIndex < ai_scene->mNumMeshes)
  {
    aiMesh* ai_mesh = ai_scene->mMeshes[meshIndex];
    if (NULL != ai_mesh)
    {
      unsigned int vertexCount = ai_mesh->mNumVertices;
      VertexWeights vertexWeights(vertexCount);

      if( ai_mesh->HasBones())
      {
        // Need to convert from bones containing vertex indices to
        // vertices containing bone indices
        for( unsigned int boneIdx = 0; boneIdx < ai_mesh->mNumBones; boneIdx++)
        {
          const aiBone* ai_bone = ai_mesh->mBones[boneIdx];
          for( unsigned int weightIdx = 0; weightIdx < ai_bone->mNumWeights; weightIdx++)
          {
            size_t vertexIdx = ai_bone->mWeights[weightIdx].mVertexId;
            vertexWeights[vertexIdx].push_back(BoneWeight(boneIdx, ai_bone->mWeights[weightIdx].mWeight));
          }
        }
      }

      MeshData::VertexContainer vertices( vertexCount );
      MeshData::FaceIndices faces;
      BoneContainer bones;
      for (unsigned int vertexIdx = 0; vertexIdx < vertexCount; ++vertexIdx)
      {
        MeshData::Vertex& vertex = vertices.at(vertexIdx);
        aiVector3D* ai_vector = &ai_mesh->mVertices[vertexIdx];

        vertex.x = ai_vector->x;
        vertex.y = ai_vector->y;
        vertex.z = ai_vector->z;

        if (ai_mesh->HasTextureCoords(0))
        {
          ai_vector = ai_mesh->mTextureCoords[0];
          ai_vector += vertexIdx;
          vertex.u = ai_vector->x;
          vertex.v = ai_vector->y;
        }

        if (ai_mesh->HasNormals())
        {
          ai_vector = &ai_mesh->mNormals[vertexIdx];
          vertex.nX = ai_vector->x;
          vertex.nY = ai_vector->y;
          vertex.nZ = ai_vector->z;
        }

        memset(&vertex.boneIndices[0], 0, sizeof(unsigned char) * MeshData::Vertex::MAX_BONE_INFLUENCE);
        memset(&vertex.boneWeights[0], 0, sizeof(float) * MeshData::Vertex::MAX_BONE_INFLUENCE);

        if( ai_mesh->HasBones())
        {
          for(size_t boneIdx=0;
              boneIdx < vertexWeights[vertexIdx].size() && boneIdx < MeshData::Vertex::MAX_BONE_INFLUENCE;
              boneIdx++)
          {
            vertex.boneIndices[boneIdx] = vertexWeights[vertexIdx][boneIdx].boneIdx;
            vertex.boneWeights[boneIdx] = vertexWeights[vertexIdx][boneIdx].weight;
          }
        }
      }

      if (ai_mesh->HasFaces() && ai_mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
      {
        faces.resize( ai_mesh->mNumFaces * 3 );

        unsigned short* pIndex = &(faces)[0];
        for( unsigned int i = 0; i < ai_mesh->mNumFaces; ++i)
        {
          *pIndex++ = ai_mesh->mFaces[i].mIndices[0];
          *pIndex++ = ai_mesh->mFaces[i].mIndices[1];
          *pIndex++ = ai_mesh->mFaces[i].mIndices[2];
        }
      }
      else
      {
        DALI_LOG_ERROR("Not triangles\n");
      }

      if (ai_mesh->HasBones())
      {
        for(size_t boneIdx = 0; boneIdx < ai_mesh->mNumBones; ++boneIdx)
        {
          aiBone& ai_bone = *ai_mesh->mBones[boneIdx];

          Matrix offsetMatrix( ai_bone.mOffsetMatrix[0] );
          offsetMatrix.Transpose();
          bones.push_back( Bone( ai_bone.mName.data, offsetMatrix ) );
        }
      }

      meshData.SetHasTextureCoords(ai_mesh->HasTextureCoords(0));
      meshData.SetHasNormals(ai_mesh->HasNormals());
      meshData.SetData(vertices, faces, bones, model.GetMaterial(ai_mesh->mMaterialIndex));
    }
  }
}


/********************************************************************************
 * Build the entity
 */
Entity AssimpModelBuilder::BuildEntity(
  const aiScene* ai_scene,
  aiNode*        ai_node,
  ModelData*     model,
  const Matrix&  transform)
{
  std::string name;
  if( 0 != ai_node->mName.length )
  {
    name = ai_node->mName.data;
  }

  Entity entity = Entity::New(name);
  if (entity)
  {
    aiMatrix4x4& m = ai_node->mTransformation;

    float assimpMatrix[16] = { m.a1, m.a2, m.a3, m.a4,
                               m.b1, m.b2, m.b3, m.b4,
                               m.c1, m.c2, m.c3, m.c4,
                               m.d1, m.d2, m.d3, m.d4 };

    Matrix entityMatrix = Matrix( assimpMatrix );
    entityMatrix.Transpose();
    entity.SetTransformMatrix(entityMatrix);

    Matrix tempMatrix( false ); // Don't initialize.
    Matrix::Multiply( tempMatrix, transform, entityMatrix );

    if (ai_node->mNumMeshes)
    {
      entity.SetMeshCapacity(ai_node->mNumMeshes);

      for( unsigned int i = 0; i < ai_node->mNumMeshes; ++i)
      {
        MeshData& mesh = model->GetMesh(ai_node->mMeshes[i]);
        entity.AddMeshIndex(ai_node->mMeshes[i]);

        Vector4 lowerBounds( entity.GetLowerBounds() );
        Vector4 upperBounds( entity.GetUpperBounds() );
        mesh.AddToBoundingVolume( lowerBounds,
                                  upperBounds,
                                  tempMatrix );
        entity.AddToBounds( Vector3(lowerBounds), Vector3(upperBounds) );
      }
    }

    if (ai_node->mNumChildren)
    {
      for (unsigned int childIdx = 0; childIdx < ai_node->mNumChildren; ++childIdx)
      {
        Entity child = BuildEntity(ai_scene, ai_node->mChildren[childIdx], model, tempMatrix);
        entity.Add(child);
        entity.AddToBounds(child);
      }
    }
  }
  return entity;
}




/********************************************************************************
 * Create an EntityAnimatorMap
 */
EntityAnimatorMap AssimpModelBuilder::CreateAnimator(
  aiNodeAnim*        aiNode,
  float              duration,
  const std::string& entityName,
  Entity             animatedEntity)
{
  Dali::KeyFrames positionKeyFrames = Dali::KeyFrames::New();
  Dali::KeyFrames scaleKeyFrames    = Dali::KeyFrames::New();
  Dali::KeyFrames rotationKeyFrames = Dali::KeyFrames::New();

  for(unsigned int aiKeyIdx = 0; aiKeyIdx < aiNode->mNumPositionKeys; aiKeyIdx++)
  {
    aiVectorKey* position = aiNode->mPositionKeys + aiKeyIdx;
    float progress = position->mTime / duration;
    Property::Value value(Vector3(position->mValue.x, position->mValue.y, position->mValue.z));
    positionKeyFrames.Add(progress, value);
  }

  for(unsigned int aiKeyIdx = 0; aiKeyIdx < aiNode->mNumScalingKeys; aiKeyIdx++)
  {
    aiVectorKey* scale = aiNode->mScalingKeys + aiKeyIdx;
    float progress = scale->mTime / duration;
    Property::Value value(Vector3(scale->mValue.x, scale->mValue.y, scale->mValue.z));
    scaleKeyFrames.Add(progress, value);
  }

  for(unsigned int aiKeyIdx = 0; aiKeyIdx < aiNode->mNumRotationKeys; aiKeyIdx++)
  {
    aiQuatKey* rotation = aiNode->mRotationKeys + aiKeyIdx;
    float progress = rotation->mTime / duration;
    aiQuaternion* ai_rotation = & rotation->mValue;
    Quaternion quat(ai_rotation->w, ai_rotation->x, ai_rotation->y, ai_rotation->z);
    Property::Value value(quat);
    rotationKeyFrames.Add(progress, value);
  }

  EntityAnimatorMap entityAnim(entityName);

  if(aiNode->mNumPositionKeys > 0)
  {
    entityAnim.SetPositionKeyFrames(positionKeyFrames);
  }

  if(aiNode->mNumScalingKeys > 0)
  {
    entityAnim.SetScaleKeyFrames(scaleKeyFrames);
  }

  if(aiNode->mNumRotationKeys > 0)
  {
    entityAnim.SetRotationKeyFrames(rotationKeyFrames);
  }

  entityAnim.SetDuration(duration);
  return entityAnim;
}

/********************************************************************************
 * Build the animation. This finds each entity that each channel animates, and
 * creates a relation between them
 */
ModelAnimationMap AssimpModelBuilder::BuildAnimation(const aiAnimation* ai_anim, ModelData* model)
{
  ModelAnimationMap animation;

  animation.name     = ai_anim->mName.data;
  animation.duration = ai_anim->mDuration;
  animation.repeats  = 0;

  for(unsigned int nodeIdx = 0; nodeIdx < ai_anim->mNumChannels; nodeIdx++)
  {
    // Each node corresponds to an animation, and named after the entity it's
    // animating.
    aiNodeAnim* aiNode = ai_anim->mChannels[nodeIdx];

    std::string nodeName;
    if (aiNode->mNodeName.length > 0)
    {
      nodeName.assign(aiNode->mNodeName.data, aiNode->mNodeName.length);
    }

    Entity rootEntity = model->GetRootEntity();
    Entity animatedEntity = rootEntity.Find(nodeName);
    if(animatedEntity)
    {
      EntityAnimatorMap entityAnimator(CreateAnimator(aiNode, ai_anim->mDuration, nodeName, animatedEntity));
      animation.animators.push_back(entityAnimator);
    }
  }

  return animation;
}


/********************************************************************************
 * Build material. This loads the material attributes from the model, and loads
 * all appropriate textures.
 */
Material AssimpModelBuilder::BuildMaterial(
  const aiMaterial*  ai_material,
  const std::string& sBasePath)
{
  Material material;

  if (NULL != ai_material)
  {
    aiString temporaryString;
    mModelImporter->GetMaterialString(ai_material, AI_MATKEY_NAME, &temporaryString);

    // create material object
    material = Material::New(temporaryString.data);

    float value;
    if (AI_SUCCESS == mModelImporter->GetMaterialFloat(ai_material, AI_MATKEY_OPACITY, &value))
    {
      material.SetOpacity(value);
    }

    if (AI_SUCCESS == mModelImporter->GetMaterialFloat(ai_material, AI_MATKEY_SHININESS, &value))
    {
      material.SetShininess(value);
    }

    aiColor4D tmpAiColor;

    if (AI_SUCCESS == mModelImporter->GetMaterialColor(ai_material, AI_MATKEY_COLOR_AMBIENT,  &tmpAiColor))
    {
      material.SetAmbientColor(Vector4(tmpAiColor.r, tmpAiColor.g, tmpAiColor.b, tmpAiColor.a));
    }

    if (AI_SUCCESS == mModelImporter->GetMaterialColor(ai_material, AI_MATKEY_COLOR_DIFFUSE,  &tmpAiColor))
    {
      material.SetDiffuseColor(Vector4(tmpAiColor.r, tmpAiColor.g, tmpAiColor.b, tmpAiColor.a));
    }

    if (AI_SUCCESS == mModelImporter->GetMaterialColor(ai_material, AI_MATKEY_COLOR_SPECULAR,  &tmpAiColor))
    {
      material.SetSpecularColor(Vector4(tmpAiColor.r, tmpAiColor.g, tmpAiColor.b, tmpAiColor.a));
    }

    if (AI_SUCCESS == mModelImporter->GetMaterialColor(ai_material, AI_MATKEY_COLOR_EMISSIVE,  &tmpAiColor))
    {
      material.SetEmissiveColor(Vector4(tmpAiColor.r, tmpAiColor.g, tmpAiColor.b, tmpAiColor.a));
    }

    std::string sPath;

    // Check for a diffuse texture
    if(AI_SUCCESS == mModelImporter->GetMaterialString(ai_material, AI_MATKEY_TEXTURE_DIFFUSE(0), &temporaryString))
    {
      sPath = temporaryString.data;
      if (sPath.find("Procedural") == std::string::npos)
      {
        sPath = sBasePath;
        sPath += temporaryString.data;
        material.SetDiffuseTextureFileName(sPath);

        int mapU, mapV;
        mModelImporter->GetMaterialInteger(ai_material, AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), &mapU);
        mModelImporter->GetMaterialInteger(ai_material, AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), &mapV);
        material.SetMapU(mapU);
        material.SetMapV(mapV);
      }
    }

    // Check for an opacity texture
    if(AI_SUCCESS == mModelImporter->GetMaterialString(ai_material, AI_MATKEY_TEXTURE_OPACITY(0), &temporaryString))
    {
      sPath = sBasePath;
      sPath += temporaryString.data;
      material.SetOpacityTextureFileName(sPath);
    }

    // Check for a normal map
    if(AI_SUCCESS == mModelImporter->GetMaterialString(ai_material, AI_MATKEY_TEXTURE_NORMALS(0), &temporaryString))
    {
      sPath = sBasePath;
      sPath += temporaryString.data;
      material.SetNormalMapFileName(sPath);
    }
    else
    {
      // Check for a height map
      if(AI_SUCCESS == mModelImporter->GetMaterialString(ai_material, AI_MATKEY_TEXTURE_HEIGHT(0), &temporaryString))
      {
        sPath = sBasePath;
        sPath += temporaryString.data;
        material.SetNormalMapFileName(sPath);
        material.SetHasHeightMap(true);
      }
    }
  }
  return material;
}

/********************************************************************************
 * Build the light from the model
 */
Light AssimpModelBuilder::BuildLight(const aiScene* ai_scene, const unsigned int lightIndex)
{
  Light light;

  if (NULL != ai_scene && lightIndex < ai_scene->mNumLights)
  {
    aiLight* ai_light = ai_scene->mLights[lightIndex];

    light = Light::New(ai_light->mName.data);
    light.SetSpotAngle(Vector2(ai_light->mAngleInnerCone, ai_light->mAngleOuterCone));
    light.SetAmbientColor(Vector3(ai_light->mColorAmbient.r, ai_light->mColorAmbient.g, ai_light->mColorAmbient.b));
    light.SetDiffuseColor(Vector3(ai_light->mColorDiffuse.r, ai_light->mColorDiffuse.g, ai_light->mColorDiffuse.b));
    light.SetSpecularColor(Vector3(ai_light->mColorSpecular.r, ai_light->mColorSpecular.g, ai_light->mColorSpecular.b));
    light.SetDirection(Vector3(ai_light->mDirection.x, ai_light->mDirection.y, ai_light->mDirection.z));
  }
  return light;
}

}//SlpPlatform

}//Dali
