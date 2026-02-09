#ifndef DALI_INTERNAL_ADAPTOR_COMMON_UI_CONTEXT_H
#define DALI_INTERNAL_ADAPTOR_COMMON_UI_CONTEXT_H

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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/ui-context.h>

namespace Dali
{
class Adaptor;

namespace Internal
{
namespace Adaptor
{
/**
 * @brief Implementation of the UiContext class.
 *
 * This internal class implements the public UiContext interface.
 */
class UiContext : public BaseObject
{
public:
  /**
   * @copydoc Dali::UiContext::Get()
   */
  static Dali::UiContext Get();

  /**
   * @brief Creates a new UiContext instance with the given adaptor.
   *
   * @param[in] adaptor Pointer to the Dali::Adaptor instance
   * @return A handle to the UiContext instance
   */
  static Dali::UiContext New(Dali::Adaptor* adaptor);

  /**
   * @brief Destructor.
   */
  ~UiContext();

  /**
   * @brief Sets the default window for the UI context.
   *
   * @param[in] window The window to set as default
   */
  void SetDefaultWindow(Dali::Window window);

  /**
   * @copydoc Dali::UiContext::GetDefaultWindow()
   */
  Dali::Window GetDefaultWindow();

  /**
   * @copydoc Dali::UiContext::AddIdle()
   */
  bool AddIdle(CallbackBase* callback);

  /**
   * @copydoc Dali::UiContext::GetRenderThreadId()
   */
  int32_t GetRenderThreadId();

  /**
   * @copydoc Dali::UiContext::FlushUpdateMessages()
   */
  void FlushUpdateMessages();

  /**
   * @copydoc Dali::UiContext::SetApplicationLocale()
   */
  void SetApplicationLocale(const std::string& locale);

private:
  /**
   * @brief Private constructor with adaptor.
   *
   * @param[in] adaptor Pointer to the Dali::Adaptor instance
   */
  UiContext(Dali::Adaptor* adaptor);

  // Deleted constructors and assignment operators
  UiContext(const UiContext&)            = delete; ///< Deleted copy constructor.
  UiContext(UiContext&&)                 = delete; ///< Deleted move constructor.
  UiContext& operator=(const UiContext&) = delete; ///< Deleted copy assignment operator.
  UiContext& operator=(UiContext&&)      = delete; ///< Deleted move assignment operator.

private:
  Dali::Adaptor* mAdaptor{nullptr}; ///< Pointer to the adaptor instance
  Dali::Window   mDefaultWindow;    ///< The default window handle
};

/**
 * @brief Retrieves the internal implementation from the public handle.
 *
 * @param[in] uiContext The public UiContext handle
 * @return Reference to the internal UiContext implementation
 */
inline UiContext& GetImplementation(Dali::UiContext& uiContext)
{
  DALI_ASSERT_ALWAYS(uiContext && "UiContext handle is empty");

  BaseObject& handle = uiContext.GetBaseObject();

  return static_cast<Internal::Adaptor::UiContext&>(handle);
}

/**
 * @brief Retrieves the internal implementation from the public handle (const version).
 *
 * @param[in] uiContext The public UiContext handle
 * @return Const reference to the internal UiContext implementation
 */
inline const UiContext& GetImplementation(const Dali::UiContext& uiContext)
{
  DALI_ASSERT_ALWAYS(uiContext && "UiContext handle is empty");

  const BaseObject& handle = uiContext.GetBaseObject();

  return static_cast<const Internal::Adaptor::UiContext&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_COMMON_UI_CONTEXT_H
