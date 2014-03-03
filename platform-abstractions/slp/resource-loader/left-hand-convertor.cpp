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

#include "left-hand-convertor.h"

#include <dali/public-api/common/light.h>
#include <dali/public-api/math/matrix.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/modeling/bone.h>
#include <dali/public-api/modeling/entity-animator-map.h>
#include <dali/public-api/modeling/model-animation-map.h>
#include <dali/public-api/modeling/entity.h>
#include <dali/public-api/modeling/model-data.h>

namespace Dali
{
namespace SlpPlatform
{

LeftHandConvertor::LeftHandConvertor(const aiScene* aiScene)
: mAssimpScene(aiScene)
{
}

LeftHandConvertor::~LeftHandConvertor()
{
}

void LeftHandConvertor::ProcessScene()
{
  ConvertNode( mAssimpScene->mRootNode );

  for( std::size_t i=0; i < mAssimpScene->mNumMeshes; ++i )
  {
    ConvertMesh( mAssimpScene->mMeshes[i] );
  }

  for( std::size_t i=0; i < mAssimpScene->mNumMaterials; ++i )
  {
    ConvertMaterial( mAssimpScene->mMaterials[i] );
  }

  for ( std::size_t i=0; i < mAssimpScene->mNumAnimations; ++i )
  {
    aiAnimation* animation = mAssimpScene->mAnimations[i];
    for( std::size_t j=0; j < animation->mNumChannels; ++j )
    {
      aiNodeAnim* nodeAnimation = animation->mChannels[j];
      ConvertAnimation( nodeAnimation );
    }
  }
}

void LeftHandConvertor::ConvertNode( aiNode* node )
{
  node->mTransformation.b1 = -node->mTransformation.b1;
  node->mTransformation.b3 = -node->mTransformation.b3;
  node->mTransformation.b4 = -node->mTransformation.b4;
  node->mTransformation.a2 = -node->mTransformation.a2;
  node->mTransformation.c2 = -node->mTransformation.c2;
  node->mTransformation.d2 = -node->mTransformation.d2;

  for( std::size_t i=0; i < node->mNumChildren; ++i )
  {
    ConvertNode( node->mChildren[i] );
  }
}

void LeftHandConvertor::ConvertMesh( aiMesh* mesh )
{
  for( std::size_t i=0; i < mesh->mNumVertices; ++i )
  {
    mesh->mVertices[i].y *= -1.0f;

    if( mesh->HasNormals() )
    {
      mesh->mNormals[i].y *= -1.0f;
    }

    if( mesh->HasTangentsAndBitangents() )
    {
      mesh->mTangents[i].y *= -1.0f;
      mesh->mBitangents[i].y *= -1.0f;
      mesh->mBitangents[i] *= -1.0f; // Assimp does both of these - is this required?
    }
  }

  for( std::size_t i=0; i < mesh->mNumBones; ++i )
  {
    aiBone* bone = mesh->mBones[i];
    bone->mOffsetMatrix.a2 = -bone->mOffsetMatrix.a2;
    bone->mOffsetMatrix.c2 = -bone->mOffsetMatrix.c2;
    bone->mOffsetMatrix.d2 = -bone->mOffsetMatrix.d2;
    bone->mOffsetMatrix.b1 = -bone->mOffsetMatrix.b1;
    bone->mOffsetMatrix.b3 = -bone->mOffsetMatrix.b3;
    bone->mOffsetMatrix.b4 = -bone->mOffsetMatrix.b4;
  }
}


void LeftHandConvertor::ConvertMaterial( aiMaterial* material )
{
  // No conversion needed
}

void LeftHandConvertor::ConvertAnimation( aiNodeAnim* animation )
{
  // Convert position channel
  for( std::size_t i=0; i < animation->mNumPositionKeys; ++i )
  {
    animation->mPositionKeys[i].mValue.y *= -1.0f;
  }

  for( std::size_t i=0; i < animation->mNumRotationKeys; ++i )
  {
    // Quick and dirty
    animation->mRotationKeys[i].mValue.x *= -1.0f;
    animation->mRotationKeys[i].mValue.z *= -1.0f;
  }
}

} // SlpPlatform
} // Dali
