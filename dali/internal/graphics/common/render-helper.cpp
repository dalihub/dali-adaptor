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
#include <dali/internal/graphics/common/render-helper.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/graphics/graphics.h>

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
: mEglFactory( &adaptorInterfaces.GetEGLFactoryInterface()),
  mEGL( NULL ),
  mGraphics( adaptorInterfaces.GetGraphics() ),
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
#if 0
  mEglFactory->Destroy();
#endif
}

void RenderHelper::Start()
{
#if 0
  if( mSurface )
  {
    mSurface->StartRender();
  }
#endif
}

void RenderHelper::Stop()
{
#if 0
  if( mSurface )
  {
    // Tell surface we have stopped rendering
    mSurface->StopRender();
  }
#endif
}

void RenderHelper::ConsumeEvents()
{
  mDisplayConnection->ConsumeEvents();
}

void RenderHelper::InitializeEgl()
{
#if 0
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
#endif
}

void RenderHelper::ReplaceSurface( RenderSurface* newSurface )
{
#if 0
  if( mSurface )
  {
    mSurface->DestroyEglSurface(*mEGL);
  }

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
#endif
}

void RenderHelper::ResizeSurface()
{
  mSurfaceResized = true;
}

void RenderHelper::ShutdownEgl()
{
#if 0
  if( mSurface )
  {
    // give a chance to destroy the OpenGL surface that created externally
    mSurface->DestroyEglSurface( *mEGL );

    mSurface = NULL;
  }

  // delete the GL context / egl surface
  mEGL->TerminateGles();
#endif
}

bool RenderHelper::PreRender()
{
#if 0
  if( mSurface )
  {
    mSurface->PreRender( *mEGL, mSurfaceResized );
  }
#endif
  return true;
}

void RenderHelper::PostRender( bool renderToFbo )
{
#if 0
  if( renderToFbo )
  {
    //mGLES.Flush();
    //mGLES.Finish();
  }
  else
  {
    if( mSurface )
    {
      // Inform the surface that rendering this frame has finished.
      mSurface->PostRender( *mEGL, mDisplayConnection, mSurfaceReplaced, mSurfaceResized );
    }
  }
  mSurfaceReplaced = false;
  mSurfaceResized = false;
#endif
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
