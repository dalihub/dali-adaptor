/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include "egl-factory.h"

// INTERNAL INCLUDES
#include <gl/egl-implementation.h>
#include <gl/egl-image-extensions.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

EglFactory::EglFactory( int multiSamplingLevel )
: mEglImplementation(NULL),
  mEglImageExtensions(NULL),
  mMultiSamplingLevel( multiSamplingLevel )
{
}

EglFactory::~EglFactory()
{
  // Ensure the EGL implementation is destroyed
  delete mEglImageExtensions;
  delete mEglImplementation;
}

EglInterface* EglFactory::Create()
{
  // Created by RenderThread (After Core construction)
  mEglImplementation = new EglImplementation( mMultiSamplingLevel );
  mEglImageExtensions = new EglImageExtensions( mEglImplementation );
  return mEglImplementation;
}

void EglFactory::Destroy()
{
  delete mEglImageExtensions;
  mEglImageExtensions = NULL;
  delete mEglImplementation;
  mEglImplementation = NULL;
}

EglInterface* EglFactory::GetImplementation()
{
  return mEglImplementation;
}

EglImageExtensions* EglFactory::GetImageExtensions()
{
  return mEglImageExtensions;
}

} // Adaptor
} // Internal
} // Dali
