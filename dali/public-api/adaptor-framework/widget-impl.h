#ifndef DALI_INTERNAL_WIDGET_H
#define DALI_INTERNAL_WIDGET_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/signals/connection-tracker-interface.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/widget.h>

namespace Dali
{
class Window;

/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal
{

namespace Adaptor
{

class Widget;
typedef IntrusivePtr<Widget> WidgetPtr;

/**
 * @brief This is the internal base class of custom widget.
 *
 * It will provides several widget instance lifecycle virtual functions
 * which the user can override.
 *
 * User should override OnCreate function and create scene for custom widget.
 *
 * Plus, Implements ConnectionTrackerInterface so that signals (typically connected to member functions) will
 * be disconnected automatically when the control is destroyed.
 *
 * @SINCE_1_3_5
 */
class DALI_ADAPTOR_API Widget : public BaseObject, public ConnectionTrackerInterface
{
public:

  /**
   * @brief Creates a new WidgetImpl instance.
   *
   * @SINCE_1_3_5
   * @return A handle to the WidgetImpl instance
   */
  static WidgetPtr New();

  /**
   * @brief The user should override this function to determine when they create widget.
   *
   * @SINCE_1_3_5
   * @param[in] contentInfo Information from WidgetView for creating. It contains previous status of widget which is sent by SetContentInfo before.
   * @param[in] window Window handle for widget
   */
  virtual void OnCreate( const std::string& contentInfo, Dali::Window window );

  /**
   * @brief The user should override this function to determine when they terminate widget.
   *
   * @SINCE_1_3_5
   * @param[in] contentInfo Data from WidgetView for deleting
   * @param[in] type Termination type. When user delete widget view, termination type is PERMANENT.
   */
  virtual void OnTerminate( const std::string& contentInfo, Dali::Widget::Termination type );

  /**
   * @brief The user should override this function to determine when they pause widget.
   * @SINCE_1_3_5
   */
  virtual void OnPause();

  /**
   * @brief The user should override this function to determine when they resume widget.
   * @SINCE_1_3_5
   */
  virtual void OnResume();

  /**
   * @brief The user should override this function to determine when they resize widget.
   *
   * @SINCE_1_3_5
   * @param[in] window Window handle for widget
   */
  virtual void OnResize( Dali::Window window );

  /**
   * @brief The user should override this function to determine when they update widget.
   *
   * @SINCE_1_3_5
   * @param[in] contentInfo Data from WidgetView for updating
   * @param[in] force Although the widget is paused, if it is true, the widget can be updated
   */
  virtual void OnUpdate( const std::string& contentInfo, int force );

  // From ConnectionTrackerInterface

  /**
   * @copydoc ConnectionTrackerInterface::SignalConnected
   */
  virtual void SignalConnected( SlotObserver* slotObserver, CallbackBase* callback );

  /**
   * @copydoc ConnectionTrackerInterface::SignalDisconnected
   */
  virtual void SignalDisconnected( SlotObserver* slotObserver, CallbackBase* callback );

  /**
   * @brief Set content info to WidgetView.
   *
   * @SINCE_1_3_5
   * @param[in] contentInfo Content info is kind of context information which contains current status of widget.
   */
  void SetContentInfo( const std::string& contentInfo );

protected:

  /**
   * @brief WidgetImpl constructor
   */
  Widget();

  /**
   * @brief Virtual destructor
   */
  virtual ~Widget();

  /// @cond internal
public:
  class Impl; // Class declaration is public so we can internally add devel API's to the WidgetImpl

  /**
   * Set pointer of WidgetImpl Internal.
   * @SINCE_1_3_5
   */
  void SetImpl( Impl* impl );

private:
  Impl* mImpl;

  // Undefined
  DALI_INTERNAL Widget(const Widget&);
  DALI_INTERNAL Widget& operator=(Widget&);
  /// @endcond

};

/**
 * @brief Gets implementation from the handle.
 *
 * @SINCE_1_3_5
 * @param handle
 * @return Implementation
 * @pre handle is initialized and points to a widget
 */
DALI_ADAPTOR_API Internal::Adaptor::Widget& GetImplementation( Dali::Widget& widget );

/**
 * @brief Gets implementation from the handle.
 *
 * @SINCE_1_3_5
 * @param handle
 * @return Implementation
 * @pre Handle is initialized and points to a widget.
 */
DALI_ADAPTOR_API const Internal::Adaptor::Widget& GetImplementation( const Dali::Widget& widget );

} // namespace Adaptor

} // namespace Internal

/**
 * @}
 */

} // namespace Dali
#endif // DALI_INTERNAL_WIDGET_H
