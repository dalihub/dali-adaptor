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
#include "render-helper.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/graphics/graphics.h>

// INTERNAL INCLUDES
#include <base/interfaces/adaptor-internal-services.h>
#include <base/display-connection.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

RenderHelper::RenderHelper( AdaptorInternalServices& adaptorInterfaces )
: mGraphics( adaptorInterfaces.GetGraphics() ),
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
}

void RenderHelper::Start()
{
}

void RenderHelper::Stop()
{
}

void RenderHelper::ConsumeEvents()
{
  mDisplayConnection->ConsumeEvents();
}

void RenderHelper::Initialize()
{
}

void RenderHelper::ReplaceSurface( RenderSurface* newSurface )
{
}

void RenderHelper::ResizeSurface()
{
  mSurfaceResized = true;
}

void RenderHelper::Shutdown()
{
}

bool RenderHelper::PreRender()
{
  mGraphics.PreRender( 1 );
  return true;
}

void RenderHelper::PostRender( bool renderToFbo )
{
  mGraphics.PostRender( 1 );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
