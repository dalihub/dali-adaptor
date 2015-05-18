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

#define LOG_TAG "DALI_DYNAMICS"

// EXTERNAL INCLUDES
#include <dlog.h>
// TODO: Change this to use #pragma GCC diagnostic push / pop when the compiler is updated to 4.6.0+
#pragma GCC diagnostic ignored "-Wfloat-equal"
# include <btBulletDynamicsCommon.h>
# include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
# include "BulletSoftBody/btSoftBodyHelpers.h"
#pragma GCC diagnostic error "-Wfloat-equal"

#include <dali/integration-api/dynamics/dynamics-body-settings.h>
#include <dali/devel-api/geometry/mesh.h>
#include <dali/public-api/math/math-utils.h>

// INTERNAL INCLUDES
#include "bullet-dynamics-body.h"
#include "bullet-dynamics-shape.h"
#include "bullet-dynamics-world.h"

#define DEBUG_PRINTF(fmt, arg...)  LOGD(fmt, ##arg)

namespace Dali
{

namespace Plugin
{

BulletDynamicsBody::BulletDynamicsBody()
: mSettings( NULL ),
  mBody( NULL ),
  mConserveVolume(false),
  mConserveShape(false),
  mKinematic(false),
  mActivationState(false),
  mCollisionGroup(0),
  mCollisionMask(0)
{
  DEBUG_PRINTF("%s\n", __func__);
}

BulletDynamicsBody::~BulletDynamicsBody()
{
  DEBUG_PRINTF("%s\n", __func__);

  delete mBody;
  delete mSettings;
}

void BulletDynamicsBody::Initialize( const Integration::DynamicsBodySettings& settings, Integration::DynamicsShape* shape,
                                     Integration::DynamicsWorld& dynamicsWorld,
                                     const Vector3& startPosition, const Quaternion& startRotation )
{
  DEBUG_PRINTF("%s type:%d\n", __func__, settings.type );

  mSettings = new Integration::DynamicsBodySettings( settings );
  mWorld = static_cast< BulletDynamicsWorld* >( &dynamicsWorld );

  if( Dali::DynamicsBodyConfig::RIGID == mSettings->type )
  {
    CreateRigidBody( settings, shape, startPosition, startRotation );
  }
  else
  {
    CreateSoftBody( settings, shape, startPosition, startRotation );
  }
}

void BulletDynamicsBody::SetMass( float mass )
{
  DEBUG_PRINTF("%s(%.2f)\n", __func__, mass);

  if( Dali::DynamicsBodyConfig::RIGID == mSettings->type )
  {
    btRigidBody* rigidBody( btRigidBody::upcast(mBody) );
    DALI_ASSERT_DEBUG( rigidBody );

    btVector3 inertia(0,0,0);
    if( ! EqualsZero( mass ) )
    {
      btCollisionShape* shape = mBody->getCollisionShape();
      shape->calculateLocalInertia(mass, inertia);
    }
    rigidBody->setMassProps(btScalar(mass), inertia);
  }
  else if( Dali::DynamicsBodyConfig::SOFT == mSettings->type )
  {
    btSoftBody* softBody( btSoftBody::upcast(mBody) );
    DALI_ASSERT_DEBUG( softBody );

    softBody->setTotalMass(mass);
  }
}

void BulletDynamicsBody::SetElasticity( float elasticity )
{
  DEBUG_PRINTF("%s(%.2f)\n", __func__, elasticity);

  static_cast<btRigidBody*>(mBody)->setRestitution(elasticity);
}

void BulletDynamicsBody::SetLinearVelocity( const Vector3& velocity )
{
  DEBUG_PRINTF("%s(%.2f %.2f %.2f)\n", __func__, velocity.x, velocity.y, velocity.z);

  if( Dali::DynamicsBodyConfig::RIGID == mSettings->type )
  {
    static_cast<btRigidBody*>(mBody)->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z) );
  }
  else if( Dali::DynamicsBodyConfig::SOFT == mSettings->type )
  {
    // TODO set velocities of all vertices/nodes in the soft body
  }
}

Vector3 BulletDynamicsBody::GetLinearVelocity() const
{
  const btVector3& linearVelocity( static_cast<btRigidBody*>(mBody)->getLinearVelocity() );

  return Vector3( linearVelocity.x(), linearVelocity.y(), linearVelocity.z() );
}

void BulletDynamicsBody::SetAngularVelocity( const Vector3& velocity )
{
  DEBUG_PRINTF("%s(%.2f %.2f %.2f)\n", __func__, velocity.x, velocity.y, velocity.z);

  if( Dali::DynamicsBodyConfig::RIGID == mSettings->type )
  {
    static_cast<btRigidBody*>(mBody)->setAngularVelocity(btVector3(velocity.x, velocity.y, velocity.z) );
  }
  else if( Dali::DynamicsBodyConfig::SOFT == mSettings->type )
  {
    // TODO set velocities of all vertices/nodes in the soft body
  }
}

Vector3 BulletDynamicsBody::GetAngularVelocity() const
{
  const btVector3& angularVelocity( static_cast<btRigidBody*>(mBody)->getAngularVelocity() );

  return Vector3( angularVelocity.x(), angularVelocity.y(), angularVelocity.z() );
}

void BulletDynamicsBody::SetKinematic( bool flag )
{
  DEBUG_PRINTF("%s( new:%d current:%d)\n", __func__, flag, mKinematic);

  if( flag != mKinematic )
  {
    mKinematic = flag;

    btRigidBody* rigidBody( static_cast< btRigidBody* >( mBody ) );

    if( flag )
    {
      rigidBody->setCollisionFlags( rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
      mBody->setActivationState(DISABLE_DEACTIVATION);
    }
    else
    {
      rigidBody->setCollisionFlags( rigidBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
    }
  }
}

/// @copydoc Dali::DynamicsBody::IsKinematic
bool BulletDynamicsBody::IsKinematic() const
{
  return mKinematic;
}

void BulletDynamicsBody::SetSleepEnabled( bool flag )
{
  DEBUG_PRINTF("%s( %d )\n", __func__, flag);

  if( !flag )
  {
    mBody->setActivationState(DISABLE_DEACTIVATION);
  }
  else
  {
    if( mBody->getActivationState() == DISABLE_DEACTIVATION )
    {
      mBody->forceActivationState(ACTIVE_TAG);
    }
  }
}

void BulletDynamicsBody::WakeUp()
{
  DEBUG_PRINTF("%s()\n", __func__ );

  if( !mKinematic )
  {
    mBody->activate();
    mActivationState = true;
  }
}

void BulletDynamicsBody::AddAnchor( unsigned int index, const Integration::DynamicsBody* anchorBody, bool collisions )
{
  DEBUG_PRINTF("%s\n", __func__);
  const BulletDynamicsBody* anchorDynamicsBody( static_cast< const BulletDynamicsBody* >( anchorBody ) );

  btSoftBody* softBody( static_cast< btSoftBody* >( mBody ) );
  btRigidBody* rigidBody( static_cast< btRigidBody* >( anchorDynamicsBody->GetBody() ) );
  softBody->appendAnchor( index, rigidBody, !collisions );
}

void BulletDynamicsBody::ConserveVolume( bool flag )
{
  if( mConserveVolume != flag )
  {
    mConserveVolume = flag;

    btSoftBody* softBody( static_cast< btSoftBody* >( mBody ) );
    softBody->setPose( mConserveVolume, mConserveShape );
  }
}

void BulletDynamicsBody::ConserveShape( bool flag )
{
  if( mConserveShape != flag )
  {
    mConserveShape = flag;

    btSoftBody* softBody( static_cast<btSoftBody*>(mBody) );
    softBody->setPose( mConserveVolume, mConserveShape );
  }
}

short int BulletDynamicsBody::GetCollisionGroup() const
{
  return mCollisionGroup;
}

void BulletDynamicsBody::SetCollisionGroup( short int collisionGroup )
{
  DEBUG_PRINTF("%s(%d)\n", __func__, collisionGroup);

  mCollisionGroup = collisionGroup;
}


short int BulletDynamicsBody::GetCollisionMask() const
{
  return mCollisionMask;
}

void BulletDynamicsBody::SetCollisionMask( short int collisionMask )
{
  DEBUG_PRINTF("%s(%d)\n", __func__, collisionMask);

  mCollisionMask = collisionMask;
}

int BulletDynamicsBody::GetType() const
{
  return mSettings->type;
}

bool BulletDynamicsBody::IsActive() const
{
  return mBody->getActivationState() == ACTIVE_TAG;
}

void BulletDynamicsBody::SetTransform( const Vector3& position, const Quaternion& rotation )
{
  Vector3 axis;
  float angle( 0.0f );
  rotation.ToAxisAngle( axis, angle );

  if( Dali::DynamicsBodyConfig::RIGID == mSettings->type )
  {
    btTransform& transform( mBody->getWorldTransform() );
    // modify parameters
    transform.setIdentity();
    transform.setOrigin( btVector3(position.x, position.y, position.z) );
    if( axis != Vector3::ZERO )
    {
      transform.setRotation( btQuaternion(btVector3(axis.x, axis.y, axis.z), btScalar(angle)) );
    }
  }
  else if( Dali::DynamicsBodyConfig::SOFT == mSettings->type )
  {
    btSoftBody* softBody( static_cast< btSoftBody* >( mBody ) );
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin( btVector3(position.x, position.y, position.z) );
    if( axis != Vector3::ZERO )
    {
      transform.setRotation( btQuaternion(btVector3(axis.x, axis.y, axis.z), btScalar(angle)) );
    }
    softBody->transform( transform );
  }
}

void BulletDynamicsBody::GetTransform( Vector3& position, Quaternion& rotation )
{
  // get updated parameters
  const btTransform& transform( GetBody()->getWorldTransform() );
  const btVector3& origin( transform.getOrigin() );
  const btQuaternion currentRotation( transform.getRotation() );
  const btVector3& axis( currentRotation.getAxis() );
  const btScalar& angle( currentRotation.getAngle() );

  position = Vector3( origin.x(), origin.y(), origin.z() );
  rotation = Quaternion( float(angle), Vector3( axis.x(), axis.y(), axis.z() ) );
}

void BulletDynamicsBody::GetSoftVertices( MeshData::VertexContainer& vertices ) const
{
  const float worldScale( 1.0f / mWorld->GetWorldScale() );

  // copy positions and normals
  const size_t vertexCount = vertices.size();
  MeshData::Vertex* vertex( &vertices[0] );

  btSoftBody* softBody( static_cast<btSoftBody*>( mBody ) );
  btSoftBody::Node* node( &softBody->m_nodes[0] );
  for( size_t i = 0; i < vertexCount; ++i )
  {
    vertex->x = node->m_x.x() * worldScale;
    vertex->y = node->m_x.y() * worldScale;
    vertex->z = node->m_x.z() * worldScale;

    vertex->nX = node->m_n.x();
    vertex->nY = node->m_n.y();
    vertex->nZ = node->m_n.z();

    ++vertex;
    ++node;
  }
}

btCollisionObject* BulletDynamicsBody::GetBody() const
{
  return mBody;
}

void BulletDynamicsBody::CreateRigidBody( const Integration::DynamicsBodySettings& bodySettings, Dali::Integration::DynamicsShape* shape,
                                          const Vector3& startPosition, const Quaternion& startRotation )
{
  DEBUG_PRINTF("%s\n", __func__ );

  Vector3 axis;
  float angle( 0.0f );
  startRotation.ToAxisAngle( axis, angle );

  btVector3 inertia( 0.0f ,0.0f ,0.0f );
  btCollisionShape* collisionShape( ((BulletDynamicsShape*)shape)->GetShape() );
  if( ! EqualsZero( bodySettings.mass ) )
  {
    collisionShape->calculateLocalInertia(bodySettings.mass, inertia);
  }

  btRigidBody::btRigidBodyConstructionInfo constructionInfo(bodySettings.mass, NULL, collisionShape, inertia);
  constructionInfo.m_restitution = bodySettings.elasticity;
  constructionInfo.m_linearDamping = bodySettings.linearDamping;
  constructionInfo.m_angularDamping = bodySettings.angularDamping;
  constructionInfo.m_friction = bodySettings.friction;
  constructionInfo.m_linearSleepingThreshold = bodySettings.linearSleepVelocity;
  constructionInfo.m_angularSleepingThreshold = bodySettings.angularSleepVelocity;
  constructionInfo.m_startWorldTransform.setOrigin(btVector3(startPosition.x, startPosition.y, startPosition.z));
  if( axis != Vector3::ZERO )
  {
    constructionInfo.m_startWorldTransform.setRotation( btQuaternion(btVector3(axis.x, axis.y, axis.z), btScalar(angle)) );
  }

  mBody = new btRigidBody( constructionInfo );
  mBody->setUserPointer(this);
}

void BulletDynamicsBody::CreateSoftBody( const Integration::DynamicsBodySettings& bodySettings, Dali::Integration::DynamicsShape* shape,
                                         const Vector3& startPosition, const Quaternion& startRotation )
{
  DEBUG_PRINTF("BulletDynamicsBody::CreateSoftBody()\n" );

  BulletDynamicsShape* dynamicsShape( static_cast< BulletDynamicsShape* >( shape ) );

  // copy positions
  MeshData::VertexContainer* vertices( dynamicsShape->mVertices );
  const size_t vertexCount = vertices->size();
  MeshData::Vertex* vertex( &(*vertices)[0] );

  MeshData::FaceIndices* faces( dynamicsShape->mFaces );
  const size_t faceCount = faces->size() / 3;
  unsigned short* faceIndex = ( &(*faces)[0] );

  const float worldScale( mWorld->GetWorldScale() );

  DEBUG_PRINTF("%s verts:%d faces:%d worldScale:%.2f\n", __func__, (int)vertexCount, (int)faceCount, worldScale );

  // copy vertex positions
  std::vector<btVector3> positions( vertexCount );
  btVector3* position = &positions[0];
  for( size_t i = 0; i < vertexCount; ++i )
  {
    *position++ = btVector3( vertex->x, vertex->y, vertex->z ) * worldScale;
    vertex++;
  }

  btSoftBody* softBody( new btSoftBody(mWorld->GetSoftBodyWorldInfo(), vertexCount, positions.data(), NULL) );

  // Add faces
  for( size_t i = 0; i < faceCount; ++i)
  {
    softBody->appendFace(faceIndex[0], faceIndex[1], faceIndex[2]);
    softBody->appendLink(faceIndex[0], faceIndex[1], NULL, true);
    softBody->appendLink(faceIndex[0], faceIndex[2], NULL, true);
    softBody->appendLink(faceIndex[1], faceIndex[2], NULL, true);
    faceIndex += 3;
  }

  softBody->m_cfg.kAHR = std::max(0.0f, std::min(1.0f, bodySettings.anchorHardness));
  softBody->m_cfg.kVC  = std::max(0.0f, bodySettings.volumeConservation);
  softBody->m_cfg.kMT  = std::max(0.0f, std::min(1.0f, bodySettings.shapeConservation));
  softBody->m_cfg.kDP  = Clamp( bodySettings.linearDamping, 0.0f, 1.0f );

  btSoftBody::Material* material( softBody->m_materials[0] );
  material->m_kLST = bodySettings.linearStiffness;

//  material->m_flags &= ~btSoftBody::fMaterial::DebugDraw;           // By default, inhibit debug draw for softbodies
  softBody->m_cfg.collisions = btSoftBody::fCollision::SDF_RS |
                               btSoftBody::fCollision::CL_SS |
                               0;

  softBody->getCollisionShape()->setMargin(0.5f);

  mBody = softBody;
  mBody->setUserPointer(this);

  SetTransform( startPosition, startRotation );
}

} // namespace Plugin

} // namespace Dali
