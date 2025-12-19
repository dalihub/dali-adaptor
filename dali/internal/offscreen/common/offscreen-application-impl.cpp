/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/internal/adaptor/common/adaptor-builder-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/framework-factory.h>
#include <dali/internal/adaptor/common/thread-controller-interface.h>
#include <dali/internal/offscreen/common/offscreen-window-impl.h>
#include <dali/internal/system/common/environment-variables.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

IntrusivePtr<OffscreenApplication> OffscreenApplication::New(Dali::OffscreenApplication::FrameworkBackend framework, Dali::OffscreenApplication::RenderMode renderMode)
{
  IntrusivePtr<OffscreenApplication> offscreenApplication = new OffscreenApplication(framework, renderMode);
  return offscreenApplication;
}

OffscreenApplication::OffscreenApplication(Dali::OffscreenApplication::FrameworkBackend framework, Dali::OffscreenApplication::RenderMode renderMode)
: mDefaultWindow(),
  mFrameworkBackend(framework),
  mRenderMode(renderMode)
{
  // Disable partial update
  EnvironmentVariable::SetEnvironmentVariable(DALI_ENV_DISABLE_PARTIAL_UPDATE, "1");

  // Disable ATSPI
  Dali::Accessibility::Bridge::DisableAutoInit();

  mEnvironmentOptions = std::unique_ptr<EnvironmentOptions>(new EnvironmentOptions());

  mFrameworkFactory = std::unique_ptr<FrameworkFactory>(Dali::Internal::Adaptor::CreateFrameworkFactory());

  // We don't need Framework, just set backend type
  mFrameworkFactory->SetFrameworkBackend(framework == Dali::OffscreenApplication::FrameworkBackend::ECORE ? FrameworkBackend::DEFAULT : FrameworkBackend::GLIB);

  CreateWindow();
  CreateAdaptor();
}

OffscreenApplication::~OffscreenApplication()
{
  SingletonService service = SingletonService::Get();
  if(service)
  {
    service.UnregisterAll();
  }

  if(mDefaultWindow)
  {
    mDefaultWindow.Reset();
  }
}

void OffscreenApplication::Start()
{
  if(!mIsAdaptorStarted)
  {
    mAdaptor->Start();
    mIsAdaptorStarted = true;

    mAdaptor->NotifySceneCreated();
  }
}

void OffscreenApplication::Terminate()
{
  if(!mIsAdaptorStoped)
  {
    mAdaptor->Stop();
    mIsAdaptorStoped = true;
  }
}

Dali::OffscreenWindow OffscreenApplication::GetWindow()
{
  return mDefaultWindow;
}

void OffscreenApplication::RenderOnce()
{
  mAdaptor->RenderOnce();
}

void OffscreenApplication::CreateWindow()
{
  IntrusivePtr<OffscreenWindow> window = OffscreenWindow::New();

  mDefaultWindow = Dali::OffscreenWindow(window.Get());
}

void OffscreenApplication::CreateAdaptor()
{
  auto& adaptorBuilder  = AdaptorBuilder::Get(*mEnvironmentOptions);
  auto& graphicsFactory = adaptorBuilder.GetGraphicsFactory();

  OffscreenWindow& window     = GetImplementation(mDefaultWindow);
  ThreadMode       threadMode = mRenderMode == Dali::OffscreenApplication::RenderMode::AUTO ? ThreadMode::NORMAL : ThreadMode::RUN_IF_REQUESTED;

  mAdaptor = std::unique_ptr<Dali::Adaptor>(Adaptor::New(graphicsFactory, Integration::SceneHolder(&window), window.GetSurface(), mEnvironmentOptions.get(), threadMode));

  // adaptorBuilder invalidate after now.
  AdaptorBuilder::Finalize();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
