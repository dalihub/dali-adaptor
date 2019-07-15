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
#include <dali/internal/system/common/thread-controller.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/adaptor/common/thread-controller-interface.h>
#include <dali/internal/adaptor/common/combined-update-render-controller.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

ThreadController::ThreadController( AdaptorInternalServices& adaptorInterfaces, const EnvironmentOptions& environmentOptions )
: mThreadControllerInterface( NULL )
{
  switch( environmentOptions.GetThreadingMode() )
  {
    case ThreadingMode::COMBINED_UPDATE_RENDER:
    {
      mThreadControllerInterface = new CombinedUpdateRenderController( adaptorInterfaces, environmentOptions );
      break;
    }
  }
}

ThreadController::~ThreadController()
{
  delete mThreadControllerInterface;
}

void ThreadController::Initialize()
{
  mThreadControllerInterface->Initialize();
}

void ThreadController::Start()
{
  mThreadControllerInterface->Start();
}

void ThreadController::Pause()
{
  mThreadControllerInterface->Pause();
}

void ThreadController::Resume()
{
  mThreadControllerInterface->Resume();
}

void ThreadController::Stop()
{
  mThreadControllerInterface->Stop();
}

void ThreadController::RequestUpdate()
{
  mThreadControllerInterface->RequestUpdate();
}

void ThreadController::RequestUpdateOnce()
{
  mThreadControllerInterface->RequestUpdateOnce();
}

void ThreadController::ReplaceSurface( Dali::RenderSurfaceInterface* newSurface )
{
  mThreadControllerInterface->ReplaceSurface( newSurface );
}

void ThreadController::DeleteSurface( Dali::RenderSurfaceInterface* surface )
{
  mThreadControllerInterface->DeleteSurface( surface );
}

void ThreadController::ResizeSurface()
{
  mThreadControllerInterface->ResizeSurface();
}

void ThreadController::SetRenderRefreshRate(unsigned int numberOfVSyncsPerRender )
{
  mThreadControllerInterface->SetRenderRefreshRate( numberOfVSyncsPerRender );
}

void ThreadController::SetPreRenderCallback( CallbackBase* callback )
{
  mThreadControllerInterface->SetPreRenderCallback( callback );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
