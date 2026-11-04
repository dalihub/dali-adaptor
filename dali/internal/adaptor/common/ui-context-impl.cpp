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
#include <dali/integration-api/core-enumerations.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace DALI_NAMESPACE
{

namespace Internal
{

namespace Adaptor
{

namespace
{
static constexpr Dali::TypeInfoId gSingletonTypeInfoId = DALI_TYPE_ID(Dali::UiContext);

// The public enumeration mirrors the integration one, so a simple cast is enough.
static_assert(static_cast<int>(Dali::UiContext::RenderingBehavior::IF_REQUIRED) == static_cast<int>(Dali::Integration::RenderingBehavior::IF_REQUIRED));
static_assert(static_cast<int>(Dali::UiContext::RenderingBehavior::CONTINUOUSLY) == static_cast<int>(Dali::Integration::RenderingBehavior::CONTINUOUSLY));

/**
 * @brief Executes an idle callback added by ID, keeping the ID registry in sync
 * when the callback removes itself by returning false.
 */
class IdleCallbackEntry
{
public:
  IdleCallbackEntry(UiContext& uiContext, Dali::UiContext::IdleCallbackId id, CallbackBase* callback)
  : mUiContext(&uiContext),
    mId(id),
    mCallback(callback)
  {
  }

  bool operator()()
  {
    bool repeat = CallbackBase::ExecuteReturn<bool>(*mCallback);
    if(!repeat)
    {
      mUiContext->OnIdleCallbackFinished(mId);
    }
    return repeat;
  }

private:
  UiContext*                      mUiContext;
  Dali::UiContext::IdleCallbackId mId;
  std::shared_ptr<CallbackBase>   mCallback; ///< Shared because CallbackFunctorReturn0 stores a copy of this entry
};
} // unnamed namespace

Dali::UiContext UiContext::Get()
{
  Dali::UiContext  uiContext;
  SingletonService singletonService(SingletonService::Get());
  if(singletonService)
  {
    // Check whether the async task manager is already created
    Dali::BaseHandle handle = singletonService.GetSingleton(gSingletonTypeInfoId);
    if(handle)
    {
      // If so, downcast the handle of singleton
      uiContext = Dali::UiContext(static_cast<Internal::Adaptor::UiContext*>(handle.GetObjectPtr()));
    }
  }
  return uiContext;
}

bool UiContext::IsAvailable()
{
  return Internal::Adaptor::Adaptor::IsAvailable();
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
      singletonService.Register(gSingletonTypeInfoId, uiContext);
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

void UiContext::RemoveIdle(CallbackBase* callback)
{
  mAdaptor->RemoveIdle(callback);
}

Dali::UiContext::IdleCallbackId UiContext::AddIdle(Dali::Callback<bool()> callback)
{
  CallbackBase* innerCallback = callback.Release();
  if(!innerCallback)
  {
    return 0u;
  }

  Dali::UiContext::IdleCallbackId id = ++mNextIdleCallbackId;
  while(id == 0u || mIdleCallbacks.count(id) != 0u) // 0 is reserved for failure; IDs may wrap around
  {
    id = ++mNextIdleCallbackId;
  }

  // The adaptor takes ownership of the registered callback: it deletes it on
  // failure, on removal, and when the entry stops repeating.
  CallbackBase* registeredCallback = new CallbackFunctorReturn0<IdleCallbackEntry, bool>(IdleCallbackEntry(*this, id, innerCallback));
  if(!mAdaptor->AddIdle(registeredCallback, true))
  {
    return 0u;
  }

  mIdleCallbacks[id] = registeredCallback;
  return id;
}

void UiContext::RemoveIdle(Dali::UiContext::IdleCallbackId id)
{
  auto iter = mIdleCallbacks.find(id);
  if(iter != mIdleCallbacks.end())
  {
    CallbackBase* registeredCallback = iter->second;
    mIdleCallbacks.erase(iter);
    mAdaptor->RemoveIdle(registeredCallback);
  }
}

void UiContext::OnIdleCallbackFinished(Dali::UiContext::IdleCallbackId id)
{
  mIdleCallbacks.erase(id);
}

void UiContext::FlushPendingChanges()
{
  mAdaptor->FlushUpdateMessages();
}

void UiContext::RenderOnce()
{
  mAdaptor->RenderOnce();
}

void UiContext::AddFrameCallback(FrameCallbackInterface& frameCallback, Dali::Actor rootActor)
{
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).GetCore().AddFrameCallback(frameCallback, rootActor);
}

void UiContext::RemoveFrameCallback(FrameCallbackInterface& frameCallback)
{
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).GetCore().RemoveFrameCallback(frameCallback);
}

Dali::UpdateProxy::NotifySyncPoint UiContext::NotifyFrameCallback(FrameCallbackInterface& frameCallback)
{
  return Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).GetCore().NotifyFrameCallback(frameCallback);
}

void UiContext::SetRenderingBehavior(Dali::UiContext::RenderingBehavior renderingBehavior)
{
  mAdaptor->SetRenderingBehavior(static_cast<Dali::Integration::RenderingBehavior>(renderingBehavior));
}

Dali::UiContext::RenderingBehavior UiContext::GetRenderingBehavior() const
{
  return static_cast<Dali::UiContext::RenderingBehavior>(mAdaptor->GetRenderingBehavior());
}

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE
