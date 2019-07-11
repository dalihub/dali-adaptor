/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/gles/egl-context-helper-implementation.h>

// EXTERNAL INCLUDES

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-implementation.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

EglContextHelperImplementation::EglContextHelperImplementation()
: mEglImplementation( NULL )
{
}

void EglContextHelperImplementation::Initialize( EglImplementation* eglImpl )
{
  mEglImplementation = eglImpl;
}

void EglContextHelperImplementation::MakeSurfacelessContextCurrent()
{
  if ( mEglImplementation && mEglImplementation->IsSurfacelessContextSupported() )
  {
    mEglImplementation->MakeContextCurrent( EGL_NO_SURFACE, mEglImplementation->GetContext() );
  }
}

void EglContextHelperImplementation::MakeContextCurrent( Integration::RenderSurface* surface )
{
  if ( mEglImplementation && surface )
  {
    surface->MakeContextCurrent();
  }
}

void EglContextHelperImplementation::MakeContextNull()
{
  if ( mEglImplementation )
  {
    mEglImplementation->MakeContextNull();
  }
}

void EglContextHelperImplementation::WaitClient()
{
  if ( mEglImplementation )
  {
    mEglImplementation->WaitClient();
  }
}

} // namespace Dali
} // namespace Internal
} // namespace Adaptor
