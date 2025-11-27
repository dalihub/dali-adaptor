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
#include <dali/devel-api/adaptor-framework/offscreen-window.h>

// EXTENRAL INCLUDES
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/offscreen/common/offscreen-window-impl.h>

namespace Dali
{
OffscreenWindow OffscreenWindow::New()
{
  OffscreenWindow newWindow;

  const bool isAdaptorAvailable = Dali::Adaptor::IsAvailable();
  bool       isNewWindowAllowed = false;

  if(isAdaptorAvailable)
  {
    Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
    isNewWindowAllowed     = Internal::Adaptor::Adaptor::GetImplementation(adaptor).IsMultipleWindowSupported();
  }

  if(isNewWindowAllowed)
  {
    IntrusivePtr<Internal::Adaptor::OffscreenWindow> window = Internal::Adaptor::OffscreenWindow::New();

    Integration::SceneHolder sceneHolder = Integration::SceneHolder(window.Get());

    if(isAdaptorAvailable)
    {
      Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
      Internal::Adaptor::Adaptor::GetImplementation(adaptor).AddWindow(sceneHolder);
    }
    newWindow = OffscreenWindow(window.Get());
  }
  else
  {
    DALI_LOG_ERROR("This device can't support multiple windows.\n");
  }

  return newWindow;
}

OffscreenWindow::OffscreenWindow() = default;

OffscreenWindow::OffscreenWindow(const OffscreenWindow& window) = default;

OffscreenWindow& OffscreenWindow::operator=(const OffscreenWindow& window) = default;

OffscreenWindow::OffscreenWindow(OffscreenWindow&& window) noexcept = default;

OffscreenWindow& OffscreenWindow::operator=(OffscreenWindow&& window) noexcept = default;

OffscreenWindow::~OffscreenWindow() = default;

void OffscreenWindow::SetNativeImage(NativeImageSourcePtr nativeImage)
{
  Internal::Adaptor::GetImplementation(*this).SetNativeImage(nativeImage);
}

void OffscreenWindow::Add(Actor actor)
{
  Internal::Adaptor::GetImplementation(*this).Add(actor);
}

void OffscreenWindow::Remove(Actor actor)
{
  Internal::Adaptor::GetImplementation(*this).Remove(actor);
}

void OffscreenWindow::SetBackgroundColor(const Vector4& color)
{
  Internal::Adaptor::GetImplementation(*this).SetBackgroundColor(color);
}

Vector4 OffscreenWindow::GetBackgroundColor() const
{
  return Internal::Adaptor::GetImplementation(*this).GetBackgroundColor();
}

Layer OffscreenWindow::GetRootLayer() const
{
  return Internal::Adaptor::GetImplementation(*this).GetRootLayer();
}

OffscreenWindow::WindowSize OffscreenWindow::GetSize() const
{
  return Internal::Adaptor::GetImplementation(*this).GetSize();
}

void OffscreenWindow::AddPostRenderSyncCallback(std::unique_ptr<CallbackBase> callback)
{
  Internal::Adaptor::GetImplementation(*this).AddPostRenderSyncCallback(std::move(callback));
}

void OffscreenWindow::AddPostRenderAsyncCallback(std::unique_ptr<CallbackBase> callback)
{
  Internal::Adaptor::GetImplementation(*this).AddPostRenderAsyncCallback(std::move(callback));
}

OffscreenWindow::OffscreenWindow(Internal::Adaptor::OffscreenWindow* window)
: BaseHandle(window)
{
}

} // namespace Dali
