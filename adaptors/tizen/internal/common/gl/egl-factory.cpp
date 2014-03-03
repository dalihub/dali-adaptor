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
#include "egl-factory.h"

// INTERNAL INCLUDES
#include <internal/common/gl/egl-implementation.h>
#include <internal/common/gl/egl-image-extensions.h>
#include <internal/common/gl/egl-sync-implementation.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

EglFactory::EglFactory()
: mEglImplementation(NULL),
  mEglImageExtensions(NULL),
  mEglSync(new EglSyncImplementation) // Created early, as needed by Core constructor
{
}

EglFactory::~EglFactory()
{
  // Ensure the EGL implementation is destroyed
  delete mEglImageExtensions;
  delete mEglImplementation;
  delete mEglSync;
}

EglInterface* EglFactory::Create()
{
  // Created by RenderThread (After Core construction)
  mEglImplementation = new EglImplementation();
  mEglImageExtensions = new EglImageExtensions(mEglImplementation);

  mEglSync->Initialize(mEglImplementation); // The sync impl needs the EglDisplay
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

EglSyncImplementation* EglFactory::GetSyncImplementation()
{
  return mEglSync;
}

} // Adaptor
} // Internal
} // Dali
