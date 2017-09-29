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

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{

namespace Adaptor
{
class Widget;
}

}

class Window;

/**
 * @brief Widget object should be created by WidgetApplication.
 *
 * The WidgetApplication class emits several widget instance lifecycle signals
 * which the user can connect to.
 * The user should connect to the CreateSignal of the Widget and
 * create the Dali Widget object in the connected callback.
 *
 * Widget should follow the example below:
 *
 * @code
 * class ExampleController: public ConnectionTracker
 * {
 * public:
 *   ExampleController( WidgetApplication& widgetApplication )
 *   : mWidgetApplication( widgetApplication )
 *   {
 *     mWidgetApplication.InitSignal().Connect( this, &ExampleController::Create );
 *   }
 *
 *   void Create( WidgetApplication& widgetApplication )
 *   {
 *     Widget widget = Widget::New( WIDGET_ID );
 *     widget.CreateSignal( this, &ExampleController::WidgetCreate );
 *   }
 *
 *   void WidgetCreate( const std::string& id, bundle* content, Window window )
 *   {
 *     // Do Dali components...
 *   }
 *  ...
 * private:
 *   WidgetApplication& mWidgetApplication;
 * };
 *
 * int main (int argc, char **argv)
 * {
 *   WidgetApplication app = WidgetApplication::New(&argc, &argv);
 *   ExampleController example( app );
 *   app.MainLoop();
 * }
 *
 * @SINCE_1_2.62
 */
class DALI_IMPORT_API Widget : public BaseHandle
{
public:

  /**
   * @brief Enumeration for terminate type of widget instance.
   * @SINCE_1_2.62
   */
  typedef enum
  {
    PERMANENT, //< User deleted this widget from the viewer @SINCE_1_2.62
    TEMPORARY, //< Widget is deleted because of other reasons (e.g. widget process is terminated temporarily by the system) @SINCE_1_2.62
  } WidgetTerminateType;

  typedef Signal< void (const std::string&, bundle*, Window) > WidgetCreateSignalType;                  ///< Widget lifecycle signal type @SINCE_1_2.62
  typedef Signal< void (const std::string&, bundle*, WidgetTerminateType) > WidgetTerminateSignalType;  ///< Widget lifecycle signal type @SINCE_1_2.62
  typedef Signal< void (const std::string&) > WidgetPauseSignalType;                                    ///< Widget lifecycle signal type @SINCE_1_2.62
  typedef Signal< void (const std::string&) > WidgetResumeSignalType;                                   ///< Widget lifecycle signal type @SINCE_1_2.62
  typedef Signal< void (const std::string&, Window) > WidgetResizeSignalType;                           ///< Widget lifecycle signal type @SINCE_1_2.62
  typedef Signal< void (const std::string&, bundle*, int) > WidgetUpdateSignalType;                     ///< Widget lifecycle signal type @SINCE_1_2.62

public:

  /**
   * @brief This is the constructor for Widget.
   * @SINCE_1_2.62
   * @param[in]  id  Id for widget class
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
   * @SINCE_1_2.62
   * @param[in] Widget Handle to an object
   */
  Widget( const Widget& widget );

  /**
   * @brief Assignment operator.
   * @SINCE_1_2.62
   * @param[in] Widget Handle to an object
   * @return A reference to this
   */
  Widget& operator=( const Widget& widget );

  /**
   * @brief Destructor
   * @SINCE_1_2.62
   */
  ~Widget();

public:  // Signals

  /**
   * @brief The user should connect to this signal to determine when they create widget.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  WidgetCreateSignalType& CreateSignal();

  /**
   * @brief The user should connect to this signal to determine when they terminate widget.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  WidgetTerminateSignalType& TerminateSignal();

  /**
   * @brief The user should connect to this signal to determine when they pause widget.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  WidgetPauseSignalType& PauseSignal();

  /**
   * @brief The user should connect to this signal to determine when they resume widget.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  WidgetResumeSignalType& ResumeSignal();

  /**
   * @brief The user should connect to this signal to determine when they resize widget.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  WidgetResizeSignalType& ResizeSignal();

  /**
   * @brief The user should connect to this signal to determine when they update widget.
   * @SINCE_1_2.62
   * @return The signal to connect to
   */
  WidgetUpdateSignalType& UpdateSignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief Internal constructor.
   */
  explicit DALI_INTERNAL Widget(Internal::Adaptor::Widget* widget);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // ___DALI_WIDGET_H__
