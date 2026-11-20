#pragma once

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

#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-direct-rendering-connection.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-direct-rendering-session.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/signals/connection-tracker.h>
#include <atomic>
#include <memory>

namespace DALI_NAMESPACE::Internal::Adaptor
{
class WebEngineDirectRenderingState : public RefObject, public ConnectionTracker
{
public:
  using Completion = std::function<void(WebEngineDirectRenderingState&)>;
  WebEngineDirectRenderingState(WebEngineDirectRenderingSessionPtr session, Completion completion);
  Dali::Renderer GetRenderer() const;
  void           SetCallbacks(std::function<void()> requestFrame, std::function<void()> frameSubmitted);
  void           Stop();

private:
  ~WebEngineDirectRenderingState() override;
  bool OnRender(const RenderCallbackInput& input);
  void OnEvent();
  void ReleaseGraphicsResources(const RenderCallbackInput& input);
  void OnStopped();
  void OnApplicationTerminate();
  void Complete();

  Completion                                  mCompletion; ///< Retains the engine until CPU retirement completes.
  WebEngineDirectRenderingSessionPtr          mSession;
  RenderCallbackPtr                           mCallback;
  Dali::Renderer                              mRenderer;
  std::unique_ptr<EventThreadCallback>        mTrigger;
  std::function<void()>                       mFrameSubmitted;
  IntrusivePtr<WebEngineDirectRenderingState> mSelfReference;
  std::atomic<bool>                           mTerminated{false};
  std::atomic<bool>                           mFrameReady{false};
  bool                                        mGraphicsReleased{false}; ///< Render thread only. Session owns share-group resources.
  bool                                        mStopped{false};
  bool                                        mCompleted{false};
};

class WebEngineDirectRenderingConnection : public BaseObject
{
public:
  explicit WebEngineDirectRenderingConnection(WebEngineDirectRenderingState* state)
  : mState(state)
  {
  }

  IntrusivePtr<WebEngineDirectRenderingState> mState;

private:
  ~WebEngineDirectRenderingConnection() override
  {
    mState->Stop();
  }
};
} // namespace DALI_NAMESPACE::Internal::Adaptor
