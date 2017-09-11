#ifndef __DALI_WIDGET_H__
#define __DALI_WIDGET_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/signals/dali-signal.h>
#include <bundle.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/math/uint-16-pair.h>

// INTERNAL INCLUDES
#include "widget-data.h"

namespace Dali
{

namespace Internal DALI_INTERNAL
{

namespace Adaptor
{
class Widget;
}

}

class WidgetData;

/**
 * @brief Widget object should be created by WidgetApplication.
 */

class DALI_IMPORT_API Widget : public BaseHandle
{
public:

  /**
   * @brief Enumeration for terminate type of widget instance.
   */
  typedef enum
  {
    PERMANENT, /**< User deleted this widget from the viewer */
    TEMPORARY, /**< Widget is deleted because of other reasons (e.g. widget process is terminated temporarily by the system) */
  } WidgetTerminateType;

  typedef enum
  {
    APP_DEAD = 0,
    CREATE  = 1,    /**< The widget is created */
    DESTROY = 2,    /**< The widget is destroyed */
    PAUSE   = 3,    /**< The widget is paused */
    RESUME  = 4,    /**< The widget is resumed */
  } WidgetLifecycleEventType;

  typedef Uint16Pair WindowSize;

  typedef Signal< void (WidgetData, bundle*, WindowSize) > WidgetInstanceCreateSignalType;  ///< Widget instance lifecycle signal and system signal callback type
  typedef Signal< void (WidgetData, bundle*, WidgetTerminateType) > WidgetInstanceTerminateSignalType;  ///< Widget instance lifecycle signal and system signal callback type
  typedef Signal< void (WidgetData) > WidgetInstancePauseSignalType;  ///< Widget instance lifecycle signal and system signal callback type
  typedef Signal< void (WidgetData) > WidgetInstanceResumeSignalType;  ///< Widget instance lifecycle signal and system signal callback type
  typedef Signal< void (WidgetData, WindowSize) > WidgetInstanceResizeSignalType;  ///< Widget instance lifecycle signal and system signal callback type
  typedef Signal< void (WidgetData, bundle*, int) > WidgetInstanceUpdateSignalType;  ///< Widget instance lifecycle signal and system signal callback type

public:

  /**
   * @brief This is the constructor for Widget.
   * @param[in]      id for widget instance
   * @return A handle to the Widget
   */
  static Widget New( const std::string& id );

  /**
   * @brief The default constructor.
   *
   */
  Widget();

  /**
   * @brief Copy Constructor.
   * @param[in] Widget Handle to an object
   */
  Widget( const Widget& widget );

  /**
   * @brief Assignment operator.
   * @param[in] Widget Handle to an object
   * @return A reference to this
   */
  Widget& operator=( const Widget& widget );

  /**
   * @brief Destructor
   *
   */
  ~Widget();

public:  // Signals

  /**
   * @brief The user should connect to this signal to determine when they create widget instance.
   * @return The signal to connect to
   */
  WidgetInstanceCreateSignalType& CreateSignal();

  /**
   * @brief The user should connect to this signal to determine when they terminate widget instance.
   * @return The signal to connect to
   */
  WidgetInstanceTerminateSignalType& TerminateSignal();

  /**
   * @brief This signal is emitted when the language is changed on the device.
   * @return The signal to connect to
   */
  WidgetInstancePauseSignalType& PauseSignal();

  /**
  * @brief This signal is emitted when the region of the device is changed.
  * @return The signal to connect to
  */
  WidgetInstanceResumeSignalType& ResumeSignal();

  /**
  * @brief This signal is emitted when the battery level of the device is low.
  * @return The signal to connect to
  */
  WidgetInstanceResizeSignalType& ResizeSignal();

  /**
  * @brief This signal is emitted when the memory level of the device is low.
  * @return The signal to connect to
  */
  WidgetInstanceUpdateSignalType& UpdateSignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief Internal constructor.
   */
  explicit DALI_INTERNAL Widget(Internal::Adaptor::Widget* widget);
  /// @endcond
};

} // namespace Dali

#endif // ___DALI_WIDGET_H__
