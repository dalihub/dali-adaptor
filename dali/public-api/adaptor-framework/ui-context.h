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
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
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
   * @brief Gets the render thread id of DALi.
   *
   * @note If render thread id getter doesn't supported, it will return 0 as default.
   * @return The render thread id
   * @SINCE_2_5.10
   */
  int32_t GetRenderThreadId();

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
  void SetApplicationLocale(const std::string& locale);

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
