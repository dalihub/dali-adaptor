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
#include <dali/public-api/adaptor-framework/ui-context.h>

// EXTERNAL INCLUDES

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/ui-context-impl.h>

namespace Dali
{

UiContext UiContext::Get()
{
  return Internal::Adaptor::UiContext::Get();
}

UiContext::UiContext(Internal::Adaptor::UiContext* uiContext)
: BaseHandle(uiContext)
{
}

UiContext::UiContext() = default;

UiContext::~UiContext() = default;

UiContext::UiContext(const UiContext& copy) = default;

UiContext& UiContext::operator=(const UiContext& rhs) = default;

UiContext::UiContext(UiContext&& rhs) noexcept = default;

UiContext& UiContext::operator=(UiContext&& rhs) noexcept = default;

Window UiContext::GetDefaultWindow()
{
  return Internal::Adaptor::GetImplementation(*this).GetDefaultWindow();
}

bool UiContext::AddIdle(CallbackBase* callback)
{
  return Internal::Adaptor::GetImplementation(*this).AddIdle(callback);
}

int32_t UiContext::GetRenderThreadId()
{
  return Internal::Adaptor::GetImplementation(*this).GetRenderThreadId();
}

void UiContext::FlushUpdateMessages()
{
  Internal::Adaptor::GetImplementation(*this).FlushUpdateMessages();
}

void UiContext::SetApplicationLocale(const std::string& locale)
{
  Internal::Adaptor::GetImplementation(*this).SetApplicationLocale(locale);
}

} // namespace Dali
