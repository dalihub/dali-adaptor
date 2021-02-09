/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/internal/adaptor/common/lifecycle-controller-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
Dali::LifecycleController LifecycleController::Get()
{
  Dali::LifecycleController lifecycleController;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::LifecycleController));
    if(handle)
    {
      // If so, downcast the handle
      lifecycleController = Dali::LifecycleController(dynamic_cast<LifecycleController*>(handle.GetObjectPtr()));
    }
    else
    {
      lifecycleController = Dali::LifecycleController(new LifecycleController());
      service.Register(typeid(lifecycleController), lifecycleController);
    }
  }

  return lifecycleController;
}

LifecycleController::LifecycleController()
{
}

LifecycleController::~LifecycleController()
{
}

Dali::LifecycleController::LifecycleSignalType& LifecycleController::InitSignal()
{
  return mInitSignal;
}

void LifecycleController::EmitInitSignal()
{
  if(!mInitSignal.Empty())
  {
    mInitSignal.Emit();
  }
}

Dali::LifecycleController::LifecycleSignalType& LifecycleController::TerminateSignal()
{
  return mTerminateSignal;
}

void LifecycleController::EmitTerminateSignal()
{
  if(!mTerminateSignal.Empty())
  {
    mTerminateSignal.Emit();
  }
}

Dali::LifecycleController::LifecycleSignalType& LifecycleController::PauseSignal()
{
  return mPauseSignal;
}

void LifecycleController::EmitPauseSignal()
{
  if(!mPauseSignal.Empty())
  {
    mPauseSignal.Emit();
  }
}

Dali::LifecycleController::LifecycleSignalType& LifecycleController::ResumeSignal()
{
  return mResumeSignal;
}

void LifecycleController::EmitResumeSignal()
{
  if(!mResumeSignal.Empty())
  {
    mResumeSignal.Emit();
  }
}

Dali::LifecycleController::LifecycleSignalType& LifecycleController::ResetSignal()
{
  return mResetSignal;
}

void LifecycleController::EmitResetSignal()
{
  if(!mResetSignal.Empty())
  {
    mResetSignal.Emit();
  }
}

Dali::LifecycleController::LifecycleSignalType& LifecycleController::LanguageChangedSignal()
{
  return mLanguageChangedSignal;
}

void LifecycleController::EmitLanguageChangedSignal()
{
  if(!mLanguageChangedSignal.Empty())
  {
    mLanguageChangedSignal.Emit();
  }
}

void LifecycleController::OnInit(Dali::Application& app)
{
  EmitInitSignal();
}

void LifecycleController::OnTerminate(Dali::Application& app)
{
  EmitTerminateSignal();
}

void LifecycleController::OnPause(Dali::Application& app)
{
  EmitPauseSignal();
}

void LifecycleController::OnResume(Dali::Application& app)
{
  EmitResumeSignal();
}

void LifecycleController::OnReset(Dali::Application& app)
{
  EmitResetSignal();
}

void LifecycleController::OnLanguageChanged(Dali::Application& app)
{
  EmitLanguageChangedSignal();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
