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

#define LOG_TAG "DALI_DYNAMICS"

// EXTERNAL INCLUDES
#include <dlog.h>
// TODO: Change this to use #pragma GCC diagnostic push / pop when the compiler is updated to 4.6.0+
#pragma GCC diagnostic ignored "-Wfloat-equal"
# include <btBulletDynamicsCommon.h>
#pragma GCC diagnostic error "-Wfloat-equal"

#include <dali/public-api/dynamics/dynamics-shape.h>
#include <dali/public-api/geometry/mesh-data.h>

// INTERNAL INCLUDES
#include "bullet-dynamics-shape.h"

#define DEBUG_PRINTF(fmt, arg...)  LOGD(fmt, ##arg)

namespace Dali
{

namespace Plugin
{

BulletDynamicsShape::BulletDynamicsShape()
: mShape( NULL ),
  mVertices( NULL ),
  mFaces( NULL )
{
  DEBUG_PRINTF( "%s\n", __PRETTY_FUNCTION__ );
}

BulletDynamicsShape::~BulletDynamicsShape()
{
  DEBUG_PRINTF( "%s\n", __PRETTY_FUNCTION__ );

  delete mFaces;
  delete mVertices;
  delete mShape;
}

void BulletDynamicsShape::Initialize( int type, const Vector3& dimensions )
{
  DEBUG_PRINTF( "BulletDynamicsShape::Initialize( type:%d dimensions(%.2f %.2f %.2f)\n", type, dimensions.width, dimensions.height, dimensions.depth );

  switch( type )
  {
    case Dali::DynamicsShape::CAPSULE:
    {
      mShape = new btCapsuleShape( dimensions.width, dimensions.height );
      break;
    }
    case Dali::DynamicsShape::CONE:
    {
      mShape = new btConeShape( dimensions.width, dimensions.height );
      break;
    }
    case Dali::DynamicsShape::CUBE:
    {
      mShape = new btBoxShape( btVector3(dimensions.width, dimensions.height, dimensions.depth) );
      break;
    }
    case Dali::DynamicsShape::CYLINDER:
    {
      mShape = new btCylinderShape( btVector3(dimensions.width, dimensions.height * 0.5f, dimensions.depth) );
      break;
    }
    case Dali::DynamicsShape::MESH:
    {
      break;
    }
    case Dali::DynamicsShape::SPHERE:
    {
      mShape = new btSphereShape( dimensions.width );
      break;
    }
  }
}

void BulletDynamicsShape::Initialize( int type,
                                      const MeshData::VertexContainer& vertices,
                                      const MeshData::FaceIndices& faces )
{
  DEBUG_PRINTF( "BulletDynamicsShape::Initialize( type:%d )\n", type );

  // copy vertices
  const size_t vertexCount( vertices.size() );

  mVertices = new MeshData::VertexContainer(vertexCount);
  if( 0 != vertexCount )
  {
    mVertices->assign( vertices.begin(), vertices.end() );
  }

  // copy faces
  const size_t faceCount( faces.size() );

  mFaces = new MeshData::FaceIndices(faceCount);
  if( 0 != faceCount )
  {
    mFaces->assign( faces.begin(), faces.end() );
  }
}

btCollisionShape* BulletDynamicsShape::GetShape()
{
  return mShape;
}

} // namespace Plugin

} // namespace Dali
