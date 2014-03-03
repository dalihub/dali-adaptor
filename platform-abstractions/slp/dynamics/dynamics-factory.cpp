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

// EXTERNAL INCLUDES
#include <dlfcn.h>

#include <dali/integration-api/debug.h>
#include <dali/integration-api/dynamics/dynamics-body-intf.h>
#include <dali/integration-api/dynamics/dynamics-body-settings.h>
#include <dali/integration-api/dynamics/dynamics-joint-intf.h>
#include <dali/integration-api/dynamics/dynamics-shape-intf.h>
#include <dali/integration-api/dynamics/dynamics-world-intf.h>
#include <dali/integration-api/dynamics/dynamics-world-settings.h>

// INTERNAL INCLUDES
#include "dynamics-factory.h"

namespace Dali
{

namespace SlpPlatform
{

namespace
{

const char* SO_BULLET( "libdali-bullet-plugin.so" );

} // unnamed namespace

DynamicsFactory::DynamicsFactory()
: mHandle( NULL ),
  mCreateDynamicsWorld( NULL ),
  mCreateDynamicsBody( NULL ),
  mCreateDynamicsJoint( NULL ),
  mCreateDynamicsShape( NULL )
{
}

DynamicsFactory::~DynamicsFactory()
{
  TerminateDynamics();
}

bool DynamicsFactory::InitializeDynamics( const Integration::DynamicsWorldSettings& worldSettings )
{
  DALI_LOG_INFO(Debug::Filter::gDynamics, Debug::Verbose, "%s - type(%d)\n", __PRETTY_FUNCTION__, worldSettings.type );

  bool result( true );

  if( NULL == mHandle )
  {
    mHandle = dlopen( SO_BULLET, RTLD_NOW | RTLD_GLOBAL );

    if( NULL == mHandle )
    {
      DALI_LOG_ERROR( "dlopen error:'%s'\n", dlerror() );
      result = false;
    }
    else
    {
      mCreateDynamicsWorld = reinterpret_cast<CreateDynamicsWorldFunction>(dlsym( mHandle, "CreateDynamicsWorld"));
      if( !mCreateDynamicsWorld )
      {
        DALI_LOG_ERROR( "Cannot load symbol CreateDynamicsWorld(): dlsym error: %s\n", dlerror() );
        result = false;
      }
      else
      {
        mCreateDynamicsBody = reinterpret_cast<CreateDynamicsBodyFunction>(dlsym( mHandle, "CreateDynamicsBody"));
        if( !mCreateDynamicsBody )
        {
          DALI_LOG_ERROR( "Cannot load symbol CreateDynamicsBody(): dlsym error: %s\n", dlerror() );
          result = false;
        }
        else
        {
          mCreateDynamicsJoint = reinterpret_cast<CreateDynamicsJointFunction>(dlsym( mHandle, "CreateDynamicsJoint"));
          if( !mCreateDynamicsJoint )
          {
            DALI_LOG_ERROR( "Cannot load symbol CreateDynamicsJoint(): dlsym error: %s\n", dlerror() );
            result = false;
          }
          else
          {
            mCreateDynamicsShape = reinterpret_cast<CreateDynamicsShapeFunction>(dlsym( mHandle, "CreateDynamicsShape"));
            if( !mCreateDynamicsShape )
            {
              DALI_LOG_ERROR( "Cannot load symbol CreateDynamicsShape(): dlsym error: %s\n", dlerror() );
              result = false;
            }
          }
        }
      }

      // Couldn't find all required symbols
      if( !result )
      {
        if( dlclose( mHandle ) )
        {
          DALI_LOG_ERROR( "Error closing Dynamics Plugin:'%s': with error %s\n", SO_BULLET, dlerror() );
        }
        mHandle = NULL;
      }
    }
  }

  return result;
}

void DynamicsFactory::TerminateDynamics()
{
  DALI_LOG_INFO(Debug::Filter::gDynamics, Debug::Verbose, "%s\n", __PRETTY_FUNCTION__ );

  if( NULL != mHandle )
  {
    if( dlclose( mHandle ) )
    {
      DALI_LOG_ERROR( "Error closing Dynamics Plugin:'%s': with error %s\n", SO_BULLET, dlerror() );
    }
    mHandle = NULL;
  }
}

Integration::DynamicsWorld* DynamicsFactory::CreateDynamicsWorld()
{
  DALI_LOG_INFO(Debug::Filter::gDynamics, Debug::Verbose, "%s\n", __PRETTY_FUNCTION__ );

  Integration::DynamicsWorld* dynamicsWorld( NULL );

  if( mHandle )
  {
    dynamicsWorld = mCreateDynamicsWorld();
  }

  return dynamicsWorld;
}

Integration::DynamicsBody* DynamicsFactory::CreateDynamicsBody()
{
  DALI_LOG_INFO(Debug::Filter::gDynamics, Debug::Verbose, "%s\n", __PRETTY_FUNCTION__ );

  Integration::DynamicsBody* dynamicsBody( NULL );

  if( mHandle )
  {
    dynamicsBody = mCreateDynamicsBody();
  }

  return dynamicsBody;
}

Integration::DynamicsJoint* DynamicsFactory::CreateDynamicsJoint()
{
  DALI_LOG_INFO(Debug::Filter::gDynamics, Debug::Verbose, "%s\n", __PRETTY_FUNCTION__ );

  Integration::DynamicsJoint* dynamicsJoint( NULL );

  if( mHandle )
  {
    dynamicsJoint = mCreateDynamicsJoint();
  }

  return dynamicsJoint;
}

Integration::DynamicsShape* DynamicsFactory::CreateDynamicsShape()
{
  DALI_LOG_INFO(Debug::Filter::gDynamics, Debug::Verbose, "%s\n", __PRETTY_FUNCTION__ );

  Integration::DynamicsShape* dynamicsShape( NULL );

  if( mHandle )
  {
    dynamicsShape = mCreateDynamicsShape();
  }

  return dynamicsShape;
}

} // namespace SlpPlatform

} // namespace Dali
