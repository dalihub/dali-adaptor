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
# include <bullet/btBulletDynamicsCommon.h>
# include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
# include "BulletSoftBody/btSoftBodyHelpers.h"
#pragma GCC diagnostic error "-Wfloat-equal"

// INTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>
#include "bullet-dynamics-body.h"
#include "bullet-dynamics-joint.h"
#include "bullet-dynamics-shape.h"
#include "bullet-dynamics-world.h"

#include <dali/integration-api/dynamics/dynamics-shape-intf.h>

#define DEBUG_PRINTF(fmt, arg...)  LOGD(fmt, ##arg)

extern "C" DALI_EXPORT_API Dali::Integration::DynamicsWorld* CreateDynamicsWorld()
{
  DEBUG_PRINTF( "%s\n", __func__ );

  return new Dali::Plugin::BulletDynamicsWorld();
}

extern "C" DALI_EXPORT_API Dali::Integration::DynamicsBody* CreateDynamicsBody()
{
  DEBUG_PRINTF( "%s\n", __func__ );

  return new Dali::Plugin::BulletDynamicsBody();
}

extern "C" DALI_EXPORT_API Dali::Integration::DynamicsJoint* CreateDynamicsJoint()
{
  DEBUG_PRINTF( "%s\n", __func__ );

  return new Dali::Plugin::BulletDynamicsJoint();
}

extern "C" DALI_EXPORT_API Dali::Integration::DynamicsShape* CreateDynamicsShape()
{
  DEBUG_PRINTF( "%s\n", __func__ );

  return new Dali::Plugin::BulletDynamicsShape();
}

