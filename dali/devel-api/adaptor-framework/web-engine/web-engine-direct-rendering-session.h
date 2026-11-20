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

#include <dali/devel-api/signals/render-callback.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/object/ref-object.h>
#include <functional>

namespace DALI_NAMESPACE
{
/**
 * @brief Optional per-instance GL output for a web engine.
 *
 * Returned by the plugin InitializeDirectRendering() to the adaptor. The adaptor
 * keeps the engine alive until this session is terminated and released. Its
 * destructor must not call GL. Event-thread notifications and Render() may overlap:
 * implementations must synchronize scene snapshots without waiting for the event
 * thread from Render(). This interface supports window-target EGL/GLES rendering;
 * resource-context support is required. Offscreen RenderTasks and other graphics
 * backends are not guaranteed.
 * An offscreen-only session may remain retained until application shutdown.
 * Termination uses the existing core isTerminated/isNativeApiUsable callback
 * contract; it does not add delivery guarantees for skipped native commands.
 * Only share-group resources (for example programs and buffers) may persist across
 * Render() calls. Current contexts can change when moving between windows within
 * the same share group. Re-establish GL state on every draw; do not cache VAOs,
 * FBOs or EGL context identities. Concurrent multi-target rendering is unsupported.
 */
class WebEngineDirectRenderingSession : public RefObject
{
public:
  using RenderRequestedCallback = std::function<void()>;

  /**
   * @brief Sets or clears the event-thread notification requesting a DALi frame.
   * All notifications must arrive on the event thread; clearing cancels pending ones.
   */
  virtual void RegisterRenderRequestedCallback(RenderRequestedCallback callback) = 0;

  /**
   * @brief Replays the latest scene into DALi's current window target.
   * Called on the DALi render thread with an isolated EGL context. Initialize GL
   * resources lazily here. Honor input.mvp, clippingBox and worldColorMultiplier.
   * Do not swap buffers, replace the EGL context, or retain a pointer to input.
   * Draw even without new web content: the window target may need reconstruction.
   * The engine owns GL blending and video-hole output. Renderer BLEND_MODE does
   * not configure GL blend state for this native callback.
   * @return True when a new web-content revision was submitted, not on every replay.
   */
  virtual bool Render(const RenderCallbackInput& input) = 0;

  /**
   * Releases shared graphics resources at most once in a usable render-thread
   * context if the first termination delivery permits native API use. Later
   * targets are not searched for a usable context after CPU retirement. The session
   * must not use context-local persistent objects. Completion is reported separately.
   */
  virtual void ReleaseGraphicsResources(const RenderCallbackInput& input) = 0;

  /**
   * CPU-only stop notification, exactly once after all Render and graphics-release
   * calls finish. Normally called on the render thread; shutdown may call it on the
   * event thread after joining outstanding calls. Never call GL or wait for the
   * event thread here. Destruction and plugin Destroy follow on the event thread.
   * Unreleased GPU resources may survive until the share group is destroyed.
   */
  virtual void OnStopped() = 0;

protected:
  ~WebEngineDirectRenderingSession() override = default;
};

using WebEngineDirectRenderingSessionPtr = IntrusivePtr<WebEngineDirectRenderingSession>;
} // namespace DALI_NAMESPACE
