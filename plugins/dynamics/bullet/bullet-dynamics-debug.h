#ifndef __DALI_PLUGIN_BULLET_DYNAMICS_DEBUG_H__
#define __DALI_PLUGIN_BULLET_DYNAMICS_DEBUG_H__

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

// EXTERNAL HEADERS
#include <dali/integration-api/dynamics/dynamics-debug-vertex.h>
// TODO: Change this to use #pragma GCC diagnostic push / pop when the compiler is updated to 4.6.0+
#pragma GCC diagnostic ignored "-Wfloat-equal"
#include <bullet/btBulletDynamicsCommon.h>
#pragma GCC diagnostic error "-Wfloat-equal"

namespace Dali
{

namespace Plugin
{

/**
 * Debug draw class, BulletPhysics engine will invoke methods on this object to render debug information.
 * Debug lines are collated into a container and then rendered as a single array during Core::Render
 */
class BulletDebugDraw : public btIDebugDraw
{
public:
  /**
   * Constructor
   */
  BulletDebugDraw();

  /**
   * Destructor
   */
  virtual ~BulletDebugDraw();

  virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color);
  virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
  virtual void reportErrorWarning(const char* warningString);
  virtual void draw3dText(const btVector3& location,const char* textString);
  virtual void setDebugMode(int debugMode);
  virtual int  getDebugMode() const;

  void ClearVertices();
  const Integration::DynamicsDebugVertexContainer& GetVertices() const;

private:

  int mDebugMode;
  Integration::DynamicsDebugVertexContainer mVertices;

}; // class BulletDebugDraw

} // namespace Plugin

} // namespace Dali

#endif /* __DALI_PLUGIN_BULLET_DYNAMICS_DEBUG_H__ */
