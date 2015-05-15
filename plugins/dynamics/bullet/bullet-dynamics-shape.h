#ifndef __DALI_PLUGIN_BULLET_DYNAMICS_SHAPE_H__
#define __DALI_PLUGIN_BULLET_DYNAMICS_SHAPE_H__

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

// INTERNAL INCLUDES
#include <dali/integration-api/dynamics/dynamics-shape-intf.h>

#include <dali/devel-api/geometry/mesh-data.h>

class btCollisionShape;

namespace Dali
{

struct Vector3;

namespace Plugin
{

/*
 *
 */
class BulletDynamicsShape : public Integration::DynamicsShape
{
public:
  BulletDynamicsShape();
  virtual ~BulletDynamicsShape();

public:
  // @copydoc Dali::Integration::DynamicsShape::Initialize(int,const Vector3&)
  void Initialize( int type, const Vector3& dimensions );

  // @copydoc Dali::Integration::DynamicsShape::Initialize(int, const MeshData&)
  void Initialize( int type, const MeshData::VertexContainer& vertices, const MeshData::FaceIndices& faceIndices );

public:
  btCollisionShape* GetShape();

private:

  btCollisionShape*          mShape;

public:

  MeshData::VertexContainer* mVertices;
  MeshData::FaceIndices*     mFaces;

}; // class BulletDynamicsShape

} // namespace Plugin

} // namespace Dali

#endif // __DALI_PLUGIN_BULLET_DYNAMICS_SHAPE_H__
