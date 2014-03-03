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
#include <bullet/btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#pragma GCC diagnostic error "-Wfloat-equal"

#include <dali/integration-api/dynamics/dynamics-collision-data.h>
#include <dali/integration-api/dynamics/dynamics-world-settings.h>
#include <dali/public-api/dynamics/dynamics-body-config.h>
#include <dali/public-api/dynamics/dynamics-world-config.h>
#include <dali/public-api/dynamics/dynamics-world.h>

// INTERNAL INCLUDES
#include "bullet-dynamics-body.h"
#include "bullet-dynamics-debug.h"
#include "bullet-dynamics-joint.h"
#include "bullet-dynamics-world.h"

#define DEBUG_PRINTF(fmt, arg...)  LOGD(fmt, ##arg)

namespace Dali
{

namespace Plugin
{

BulletDynamicsWorld::BulletDynamicsWorld()
: Integration::DynamicsWorld(),
  mSettings( NULL ),
  mDynamicsWorld( NULL ),
  mSolver( NULL ),
  mBroadphase( NULL ),
  mDispatcher( NULL ),
  mCollisionConfiguration( NULL ),
  mSoftBodyWorldInfo( NULL )
{
  printf( "%s\n", __PRETTY_FUNCTION__ );
}

BulletDynamicsWorld::~BulletDynamicsWorld()
{
  printf( "%s\n", __PRETTY_FUNCTION__ );
  delete mSettings;

  // Destroy simulation
  delete mDynamicsWorld;
  delete mSolver;
  delete mBroadphase;
  delete mDispatcher;
  delete mCollisionConfiguration;
  delete mDebugDraw;
}

void BulletDynamicsWorld::Initialize( const Dali::Integration::DynamicsWorldSettings& worldSettings )
{
  mSettings = new Dali::Integration::DynamicsWorldSettings( worldSettings );

  const bool softWorld( Dali::DynamicsWorldConfig::SOFT == mSettings->type );

  if( softWorld )
  {
    mCollisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
  }
  else // Dali::DynamicsWorldConfig::RIGID
  {
    mCollisionConfiguration = new btDefaultCollisionConfiguration();
  }

  mSolver = new btSequentialImpulseConstraintSolver();

  mDispatcher = new btCollisionDispatcher( mCollisionConfiguration );
  mBroadphase = new btDbvtBroadphase();

  const Vector3& gravity = mSettings->gravity;

  if( softWorld )
  {
    mDynamicsWorld = new btSoftRigidDynamicsWorld( mDispatcher, mBroadphase, mSolver, mCollisionConfiguration, NULL );
    mSoftBodyWorldInfo = &static_cast<btSoftRigidDynamicsWorld*>(mDynamicsWorld)->getWorldInfo();
    mSoftBodyWorldInfo->m_dispatcher = mDispatcher;
    mSoftBodyWorldInfo->m_broadphase = mBroadphase;
    mSoftBodyWorldInfo->air_density = mSettings->airDensity;
    mSoftBodyWorldInfo->water_density = mSettings->waterDensity;
    mSoftBodyWorldInfo->water_offset = mSettings->waterOffset;
    mSoftBodyWorldInfo->water_normal.setValue( mSettings->waterNormal.x, mSettings->waterNormal.y, mSettings->waterNormal.z );
    mSoftBodyWorldInfo->m_gravity.setValue( gravity.x, gravity.y, gravity.z );
    mSoftBodyWorldInfo->m_sparsesdf.Initialize();
    DEBUG_PRINTF("Created soft/rigid dynamics world\n");
  }
  else
  {
    mDynamicsWorld = new btDiscreteDynamicsWorld( mDispatcher, mBroadphase, mSolver, mCollisionConfiguration );
    DEBUG_PRINTF("Created rigid dynamics world\n");
  }

  btDynamicsWorld* world( static_cast<btDynamicsWorld*>(mDynamicsWorld) );

  world->setGravity( btVector3(gravity.x, gravity.y, gravity.z) );
  world->getSolverInfo().m_splitImpulse = true;

  mDebugDraw = new BulletDebugDraw();
  mDynamicsWorld->setDebugDrawer(mDebugDraw);
}

void BulletDynamicsWorld::AddBody( Dali::Integration::DynamicsBody* body )
{
  DEBUG_PRINTF( "Adding a body\n" );

  BulletDynamicsBody* dynamicsBody( static_cast< BulletDynamicsBody* >( body ) );

  if( Dali::DynamicsBodyConfig::RIGID == dynamicsBody->GetType() )
  {
    static_cast<btDynamicsWorld*>(mDynamicsWorld)->addRigidBody( static_cast<btRigidBody*>(dynamicsBody->GetBody()),
                                                                 dynamicsBody->GetCollisionGroup(), dynamicsBody->GetCollisionMask() );
  }
  else if( Dali::DynamicsBodyConfig::SOFT == dynamicsBody->GetType() )
  {
    if( Dali::DynamicsWorldConfig::SOFT == mSettings->type )
    {
      static_cast<btSoftRigidDynamicsWorld*>(mDynamicsWorld)->addSoftBody( static_cast<btSoftBody*>(dynamicsBody->GetBody()),
                                                                           dynamicsBody->GetCollisionGroup(), dynamicsBody->GetCollisionMask() );
    }
  }
}

void BulletDynamicsWorld::RemoveBody( Dali::Integration::DynamicsBody* body )
{
  DEBUG_PRINTF( "Removing a body\n" );

  BulletDynamicsBody* dynamicsBody( static_cast< BulletDynamicsBody* >( body ) );

  // remove from simulation
  if( Dali::DynamicsBodyConfig::RIGID == body->GetType() )
  {
    static_cast<btDynamicsWorld*>(mDynamicsWorld)->removeRigidBody( static_cast<btRigidBody*>( dynamicsBody->GetBody() ) );
  }
  else
  {
    if( Dali::DynamicsWorldConfig::SOFT == mSettings->type )
    {
      static_cast<btSoftRigidDynamicsWorld*>(mDynamicsWorld)->removeSoftBody( static_cast<btSoftBody*>( dynamicsBody->GetBody() ) );
    }
  }
}

void BulletDynamicsWorld::AddJoint( Dali::Integration::DynamicsJoint* joint )
{
  DEBUG_PRINTF( "Adding a joint\n" );

  const BulletDynamicsJoint* dynamicsJoint( static_cast< BulletDynamicsJoint* >( joint ) );

  // Add to simulation
  static_cast<btDynamicsWorld*>(mDynamicsWorld)->addConstraint( dynamicsJoint->GetConstraint(), true );
}

void BulletDynamicsWorld::RemoveJoint( Dali::Integration::DynamicsJoint* joint )
{
  DEBUG_PRINTF("Removing a joint\n" );

  const BulletDynamicsJoint* dynamicsJoint( static_cast< BulletDynamicsJoint* >( joint ) );
  // remove from simulation
  static_cast<btDynamicsWorld*>(mDynamicsWorld)->removeConstraint( dynamicsJoint->GetConstraint() );
}

void BulletDynamicsWorld::SetGravity( const Vector3& gravity )
{
  DEBUG_PRINTF("%s( %.2f %.2f %.2f )\n", __func__, gravity.x, gravity.y, gravity.z );
  static_cast<btDynamicsWorld*>(mDynamicsWorld)->setGravity( btVector3(gravity.x, gravity.y, gravity.z) );
}

void BulletDynamicsWorld::SetDebugDrawMode(int mode)
{
  const int internalMode = ( (mode & Dali::DynamicsWorld::DEBUG_MODE_WIREFRAME)          ? btIDebugDraw::DBG_DrawWireframe : 0 )        |
                           ( (mode & Dali::DynamicsWorld::DEBUG_MODE_FAST_WIREFRAME)     ? btIDebugDraw::DBG_FastWireframe : 0 )        |
                           ( (mode & Dali::DynamicsWorld::DEBUG_MODE_AABB)               ? btIDebugDraw::DBG_DrawAabb : 0 )             |
                           ( (mode & Dali::DynamicsWorld::DEBUG_MODE_CONTACT_POINTS)     ? btIDebugDraw::DBG_DrawContactPoints : 0 )    |
                           ( (mode & Dali::DynamicsWorld::DEBUG_MODE_NO_DEACTIVATION)    ? btIDebugDraw::DBG_NoDeactivation : 0 )       |
                           ( (mode & Dali::DynamicsWorld::DEBUG_MODE_CONSTRAINTS)        ? btIDebugDraw::DBG_DrawConstraints : 0 )      |
                           ( (mode & Dali::DynamicsWorld::DEBUG_MODE_CONSTRAINTS_LIMITS) ? btIDebugDraw::DBG_DrawConstraintLimits : 0 ) |
                           ( (mode & Dali::DynamicsWorld::DEBUG_MODES_NORMALS)           ? btIDebugDraw::DBG_DrawNormals : 0 );

  mDebugDraw->setDebugMode( internalMode );
}

const Integration::DynamicsDebugVertexContainer& BulletDynamicsWorld::DebugDraw()
{
//  DEBUG_PRINTF("%s()\n", __func__ );
  mDebugDraw->ClearVertices();

  mDynamicsWorld->debugDrawWorld();

  return mDebugDraw->GetVertices();
}


void BulletDynamicsWorld::Update( float elapsedSeconds )
{
//  DEBUG_PRINTF("Stepping simulation by %.3fms\n", elapsedSeconds * 1e3f );

  const float timeStep( mSettings->subSteps );
  const float fixedTimeStep( (1.0f/60.0f) / (mSettings->subSteps ? mSettings->subSteps : 1) );

  static_cast<btDynamicsWorld*>(mDynamicsWorld)->stepSimulation( elapsedSeconds, timeStep, fixedTimeStep );
}

void BulletDynamicsWorld::CheckForCollisions( Integration::CollisionDataContainer& contacts )
{
  /* Browse all collision pairs */
  int numManifolds = mDynamicsWorld->getDispatcher()->getNumManifolds();
  for( int manifold = 0; manifold < numManifolds; ++manifold )
  {
    btPersistentManifold* contactManifold = mDynamicsWorld->getDispatcher()->getManifoldByIndexInternal( manifold );
    btCollisionObject* objectA = static_cast< btCollisionObject* >(contactManifold->getBody0());
    btCollisionObject* objectB = static_cast< btCollisionObject* >(contactManifold->getBody1());

    /* Check all contacts points */
    int numContacts = contactManifold->getNumContacts();
    for( int contact = 0; contact < numContacts; ++contact )
    {
      btManifoldPoint& point = contactManifold->getContactPoint( contact );
      if( point.getDistance() < 0.0f )
      {
        const btVector3& pointOnA = point.m_localPointA;
        const btVector3& pointOnB = point.m_localPointB;
        const btVector3& normalOnB = point.m_normalWorldOnB;
        const float impulse = point.m_appliedImpulse;

        if( contacts.find( objectB ) == contacts.end() )
        {
          DALI_ASSERT_DEBUG(NULL != objectA);
          DALI_ASSERT_DEBUG(NULL != objectB);
          DALI_ASSERT_DEBUG(NULL != objectA->getUserPointer());
          DALI_ASSERT_DEBUG(NULL != objectB->getUserPointer());

          contacts[ objectB ] = Integration::DynamicsCollisionData(static_cast< Integration::DynamicsBody* >( objectA->getUserPointer() ),
                                                                  static_cast< Integration::DynamicsBody* >( objectB->getUserPointer() ),
                                                                  Vector3(pointOnA), Vector3(pointOnB), Vector3(normalOnB), impulse);
        }
      }
    }
  }
}

float BulletDynamicsWorld::GetWorldScale() const
{
  return mSettings->worldScale;
}

btSoftBodyWorldInfo* BulletDynamicsWorld::GetSoftBodyWorldInfo()
{
  return mSoftBodyWorldInfo;
}

} // namespace Plugin

} // namespace Dali
