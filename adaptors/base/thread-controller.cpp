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
#include "thread-controller.h"

// INTERNAL INCLUDES
#include <base/environment-options.h>
#include <base/thread-controller-interface.h>
#include <base/combined-update-render/combined-update-render-controller.h>
#include <base/separate-update-render/separate-update-render-controller.h>
#include <base/single-threaded/single-thread-controller.h>

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
    case ThreadingMode::SEPARATE_UPDATE_RENDER:
    {
      mThreadControllerInterface = new SeparateUpdateRenderController( adaptorInterfaces, environmentOptions );
      break;
    }

    case ThreadingMode::COMBINED_UPDATE_RENDER:
    {
      mThreadControllerInterface = new CombinedUpdateRenderController( adaptorInterfaces, environmentOptions );
      break;
    }

    case ThreadingMode::SINGLE_THREADED:
    {
      mThreadControllerInterface = new SingleThreadController( adaptorInterfaces, environmentOptions );
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

void ThreadController::ReplaceSurface( RenderSurface* newSurface )
{
  mThreadControllerInterface->ReplaceSurface( newSurface );
}

void ThreadController::ResizeSurface()
{
  mThreadControllerInterface->ResizeSurface();
}

void ThreadController::SetRenderRefreshRate(unsigned int numberOfVSyncsPerRender )
{
  mThreadControllerInterface->SetRenderRefreshRate( numberOfVSyncsPerRender );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
