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
#include <btBulletDynamicsCommon.h>
#pragma GCC diagnostic error "-Wfloat-equal"

// INTERNAL INCLUDES
#include "bullet-dynamics-debug.h"

#define DEBUG_PRINTF(fmt, arg...)  LOGD(fmt, ##arg)

namespace Dali
{

namespace Plugin
{

BulletDebugDraw::BulletDebugDraw()
: mDebugMode(0)
{
  DEBUG_PRINTF( "%s\n", __PRETTY_FUNCTION__ );
}

BulletDebugDraw::~BulletDebugDraw()
{
  DEBUG_PRINTF( "%s\n", __PRETTY_FUNCTION__ );
}

void BulletDebugDraw::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
{
//  DEBUG_PRINTF( "%s\n", __PRETTY_FUNCTION__ );

  Integration::DynamicsDebugVertex vertex;
  vertex.position = Vector3( from.getX(), from.getY(), from.getZ() );
  vertex.color = Vector4( color.getX(), color.getY(), color.getZ(), 1.0f );
  mVertices.push_back( vertex );
  vertex.position = Vector3( to.getX(), to.getY(), to.getZ());
  mVertices.push_back( vertex );
}

void BulletDebugDraw::drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
  btVector3 to = PointOnB + (normalOnB * distance);// * 10.0f;

  drawLine(PointOnB, to, color);
}

void BulletDebugDraw::reportErrorWarning(const char* warningString)
{
  DEBUG_PRINTF( "Dynamics error: %s\n", warningString );
}

void BulletDebugDraw::draw3dText(const btVector3& location,const char* textString)
{
}

void BulletDebugDraw::setDebugMode(int debugMode)
{
  mDebugMode = debugMode;
}

int BulletDebugDraw::getDebugMode() const
{
  return mDebugMode;
}

void BulletDebugDraw::ClearVertices()
{
  mVertices.clear();
}

const Integration::DynamicsDebugVertexContainer& BulletDebugDraw::GetVertices() const
{
  return mVertices;
}

} // namespace Plugin

} // namespace Dali
