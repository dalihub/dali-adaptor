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

#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/rendering/renderer.h>
#include <functional>

namespace DALI_NAMESPACE
{
namespace Internal::Adaptor
{
class WebEngineDirectRenderingConnection;
}

/**
 * Event-thread connection to adaptor-managed web engine output.
 * Attach GetRenderer() to one actor. Moving that actor between windows in the
 * same graphics share group is supported; simultaneous rendering to multiple
 * targets and persistent context-local GL objects are not supported.
 * Close() or release the last connection handle to stop output. The adaptor
 * retains the engine until in-flight rendering and termination have finished.
 */
class DALI_ADAPTOR_API WebEngineDirectRenderingConnection : public BaseHandle
{
public:
  WebEngineDirectRenderingConnection();
  ~WebEngineDirectRenderingConnection();

  Renderer GetRenderer() const;

  /** Event-thread callbacks. Empty callbacks cancel subsequent notifications. */
  void SetCallbacks(std::function<void()> requestFrame, std::function<void()> frameSubmitted);

  /**
   * Idempotent asynchronous close. Detach the renderer from its actor as well.
   * Retires the session and calls the owning plugin's Destroy() on completion.
   * Do not issue further engine operations for this session after Close().
   * Return from Close() does not signal completion. This connection provides no
   * completion notification or supported protocol for reinitializing a retained
   * WebEngine handle; create a new engine for a new session.
   */
  void Close();

  explicit DALI_INTERNAL WebEngineDirectRenderingConnection(Internal::Adaptor::WebEngineDirectRenderingConnection* implementation);
};
} // namespace DALI_NAMESPACE
