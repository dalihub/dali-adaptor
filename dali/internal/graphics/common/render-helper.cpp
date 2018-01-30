/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/common/render-helper.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/window-system/common/display-connection.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

RenderHelper::RenderHelper( AdaptorInternalServices& adaptorInterfaces )
: mGLES( adaptorInterfaces.GetGlesInterface() ),
  mEglFactory( &adaptorInterfaces.GetEGLFactoryInterface()),
  mEGL( NULL ),
  mSurfaceReplaced( false ),
  mSurfaceResized( false )
{
  // set the initial values before render thread starts
  mSurface = adaptorInterfaces.GetRenderSurfaceInterface();

  if( mSurface )
  {
    mDisplayConnection = Dali::DisplayConnection::New( mSurface->GetSurfaceType() );
  }
  else
  {
    mDisplayConnection = Dali::DisplayConnection::New();
  }
}

RenderHelper::~RenderHelper()
{
  if (mDisplayConnection)
  {
    delete mDisplayConnection;
    mDisplayConnection = NULL;
  }

  mEglFactory->Destroy();
}

void RenderHelper::Start()
{
  if( mSurface )
  {
    mSurface->StartRender();
  }
}

void RenderHelper::Stop()
{
  if( mSurface )
  {
    // Tell surface we have stopped rendering
    mSurface->StopRender();
  }
}

void RenderHelper::ConsumeEvents()
{
  mDisplayConnection->ConsumeEvents();
}

void RenderHelper::InitializeEgl()
{
  mEGL = mEglFactory->Create();

  DALI_ASSERT_ALWAYS( mSurface && "NULL surface" );

  // Initialize EGL & OpenGL
  mDisplayConnection->InitializeEgl( *mEGL );
  mSurface->InitializeEgl( *mEGL );

  // create the OpenGL context
  mEGL->CreateContext();

  // create the OpenGL surface
  mSurface->CreateEglSurface(*mEGL);

  // Make it current
  mEGL->MakeContextCurrent();
}

void RenderHelper::ReplaceSurface( RenderSurface* newSurface )
{
  mSurface->DestroyEglSurface(*mEGL);

  // This is designed for replacing pixmap surfaces, but should work for window as well
  // we need to delete the egl surface and renderable (pixmap / window)
  // Then create a new pixmap/window and new egl surface
  // If the new surface has a different display connection, then the context will be lost
  DALI_ASSERT_ALWAYS(newSurface && "NULL surface");

  mDisplayConnection->InitializeEgl(*mEGL);

  newSurface->ReplaceEGLSurface(*mEGL);

  // use the new surface from now on
  mSurface = newSurface;
  mSurfaceReplaced = true;
}

void RenderHelper::ResizeSurface()
{
  mSurfaceResized = true;
}

void RenderHelper::ShutdownEgl()
{
  if( mSurface )
  {
    // give a chance to destroy the OpenGL surface that created externally
    mSurface->DestroyEglSurface( *mEGL );

    mSurface = NULL;
  }

  // delete the GL context / egl surface
  mEGL->TerminateGles();
}

bool RenderHelper::PreRender()
{
  if( mSurface )
  {
    mSurface->PreRender( *mEGL, mGLES, mSurfaceResized );
  }
  mGLES.PreRender();
  return true;
}

void RenderHelper::PostRender( bool renderToFbo )
{
  // Inform the gl implementation that rendering has finished before informing the surface
  mGLES.PostRender();

  if( renderToFbo )
  {
    mGLES.Flush();
    mGLES.Finish();
  }
  else
  {
    if( mSurface )
    {
      // Inform the surface that rendering this frame has finished.
      mSurface->PostRender( *mEGL, mGLES, mDisplayConnection, mSurfaceReplaced, mSurfaceResized );
    }
  }
  mSurfaceReplaced = false;
  mSurfaceResized = false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
