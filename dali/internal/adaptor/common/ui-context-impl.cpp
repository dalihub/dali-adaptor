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
#include <dali/internal/adaptor/common/ui-context-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

Dali::UiContext UiContext::Get()
{
  Dali::UiContext  uiContext;
  SingletonService singletonService(SingletonService::Get());
  if(singletonService)
  {
    // Check whether the async task manager is already created
    Dali::BaseHandle handle = singletonService.GetSingleton(typeid(Dali::UiContext));
    if(handle)
    {
      // If so, downcast the handle of singleton
      uiContext = Dali::UiContext(dynamic_cast<Internal::Adaptor::UiContext*>(handle.GetObjectPtr()));
    }
  }
  return uiContext;
}

Dali::UiContext UiContext::New(Dali::Adaptor* adaptor)
{
  Dali::UiContext uiContext = UiContext::Get();
  if(!uiContext)
  {
    SingletonService singletonService(SingletonService::Get());
    if(singletonService)
    {
      // If not, create the UiContext and register it as a singleton
      Internal::Adaptor::UiContext* internalUiContext = new Internal::Adaptor::UiContext(adaptor);
      uiContext                                       = Dali::UiContext(internalUiContext);
      singletonService.Register(typeid(uiContext), uiContext);
    }
  }

  return uiContext;
}

UiContext::~UiContext()
{
}

UiContext::UiContext(Dali::Adaptor* adaptor)
: mAdaptor(adaptor)
{
}

void UiContext::SetDefaultWindow(Dali::Window window)
{
  mDefaultWindow = window;
}

Dali::Window UiContext::GetDefaultWindow()
{
  return mDefaultWindow;
}

bool UiContext::AddIdle(CallbackBase* callback)
{
  return mAdaptor->AddIdle(callback, true);
}

int32_t UiContext::GetRenderThreadId()
{
  return Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).GetRenderThreadId();
}

void UiContext::FlushUpdateMessages()
{
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).FlushUpdateMessages();
}

void UiContext::SetApplicationLocale(const std::string& locale)
{
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).SetApplicationLocale(locale);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
