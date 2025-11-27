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
#include <dali/devel-api/adaptor-framework/offscreen-application.h>

// INTERNAL INCLUDES
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

namespace Dali
{
OffscreenApplication OffscreenApplication::New(int* argc, char** argv[], FrameworkBackend framework, RenderMode renderMode)
{
  // Now we don't use command line args
  IntrusivePtr<Internal::Adaptor::OffscreenApplication> impl = Internal::Adaptor::OffscreenApplication::New(framework, renderMode);

  OffscreenApplication offscreenApplication = OffscreenApplication(impl.Get());

  return offscreenApplication;
}

OffscreenApplication::OffscreenApplication() = default;

OffscreenApplication::OffscreenApplication(const OffscreenApplication& offscreenApplication) = default;

OffscreenApplication& OffscreenApplication::operator=(const OffscreenApplication& offscreenApplication) = default;

OffscreenApplication::OffscreenApplication(OffscreenApplication&& rhs) noexcept = default;

OffscreenApplication& OffscreenApplication::operator=(OffscreenApplication&& rhs) noexcept = default;

OffscreenApplication::~OffscreenApplication() = default;

void OffscreenApplication::Start()
{
  Internal::Adaptor::GetImplementation(*this).Start();
}

void OffscreenApplication::Terminate()
{
  Internal::Adaptor::GetImplementation(*this).Terminate();
}

OffscreenWindow OffscreenApplication::GetWindow()
{
  return Internal::Adaptor::GetImplementation(*this).GetWindow();
}

void OffscreenApplication::RenderOnce()
{
  Internal::Adaptor::GetImplementation(*this).RenderOnce();
}

OffscreenApplication::OffscreenApplication(Internal::Adaptor::OffscreenApplication* offscreenApplication)
: BaseHandle(offscreenApplication)
{
}

} // namespace Dali
