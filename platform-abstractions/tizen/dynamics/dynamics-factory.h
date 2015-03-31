#ifndef __DYNAMICS_FACTORY_H__
#define __DYNAMICS_FACTORY_H__

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
#include <dali/integration-api/dynamics/dynamics-factory-intf.h>

// INTERNAL INCLUDES

namespace Dali
{

namespace TizenPlatform
{

/**
 *
 */
class DynamicsFactory : public Integration::DynamicsFactory
{
public:
  /**
   * Constructor
   */
  DynamicsFactory();

  /**
   * Destructor
   */
  virtual ~DynamicsFactory();

  /**
   * @copydoc Dali::Integration::DynamicsFactory::InitializeDynamics
   */
  bool InitializeDynamics( const Integration::DynamicsWorldSettings& worldSettings );

  /**
   * @copydoc Dali::Integration::DynamicsFactory::TerminateDynamics
   */
  void TerminateDynamics();

  /**
   * @copydoc Dali::Integration::DynamicsFactory::CreateDynamicsWorld
   */
  Integration::DynamicsWorld* CreateDynamicsWorld();

  /**
   * @copydoc Dali::Integration::DynamicsFactory::CreateDynamicsBody
   */
  virtual Integration::DynamicsBody* CreateDynamicsBody();

  /**
   * @copydoc Dali::Integration::DynamicsFactory::CreateDynamicsJoint
   */
  virtual Integration::DynamicsJoint* CreateDynamicsJoint();

  /**
   * @copydoc Dali::Integration::DynamicsFactory::CreateDynamicsShape
   */
  virtual Integration::DynamicsShape* CreateDynamicsShape();

private:
  void* mHandle;    ///< The handle to the open shared object library

  typedef Integration::DynamicsWorld* (*CreateDynamicsWorldFunction)();
  typedef Integration::DynamicsBody* (*CreateDynamicsBodyFunction)();
  typedef Integration::DynamicsJoint* (*CreateDynamicsJointFunction)();
  typedef Integration::DynamicsShape* (*CreateDynamicsShapeFunction)();

  CreateDynamicsWorldFunction mCreateDynamicsWorld;
  CreateDynamicsBodyFunction mCreateDynamicsBody;
  CreateDynamicsJointFunction mCreateDynamicsJoint;
  CreateDynamicsShapeFunction mCreateDynamicsShape;

}; // class DynamicsFactory

} // namespace TizenPlatform

} // namespace Dali

#endif // __DYNAMICS_FACTORY_H__
