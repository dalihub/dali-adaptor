/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/offscreen-application.h>

// INTERNAL INCLUDES
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

namespace Dali
{
OffscreenApplication OffscreenApplication::New(uint16_t width, uint16_t height, bool isTranslucent, OffscreenApplication::RenderMode renderMode)
{
  Dali::Any                                    surface;
  IntrusivePtr<Internal::OffscreenApplication> impl = Internal::OffscreenApplication::New(width, height, surface, isTranslucent, renderMode);

  OffscreenApplication offscreenApplication = OffscreenApplication(impl.Get());

  return offscreenApplication;
}

OffscreenApplication OffscreenApplication::New(Dali::Any surface, OffscreenApplication::RenderMode renderMode)
{
  IntrusivePtr<Internal::OffscreenApplication> impl = Internal::OffscreenApplication::New(0, 0, surface, false, renderMode);

  OffscreenApplication offscreenApplication = OffscreenApplication(impl.Get());

  return offscreenApplication;
}

OffscreenApplication::OffscreenApplication() = default;

OffscreenApplication::OffscreenApplication(const OffscreenApplication& offscreenApplication) = default;

OffscreenApplication& OffscreenApplication::operator=(const OffscreenApplication& offscreenApplication) = default;

OffscreenApplication::~OffscreenApplication() = default;

void OffscreenApplication::MainLoop()
{
  Internal::GetImplementation(*this).MainLoop();
}

void OffscreenApplication::Quit()
{
  Internal::GetImplementation(*this).Quit();
}

Dali::OffscreenWindow OffscreenApplication::GetWindow()
{
  return Internal::GetImplementation(*this).GetWindow();
}

void OffscreenApplication::RenderOnce()
{
  Internal::GetImplementation(*this).RenderOnce();
}

Any OffscreenApplication::GetFrameworkContext() const
{
  return Internal::GetImplementation(*this).GetFrameworkContext();
}

OffscreenApplication::OffscreenApplicationSignalType& OffscreenApplication::InitSignal()
{
  return Internal::GetImplementation(*this).InitSignal();
}

OffscreenApplication::OffscreenApplicationSignalType& OffscreenApplication::TerminateSignal()
{
  return Internal::GetImplementation(*this).TerminateSignal();
}

OffscreenApplication::OffscreenApplication(Internal::OffscreenApplication* offscreenApplication)
: BaseHandle(offscreenApplication)
{
}

} // namespace Dali
