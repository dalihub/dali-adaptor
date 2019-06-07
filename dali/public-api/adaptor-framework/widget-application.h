#ifndef DALI_WIDGET_APPLICATION_H
#define DALI_WIDGET_APPLICATION_H

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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/application.h>

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
class WidgetApplication;
}

}

class Widget;

/**
 * @brief An WidgetApplication class object should be created by every widget application
 * that wishes to use Dali.
 *
 * It provides a means for initializing the
 * resources required by the Dali::Core.
 *
 * The WidgetApplication class emits several signals which the user can
 * connect to.  The user should not create any Dali objects in the main
 * function and instead should connect to the Init signal of the
 * WidgetApplication and create the Dali Widget object in the connected callback.
 *
 * WidgetApplications should follow the example below:
 *
 * @code
 *
 * //Widget header which
 * #include <my-widget.h>
 *
 * class ExampleController: public ConnectionTracker
 * {
 * public:
 *   ExampleController( Application& application )
 *   : mWidgetApplication( application )
 *   {
 *     mApplication.InitSignal().Connect( this, &ExampleController::Create );
 *   }
 *
 *   static Widget CreateWidgetFunction(const std::string& widgetName)
 *   {
 *     MyWidget widget = MyWidget::New();
 *     return widget;
 *   }
 *
 *   void Create( Application& application )
 *   {
 *     mApplication.RegisterWidgetCreatingFunction( "myWidget", &ExampleController::CreateWidgetFunction );
 *   }
 *
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
 * @endcode
 *
 * If required, you can also connect class member functions to a signal:
 *
 * @code
 * MyWidgetApplication app;
 * app.ResumeSignal().Connect(&app, &MyWidgetApplication::Resume);
 * @endcode
 *
 * @SINCE_1_3_5
 */
class DALI_ADAPTOR_API WidgetApplication : public Application
{
public:

  /**
   * @brief This is the typedef for Widget creator.
   * @SINCE_1_3_5
   */
  typedef Widget(*CreateWidgetFunction)(const std::string&);

public:

  /**
   * @brief This is the constructor for WidgetApplications with a name.
   *
   * @SINCE_1_3_5
   * @param[in,out]  argc        A pointer to the number of arguments
   * @param[in,out]  argv        A pointer to the argument list
   * @param[in]      stylesheet  The path to user defined theme file
   * @return A handle to the WidgetApplication
   * @note If the stylesheet is not specified, then the library's default stylesheet will not be overridden.
   */
  static WidgetApplication New( int* argc, char **argv[], const std::string& stylesheet );

  /**
   * @brief The default constructor.
   * @SINCE_1_3_5
   */
  WidgetApplication();

  /**
   * @brief Copy Constructor.
   *
   * @SINCE_1_3_5
   * @param[in] widgetApplication Handle to an object
   */
  WidgetApplication( const WidgetApplication& widgetApplication );

  /**
   * @brief Assignment operator.
   *
   * @SINCE_1_3_5
   * @param[in] widgetApplication Handle to an object
   * @return A reference to this
   */
  WidgetApplication& operator=( const WidgetApplication& widgetApplication );

 /**
   * @brief Destructor
   * @SINCE_1_3_5
   */
  ~WidgetApplication();

  /**
   * @brief Register create function for widget.
   *
   * @SINCE_1_3_5
   * @param[in] widgetName  Name of widget
   * @param[in] createFunction     Function pointer for widget creation.
   */
  void RegisterWidgetCreatingFunction( const std::string& widgetName, CreateWidgetFunction createFunction );

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief Internal constructor.
   */
  explicit DALI_INTERNAL WidgetApplication(Internal::Adaptor::WidgetApplication* widgetApplication);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_WIDGET_APPLICATION_H
