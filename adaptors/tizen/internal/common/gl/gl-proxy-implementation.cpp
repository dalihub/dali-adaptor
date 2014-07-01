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

// CLASS HEADER
#include "gl-proxy-implementation.h"

// EXTERNAL INCLUDES
#include <math.h>

// INTERNAL INCLUDES
#include <base/environment-options.h>
#include <dali/integration-api/debug.h>

namespace
{
const int NUM_FRAMES_PER_SECOND(60);
}


namespace Dali
{
namespace Internal
{
namespace Adaptor
{

Sampler::Sampler()
: mAccumulated(0.0f),
  mAccumulatedSquare(0.0f),
  mMin(0.0f),
  mMax(0.0f),
  mNumSamples(0)
{
}


void Sampler::Reset()
{
  mAccumulated = 0.0f;
  mAccumulatedSquare= 0.0f;
  mMin = 0.0f;
  mMax = 0.0f;
  mNumSamples = 0;
}

void Sampler::Accumulate(float value)
{
  if( mNumSamples == 0 )
  {
    mMin = value;
    mMax = value;
  }
  else
  {
    if(value < mMin)
    {
      mMin = value;
    }
    if(value > mMax)
    {
      mMax = value;
    }
  }

  mNumSamples++;

  mAccumulated += value;
  mAccumulatedSquare += (value * value);
}

float Sampler::GetMeanValue()
{
  float meanValue = 0;
  if( mNumSamples > 0 )
  {
    meanValue = mAccumulated / (float)mNumSamples;
  }
  return meanValue;
}

float Sampler::GetStandardDeviation()
{
  float standardDeviation=0.0f;
  if( mNumSamples > 0 )
  {
    standardDeviation = sqrtf( mNumSamples * mAccumulatedSquare - (mAccumulated*mAccumulated)) / mNumSamples;
  }
  return standardDeviation;
}

float Sampler::GetMin()
{
  return mMin;
}

float Sampler::GetMax()
{
  return mMax;
}

GlProxyImplementation::GlProxyImplementation(EnvironmentOptions& environmentOptions)
: mEnvironmentOptions(environmentOptions),
  mDrawCount(0),
  mFrameCount(0)
{
}

GlProxyImplementation::~GlProxyImplementation()
{
}

void GlProxyImplementation::PreRender()
{
}

void GlProxyImplementation::PostRender( unsigned int timeDelta )
{
  // Accumulate counts in each sampler
  mDrawSampler.Accumulate(mDrawCount);
  mDrawCount=0;

  // When we reach the desired frame count, output the averages from the samples
  mFrameCount++;
  if( mFrameCount >= mEnvironmentOptions.GetGlesCallTime() * NUM_FRAMES_PER_SECOND )
  {
    Debug::LogMessage( Debug::DebugInfo, "Mean number of draw calls per frame: %5.2f  (Min:%5.2f, Max:%5.2f, StdDev:%5.2f sampled over %d frames\n",
                       mDrawSampler.GetMeanValue(),
                       mDrawSampler.GetMin(),
                       mDrawSampler.GetMax(),
                       mDrawSampler.GetStandardDeviation(),
                       mFrameCount );

    mDrawSampler.Reset();
    mFrameCount = 0;
  }
}

void GlProxyImplementation::DrawArrays (GLenum mode, GLint first, GLsizei count)
{
  mDrawCount++;
  GlImplementation::DrawArrays(mode,first,count);
}

void GlProxyImplementation::DrawElements (GLenum mode, GLsizei count, GLenum type, const void* indices)
{
  mDrawCount++;
  GlImplementation::DrawElements(mode,count,type,indices);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
