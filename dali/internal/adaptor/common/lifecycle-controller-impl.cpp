/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/common/singleton-service.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
Dali::LifecycleController LifecycleController::Get()
{
  // LifecycleController only holds signals and does not depend on Core, whilst the lifecycle it
  // describes is process-wide. It is therefore kept alive for the lifetime of the process rather
  // than being owned by SingletonService, so that observers can connect to the signals before Core
  // has been created (e.g. from main() before the application main loop starts).
  static Dali::LifecycleController lifecycleController(new LifecycleController());

  // Register with SingletonService, when available, so the instance can still be discovered through it.
  Dali::SingletonService service(SingletonService::Get());
  if(service && !service.GetSingleton(typeid(Dali::LifecycleController)))
  {
    service.Register(typeid(Dali::LifecycleController), lifecycleController);
  }

  return lifecycleController;
}

LifecycleController::LifecycleController()
{
}

LifecycleController::~LifecycleController()
{
}
Dali::LifecycleController::LifecycleSignalType& LifecycleController::PreInitSignal()
{
  return mPreInitSignal;
}

void LifecycleController::EmitPreInitSignal()
{
  if(!mPreInitSignal.Empty())
  {
    mPreInitSignal.Emit();
  }
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

void LifecycleController::OnPreInit()
{
  EmitPreInitSignal();
}

void LifecycleController::OnInit()
{
  EmitInitSignal();
}

void LifecycleController::OnTerminate()
{
  EmitTerminateSignal();
}

void LifecycleController::OnPause()
{
  EmitPauseSignal();
}

void LifecycleController::OnResume()
{
  EmitResumeSignal();
}

void LifecycleController::OnReset()
{
  EmitResetSignal();
}

void LifecycleController::OnLanguageChanged()
{
  EmitLanguageChangedSignal();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
