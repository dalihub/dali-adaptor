#ifndef __DALI_PLUGIN_BULLET_DYNAMICS_WORLD_H__
#define __DALI_PLUGIN_BULLET_DYNAMICS_WORLD_H__

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

// EXTERNAL INCLUDES
// INTERNAL INCLUDES
#include <dali/integration-api/dynamics/dynamics-world-intf.h>

// forward declarations
class btCollisionWorld;
class btConstraintSolver;
class btBroadphaseInterface;
class btCollisionDispatcher;
class btCollisionConfiguration;
class btSoftBodyWorldInfo;

namespace Dali
{

namespace Integration
{

struct DynamicsWorldSettings;
class DynamicsBody;
class DynamicsJoint;

} // namespace Integration

namespace Plugin
{

class BulletDebugDraw;

/*
 *
 */
class BulletDynamicsWorld : public Dali::Integration::DynamicsWorld
{
public:
  BulletDynamicsWorld();
  virtual ~BulletDynamicsWorld();

  /// @copydoc Dali::Integration::DynamicsWorld::InitializeWorld
  void Initialize( const Dali::Integration::DynamicsWorldSettings& worldSettings );

  /// @copydoc Dali::Integration::DynamicsWorld::AddBody
  void AddBody( Dali::Integration::DynamicsBody* body );

  /// @copydoc Dali::Integration::DynamicsWorld::RemoveBody
  void RemoveBody( Dali::Integration::DynamicsBody* body );

  /// @copydoc Dali::Integration::DynamicsWorld::AddJoint
  void AddJoint( Dali::Integration::DynamicsJoint* joint );

  /// @copydoc Dali::Integration::DynamicsWorld::RemoveJoint
  void RemoveJoint( Dali::Integration::DynamicsJoint* joint );

  /// @copydoc Dali::Integration::DynamicsWorld::SetGravity
  void SetGravity( const Vector3& gravity );

  /// @copydoc Dali::DynamicsWorld::SetDebugDrawMode
  void SetDebugDrawMode(int mode);

  /// @copydoc Dali::Integration::DynamicsWorld::DebugDraw
  const Integration::DynamicsDebugVertexContainer& DebugDraw();

  /// @copydoc Dali::Integration::DynamicsWorld::Update
  void Update( float elapsedSeconds );

  /// @copydoc Dali::Integration::DynamicsWorld::CheckForCollisions
  void CheckForCollisions( Integration::CollisionDataContainer& contacts );

public:

  float GetWorldScale() const;

  btSoftBodyWorldInfo* GetSoftBodyWorldInfo();

private:
  Dali::Integration::DynamicsWorldSettings* mSettings;
  btCollisionWorld* mDynamicsWorld;
  btConstraintSolver* mSolver;
  btBroadphaseInterface* mBroadphase;
  btCollisionDispatcher* mDispatcher;
  btCollisionConfiguration* mCollisionConfiguration;
  btSoftBodyWorldInfo* mSoftBodyWorldInfo;

  BulletDebugDraw* mDebugDraw;
}; // class BulletDynamicsWorld

} // namespace Plugin

} // namespace Dali

#endif // __DALI_PLUGIN_BULLET_DYNAMICS_WORLD_H__
