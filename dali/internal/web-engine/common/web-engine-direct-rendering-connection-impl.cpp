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

#include <dali/devel-api/adaptor-framework/lifecycle-controller.h>
#include <dali/devel-api/rendering/renderer-devel.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/internal/web-engine/common/web-engine-direct-rendering-connection-impl.h>

namespace DALI_NAMESPACE::Internal::Adaptor
{
WebEngineDirectRenderingState::WebEngineDirectRenderingState(WebEngineDirectRenderingSessionPtr session, Completion completion)
: mCompletion(std::move(completion)),
  mSession(std::move(session)),
  mCallback(RenderCallback::New(this, &WebEngineDirectRenderingState::OnRender, RenderCallback::ExecutionMode::ISOLATED)),
  mRenderer(DevelRenderer::New(mCallback)),
  mTrigger(std::make_unique<EventThreadCallback>(MakeCallback(this, &WebEngineDirectRenderingState::OnEvent)))
{
  Dali::LifecycleController::Get().TerminateSignal().Connect(this, &WebEngineDirectRenderingState::OnApplicationTerminate);
}

WebEngineDirectRenderingState::~WebEngineDirectRenderingState()
{
  mCallback->Invalidate();
}

Dali::Renderer WebEngineDirectRenderingState::GetRenderer() const
{
  return mRenderer;
}

void WebEngineDirectRenderingState::SetCallbacks(std::function<void()> requestFrame, std::function<void()> frameSubmitted)
{
  if(mStopped || mCompleted)
  {
    return;
  }
  mFrameSubmitted = std::move(frameSubmitted);
  mSession->RegisterRenderRequestedCallback(std::move(requestFrame));
}

void WebEngineDirectRenderingState::Stop()
{
  if(mStopped || mCompleted)
  {
    return;
  }
  mStopped        = true;
  mFrameSubmitted = {};
  mSession->RegisterRenderRequestedCallback({});
  mSelfReference = this;
  if(!Dali::Adaptor::IsAvailable())
  {
    Complete();
    return;
  }
  DevelRenderer::TerminateRenderCallback(mRenderer);
  Dali::Adaptor::Get().RequestProcessEventsAndUpdate();
}

bool WebEngineDirectRenderingState::OnRender(const RenderCallbackInput& input)
{
  if(mTerminated.load())
  {
    return true;
  }
  if(input.isTerminated)
  {
    if(input.isNativeApiUsable)
    {
      ReleaseGraphicsResources(input);
    }
    OnStopped();
    return true;
  }
  if(mSession->Render(input))
  {
    mFrameReady.store(true);
    mTrigger->Trigger();
  }
  return true;
}

void WebEngineDirectRenderingState::ReleaseGraphicsResources(const RenderCallbackInput& input)
{
  // Visited targets share one session; release its graphics resources at most once.
  if(!mGraphicsReleased)
  {
    mSession->ReleaseGraphicsResources(input);
    mGraphicsReleased = true;
  }
}

void WebEngineDirectRenderingState::OnStopped()
{
  if(!mTerminated.exchange(true))
  {
    mSession->OnStopped();
    mTrigger->Trigger();
  }
}

void WebEngineDirectRenderingState::OnEvent()
{
  IntrusivePtr<WebEngineDirectRenderingState> keepAlive(this);
  if(mTerminated.load())
  {
    Complete();
    return;
  }
  if(mFrameReady.exchange(false) && mFrameSubmitted)
  {
    auto submitted = mFrameSubmitted;
    submitted();
  }
}

void WebEngineDirectRenderingState::OnApplicationTerminate()
{
  // Event processing may end before render-thread termination is delivered.
  // Invalidation joins an in-flight callback. Session destruction must not use GL;
  // shared GPU resources may remain until the graphics share group is destroyed.
  Complete();
}

void WebEngineDirectRenderingState::Complete()
{
  if(mCompleted)
  {
    return;
  }
  IntrusivePtr<WebEngineDirectRenderingState> keepAlive(this);
  mCompleted = true;
  mCallback->Invalidate();
  // Shutdown cannot depend on another event-loop iteration. Invalidation has
  // joined any active render/cleanup callback, so CPU retirement is safe here.
  if(!mTerminated.exchange(true))
  {
    mSession->OnStopped();
  }
  Dali::LifecycleController::Get().TerminateSignal().Disconnect(this, &WebEngineDirectRenderingState::OnApplicationTerminate);
  mFrameSubmitted = {};
  mSession->RegisterRenderRequestedCallback({});
  mSession.Reset();
  mRenderer.Reset();
  auto completion = std::move(mCompletion);
  completion(*this);
  mSelfReference.Reset();
}
} // namespace DALI_NAMESPACE::Internal::Adaptor

namespace DALI_NAMESPACE
{
WebEngineDirectRenderingConnection::WebEngineDirectRenderingConnection()  = default;
WebEngineDirectRenderingConnection::~WebEngineDirectRenderingConnection() = default;

WebEngineDirectRenderingConnection::WebEngineDirectRenderingConnection(Internal::Adaptor::WebEngineDirectRenderingConnection* implementation)
: BaseHandle(implementation)
{
}

Renderer WebEngineDirectRenderingConnection::GetRenderer() const
{
  return static_cast<const Internal::Adaptor::WebEngineDirectRenderingConnection&>(GetBaseObject()).mState->GetRenderer();
}

void WebEngineDirectRenderingConnection::SetCallbacks(std::function<void()> requestFrame, std::function<void()> frameSubmitted)
{
  static_cast<Internal::Adaptor::WebEngineDirectRenderingConnection&>(GetBaseObject()).mState->SetCallbacks(std::move(requestFrame), std::move(frameSubmitted));
}

void WebEngineDirectRenderingConnection::Close()
{
  static_cast<Internal::Adaptor::WebEngineDirectRenderingConnection&>(GetBaseObject()).mState->Stop();
}
} // namespace DALI_NAMESPACE
