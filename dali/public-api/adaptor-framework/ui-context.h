#ifndef DALI_UI_CONTEXT_H
#define DALI_UI_CONTEXT_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/update/frame-callback-interface.h>
#include <dali/public-api/update/update-proxy.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/common/dali-string.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

namespace Internal
{
namespace Adaptor
{
class UiContext;
} // namespace Adaptor
} // namespace Internal

/**
 * @brief UiContext provides access to UI-related functionality and context.
 *
 * UiContext is a singleton that provides access to UI context information including the default window,
 * render thread management, idle callbacks, and application locale settings.
 *
 * Usage example:
 * @code
 * UiContext context = UiContext::Get();
 * Dali::Window window = context.GetDefaultWindow();
 * // Use window and context for UI operations
 * @endcode
 *
 * @note UiContext is a singleton - use Get() to retrieve the instance
 * @SINCE_2_5.10
 */
class DALI_ADAPTOR_API UiContext : public BaseHandle
{
public:
  /**
   * @brief Retrieves the singleton UiContext instance.
   *
   * @return A handle to the UiContext instance
   * @SINCE_2_5.10
   */
  static UiContext Get();

  /**
   * @brief Constructs an empty handle.
   * @SINCE_2_5.10
   */
  UiContext();

  /**
   * @brief Destructor.
   * @SINCE_2_5.10
   */
  ~UiContext();

  /**
   * @brief Retrieves the main window.
   *
   * The application writer can use the window to build a scene.
   *
   * @return A handle to the window
   * @SINCE_2_5.10
   */
  Dali::Window GetDefaultWindow();

  /**
   * @brief Ensures that the function passed in is called from the main loop when it is idle.
   *
   * @param[in] callback The function to call
   * @return @c true if added successfully, @c false otherwise
   *
   * @note Function must be called from main event thread only
   *
   * A callback of the following type may be used:
   * @code
   *   bool MyFunction();
   * @endcode
   * This callback will be called repeatedly as long as it returns true. A return of false deletes this callback.
   *
   * @note Ownership of the callback is passed onto this class.
   * @SINCE_2_5.10
   */
  bool AddIdle(CallbackBase* callback);

  /**
   * @brief Removes a previously added idle callback.
   *
   * @param[in] callback The callback to remove
   *
   * @note Function must be called from main event thread only
   * @note Ownership of the callback is NOT returned; the callback is deleted.
   * @SINCE_2_5.20
   */
  void RemoveIdle(CallbackBase* callback);

  /**
   * @brief Relayout the application and ensure all pending operations are flushed to the update thread.
   * @SINCE_2_5.10
   */
  void FlushUpdateMessages();

  /**
   * @brief Set the application's language independently of the system language.
   *
   * @param[in] locale Specifies the application's language and region in BCP 47 format (e.g., "en_US", "ko_KR")
   * @SINCE_2_5.10
   */
  void SetApplicationLocale(const Dali::String& locale);

  /**
   * @brief The FrameCallbackInterface implementation added gets called on every frame from the update-thread.
   *
   * @param[in] frameCallback An implementation of the FrameCallbackInterface
   * @param[in] rootActor The root-actor in the scene that the callback applies to. Or empty handle if we don't care whether the node is scene on or not.
   *
   * @note The frameCallback cannot be added more than once. This will assert if that is attempted.
   * @note Only the rootActor and its children will be parsed by the UpdateProxy.
   * @note If the rootActor is destroyed, then the callback is automatically removed.
   * @note If the rootActor is empty handle, given frameCallback will not be removed automatically.
   * @SINCE_2_5.23
   * @see FrameCallbackInterface
   */
  void AddFrameCallback(FrameCallbackInterface& frameCallback, Actor rootActor);

  /**
   * @brief Removes the specified FrameCallbackInterface implementation from being called on every frame.
   *
   * @param[in] frameCallback The FrameCallbackInterface implementation to remove
   *
   * @note This function will block if the FrameCallbackInterface::Update method is being processed in the update-thread.
   * @note If the callback implementation has already been removed, then this is a no-op.
   * @SINCE_2_5.23
   */
  void RemoveFrameCallback(FrameCallbackInterface& frameCallback);

  /**
   * @brief Notify the frame callback that there is a new sync point.
   *
   * The sync point can be matched during a subsequent frameCallback::Update().
   *
   * @param[in] frameCallback The FrameCallbackInterface implementation to notify
   *
   * @return NotifySyncPoint - a unique sync value that will also be sent to the
   * UpdateProxy prior to FrameCallback::Update() being called from the update thread.
   * @SINCE_2_5.23
   */
  UpdateProxy::NotifySyncPoint NotifyFrameCallback(FrameCallbackInterface& frameCallback);

  /**
   * @brief Copy Constructor.
   *
   * @param[in] uiContext Handle to an object
   * @SINCE_2_5.10
   */
  UiContext(const UiContext& uiContext);

  /**
   * @brief Assignment operator.
   *
   * @param[in] uiContext Handle to an object
   * @return A reference to this
   * @SINCE_2_5.10
   */
  UiContext& operator=(const UiContext& uiContext);

  /**
   * @brief Move constructor.
   *
   * @param[in] rhs A reference to the moved handle
   * @SINCE_2_5.10
   */
  UiContext(UiContext&& rhs) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this handle
   * @SINCE_2_5.10
   */
  UiContext& operator=(UiContext&& rhs) noexcept;

public: // Not intended for application developers
  /**
   * @brief Internal constructor.
   *
   * @param[in] uiContext Pointer to the internal implementation
   * @SINCE_2_5.10
   */
  explicit DALI_INTERNAL UiContext(Internal::Adaptor::UiContext* uiContext);
};

} // namespace Dali

#endif // DALI_UI_CONTEXT_H
