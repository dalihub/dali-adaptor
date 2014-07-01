#ifndef __DALI_INTERNAL_GL_PROXY_IMPLEMENTATION_H__
#define __DALI_INTERNAL_GL_PROXY_IMPLEMENTATION_H__

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

#include <internal/common/gl/gl-implementation.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class EnvironmentOptions;


class Sampler
{
public:
  Sampler();
  void  Reset();
  void  Accumulate(float value);
  float GetMeanValue();
  float GetStandardDeviation();
  float GetMin();
  float GetMax();

private:
  float mAccumulated;
  float mAccumulatedSquare;
  float mMin;
  float mMax;
  int   mNumSamples;
};

/**
 * GlProxyImplementation is a wrapper for the concrete implementation
 * of GlAbstraction that also gathers statistical information.
 */
class GlProxyImplementation : public GlImplementation
{
public:
  GlProxyImplementation(EnvironmentOptions& environmentOptions);

  virtual ~GlProxyImplementation();

  virtual void PreRender();

  virtual void PostRender( unsigned int timeDelta );

  virtual void DrawArrays (GLenum mode, GLint first, GLsizei count);

  virtual void DrawElements (GLenum mode, GLsizei count, GLenum type, const void* indices);

private:
  EnvironmentOptions& mEnvironmentOptions;
  Sampler mDrawSampler;
  int mDrawCount;
  int mFrameCount;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_GL_PROXY_IMPLEMENTATION_H__
