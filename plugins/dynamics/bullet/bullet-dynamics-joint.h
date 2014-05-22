#ifndef __DALI_PLUGIN_BULLET_DYNAMICS_JOINT_H__
#define __DALI_PLUGIN_BULLET_DYNAMICS_JOINT_H__

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
#include <dali/integration-api/dynamics/dynamics-joint-intf.h>

class btGeneric6DofSpringConstraint;
class btTypedConstraint;

namespace Dali
{

namespace Plugin
{

/*
 *
 */
class BulletDynamicsJoint : public Integration::DynamicsJoint
{
public:
  BulletDynamicsJoint();
  virtual ~BulletDynamicsJoint();

public: // From Dali::Integration::DynamicsJoint

  /// @copydoc Dali::Integration::DynamicsJoint::Initialize
  void Initialize( Integration::DynamicsBody* bodyA, const Vector3& positionA, const Quaternion& rotationA, const Vector3& offsetA,
                   Integration::DynamicsBody* bodyB, const Vector3& positionB, const Quaternion& rotationB, const Vector3& offsetB );

  /// @copydoc Dali::Integration::DynamicsJoint::SetLinearLimit
  void SetLimit( const int axisIndex, const float lowerLimit, const float upperLimit );

  /// @copydoc Dali::Integration::DynamicsJoint::EnableSpring
  void EnableSpring( int axisIndex, bool flag );

  /// @copydoc Dali::Integration::DynamicsJoint::SetSpringStiffness
  void SetSpringStiffness( int axisIndex, float stiffness );

  /// @copydoc Dali::Integration::DynamicsJoint::SetSpringDamping
  void SetSpringDamping( int axisIndex, float damping );

  /// @copydoc Dali::Integration::DynamicsJoint::SetSpringCenterPoint
  void SetSpringCenterPoint( int axisIndex, float ratio );

  /// @copydoc Dali::Integration::DynamicsJoint::EnableMotor
  void EnableMotor( int axisIndex, bool flag );

  /// @copydoc Dali::Integration::DynamicsJoint::SetMotorVelocity
  void SetMotorVelocity( int axisIndex, float velocity );

  /// @copydoc Dali::Integration::DynamicsJoint::SetMotorForce
  void SetMotorForce( int axisIndex, float force );

public:

  btTypedConstraint* GetConstraint() const;

private:

  btGeneric6DofSpringConstraint* mConstraint;

}; // class BulletDynamicsJoint

} // namespace Plugin

} // namespace Dali

#endif // __DALI_PLUGIN_BULLET_DYNAMICS_JOINT_H__
