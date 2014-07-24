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
#include <btBulletDynamicsCommon.h>
#pragma GCC diagnostic error "-Wfloat-equal"

#include <dali/public-api/math/quaternion.h>
#include <dali/public-api/math/vector3.h>

// INTERNAL INCLUDES
#include "bullet-dynamics-joint.h"
#include "bullet-dynamics-body.h"

#define DEBUG_PRINTF(fmt, arg...)  LOGD(fmt, ##arg)

namespace Dali
{

namespace Plugin
{

namespace
{

const int RotationAxis( 3 );

} // unnamed namespace

BulletDynamicsJoint::BulletDynamicsJoint()
{
  DEBUG_PRINTF( "%s()\n ", __PRETTY_FUNCTION__ );
}

BulletDynamicsJoint::~BulletDynamicsJoint()
{
  DEBUG_PRINTF( "%s()\n ", __PRETTY_FUNCTION__ );
}

void BulletDynamicsJoint::Initialize( Integration::DynamicsBody* bodyA, const Vector3& positionA, const Quaternion& rotationA, const Vector3& offsetA,
                                      Integration::DynamicsBody* bodyB, const Vector3& positionB, const Quaternion& rotationB, const Vector3& offsetB )
{
  DEBUG_PRINTF( "%s\n ", __PRETTY_FUNCTION__ );

  Vector3 axis;
  float angle( 0.0f );
  btTransform transformA;
  btTransform transformB;

  // Convert points and orientations to btTransforms
  transformA.setIdentity();
  transformA.setOrigin( btVector3( offsetA.x, offsetA.y, offsetA.z ) );
  rotationA.ToAxisAngle( axis, angle );
  if( axis != Vector3::ZERO )
  {
    transformA.setRotation( btQuaternion( btVector3( axis.x, axis.y, axis.z ), btScalar(angle) ) );
  }

  // Convert points and orientations to btTransforms
  transformB.setIdentity();
  transformB.setOrigin( btVector3( offsetB.x, offsetB.y, offsetB.z ) );
  rotationB.ToAxisAngle( axis, angle );
  if( axis != Vector3::ZERO )
  {
    transformB.setRotation( btQuaternion( btVector3( axis.x, axis.y, axis.z ), btScalar(angle) ) );
  }

  BulletDynamicsBody* dynamicsBodyA( static_cast< BulletDynamicsBody* >( bodyA ) );
  BulletDynamicsBody* dynamicsBodyB( static_cast< BulletDynamicsBody* >( bodyB ) );

  btRigidBody& rigidBodyA( static_cast< btRigidBody& >( *(dynamicsBodyA->GetBody()) ) );
  btRigidBody& rigidBodyB( static_cast< btRigidBody& >( *(dynamicsBodyB->GetBody()) ) );

  // create constraint
  mConstraint = new btGeneric6DofSpringConstraint(rigidBodyA, rigidBodyB, transformA, transformB, true);

  // default to no translation and no rotation
  mConstraint->setLinearLowerLimit(btVector3(0.0f, 0.0f, 0.0f));
  mConstraint->setLinearUpperLimit(btVector3(0.0f, 0.0f, 0.0f));
  mConstraint->setAngularLowerLimit(btVector3(0.0f, 0.0f, 0.0f));
  mConstraint->setAngularUpperLimit(btVector3(0.0f, 0.0f, 0.0f));
}

void BulletDynamicsJoint::SetLimit( int axisIndex, float lowerLimit, float upperLimit )
{
  DEBUG_PRINTF( "%s( %d %.2f %.2f)\n ", __PRETTY_FUNCTION__, axisIndex, lowerLimit, upperLimit );

  mConstraint->setLimit( axisIndex, lowerLimit, upperLimit );
}

void BulletDynamicsJoint::EnableSpring( int axisIndex, bool flag )
{
  DEBUG_PRINTF( "%s( %d %d)\n ", __PRETTY_FUNCTION__, axisIndex, flag );

  mConstraint->enableSpring( axisIndex, flag );
}

void BulletDynamicsJoint::SetSpringStiffness( int axisIndex, float stiffness )
{
  DEBUG_PRINTF( "%s( %d %.2f)\n ", __PRETTY_FUNCTION__, axisIndex, stiffness );

  mConstraint->setStiffness( axisIndex, stiffness );
}

void BulletDynamicsJoint::SetSpringDamping( int axisIndex, float damping )
{
  DEBUG_PRINTF( "%s\n ", __PRETTY_FUNCTION__ );

  mConstraint->setDamping( axisIndex, 1.0f - damping );
}

void BulletDynamicsJoint::SetSpringCenterPoint( int axisIndex, float ratio )
{
  DEBUG_PRINTF( "%s( %d %.2f)\n ", __PRETTY_FUNCTION__, axisIndex, ratio );

  float equilibriumPoint( 0.0f );

  if( axisIndex < RotationAxis )
  {
    btVector3 translationLowerLimit;
    btVector3 translationUpperLimit;
    mConstraint->getLinearLowerLimit( translationLowerLimit );
    mConstraint->getLinearUpperLimit( translationUpperLimit );
    equilibriumPoint = translationLowerLimit[axisIndex] + ((translationUpperLimit[axisIndex] - translationLowerLimit[axisIndex]) * ratio);
  }
  else
  {
    const int axis( axisIndex - RotationAxis );
    btVector3 rotationLowerLimit;
    btVector3 rotationUpperLimit;
    mConstraint->getAngularLowerLimit( rotationLowerLimit );
    mConstraint->getAngularUpperLimit( rotationUpperLimit );
    equilibriumPoint = rotationLowerLimit[axis] + ((rotationUpperLimit[axis] - rotationLowerLimit[axis]) * ratio);
  }

  mConstraint->setEquilibriumPoint( axisIndex, equilibriumPoint );
}

void BulletDynamicsJoint::EnableMotor( int axisIndex, bool flag )
{
  DEBUG_PRINTF( "%s( %d %d)\n ", __PRETTY_FUNCTION__, axisIndex, flag );

  if( axisIndex < RotationAxis )
  {
    mConstraint->getTranslationalLimitMotor()->m_enableMotor[axisIndex] = flag;
  }
  else
  {
    mConstraint->getRotationalLimitMotor(axisIndex - RotationAxis)->m_enableMotor = flag;
  }
}

void BulletDynamicsJoint::SetMotorVelocity( int axisIndex, float velocity )
{
  DEBUG_PRINTF( "%s - %d %.2f\n ", __PRETTY_FUNCTION__, axisIndex, velocity );

  if( axisIndex < RotationAxis )
  {
    mConstraint->getTranslationalLimitMotor()->m_targetVelocity[axisIndex] = velocity;
  }
  else
  {
    mConstraint->getRotationalLimitMotor(axisIndex - RotationAxis)->m_targetVelocity = velocity;
  }
}

void BulletDynamicsJoint::SetMotorForce( int axisIndex, float force )
{
  DEBUG_PRINTF( "%s - %d %.2f\n ", __PRETTY_FUNCTION__, axisIndex, force );

  if( axisIndex < RotationAxis )
  {
    mConstraint->getTranslationalLimitMotor()->m_maxMotorForce[axisIndex] = force;
  }
  else
  {
    mConstraint->getRotationalLimitMotor(axisIndex - RotationAxis)->m_maxMotorForce = force;
  }
}

btTypedConstraint* BulletDynamicsJoint::GetConstraint() const
{
  return mConstraint;
}

} // namespace Plugin

} // namespace Dali
