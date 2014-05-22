#ifndef __DALI_SLP_PLATFORM_LEFT_HAND_CONVERTOR_H__
#define __DALI_SLP_PLATFORM_LEFT_HAND_CONVERTOR_H__

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

#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>

namespace Dali
{
namespace SlpPlatform
{

/**
 * Converts right handed assimp scene into our left handed coordinate system
 * (X right, Y up, Z forward) --> (X right, Y down, Z forward)
 */
class LeftHandConvertor
{
public:
  /**
   * Constructor
   * @param[in] aiScene The loaded assimp scene
   */
  LeftHandConvertor(const aiScene* aiScene);

  /**
   * Destructor
   */
  ~LeftHandConvertor();

  /**
   * Convert the scene
   */
  void ProcessScene();

private:
  /**
   * Convert nodes & children
   * @param[in] node The node
   */
  void ConvertNode( aiNode* node );

  /**
   * Convert meshes
   * @param[in] mesh The mesh
   */
  void ConvertMesh( aiMesh* mesh );

  /**
   * Convert material
   * @param[in] material The material
   */
  void ConvertMaterial( aiMaterial* material );

  /**
   * Convert animations
   * @param[in] nodeAnimation The node animation
   */
  void ConvertAnimation( aiNodeAnim* nodeAnimation );

private:
  const aiScene* mAssimpScene; ///< The assimp scene
};

} // SlpPlatform
} // Dali

#endif //__DALI_SLP_PLATFORM_LEFT_HAND_CONVERTOR_H__
