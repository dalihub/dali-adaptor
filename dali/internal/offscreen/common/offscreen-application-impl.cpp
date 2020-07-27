/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/integration-api/adaptor-framework/native-render-surface.h>

// INTERNAL INCLUDES
#include <dali/internal/offscreen/common/offscreen-window-impl.h>
#include <dali/internal/offscreen/common/offscreen-window-factory.h>
#include <dali/internal/adaptor/common/thread-controller-interface.h>

// CLASS HEADER
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

using Dali::Internal::Adaptor::ThreadMode;

namespace Dali
{

namespace Internal
{

IntrusivePtr< OffscreenApplication > OffscreenApplication::New( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent, RenderMode renderMode )
{
  IntrusivePtr< OffscreenApplication > offscreenApplication = new OffscreenApplication( width, height, surface, isTranslucent, renderMode );
  return offscreenApplication;
}

OffscreenApplication::OffscreenApplication( uint16_t width, uint16_t height, Dali::Any surface, bool isTranslucent, RenderMode renderMode )
{
  // Generate a default window
  auto windowFactory = Dali::Internal::GetOffscreenWindowFactory();
  IntrusivePtr <Internal::OffscreenWindow > windowPtr = windowFactory->CreateOffscreenWindow( width, height, surface, isTranslucent ).release();
  mDefaultWindow = Dali::OffscreenWindow( windowPtr.Get() );

  mAdaptor = Dali::Internal::Adaptor::Adaptor::New( Dali::Integration::SceneHolder( windowPtr.Get() ), windowPtr->GetSurface(),
                            Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS, NULL,
                            renderMode == RenderMode::AUTO ? ThreadMode::NORMAL : ThreadMode::RUN_IF_REQUESTED );

  // Initialize default window
  windowPtr->Initialize( true );
}

OffscreenApplication::~OffscreenApplication()
{
  delete mAdaptor;
  mAdaptor = nullptr;
}

void OffscreenApplication::Start()
{
   // Start the adaptor
   mAdaptor->Start();
   mInitSignal.Emit();
   mAdaptor->NotifySceneCreated();
}

void OffscreenApplication::Stop()
{
  // Stop the adaptor
  mAdaptor->Stop();
  mTerminateSignal.Emit();
}

Dali::OffscreenWindow OffscreenApplication::GetWindow()
{
  return mDefaultWindow;
}

void OffscreenApplication::RenderOnce()
{
  mAdaptor->RenderOnce();
}

} // namespace Internal

} // namespace Dali
