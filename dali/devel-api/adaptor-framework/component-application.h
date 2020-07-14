#ifndef DALI_COMPONENT_APPLICATION_H
#define DALI_COMPONENT_APPLICATION_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
namespace Internal DALI_INTERNAL
{

namespace Adaptor
{
class ComponentApplication;
}

}

/**
 * @brief An ComponentApplication class object should be created by every component-based application
 * that wishes to use Dali.
 *
 * Component application draw multiple UI applications based on frame components.
 * component application can draw multiple UIs in one process.
 *
 * It provides a means for initializing the
 * resources required by the Dali::Core.
 *
 * The ComponentApplication class emits several signals which the user can
 * connect to.  The user should not create any Dali objects in the main
 * function and instead should connect to the Init signal of the
 * ComponentApplication and create the Dali Widget object in the connected callback.
 *
 */
class DALI_ADAPTOR_API ComponentApplication : public Application
{
public:
  typedef Signal< Any () > CreateSignalType;

public:
  /**
   * @brief This is the constructor for component applications without an argument list.
   * @return A handle to the ComponentApplication
   */
  static ComponentApplication New( );

  /**
   * @brief This is the constructor for component applications.
   *
   * @param[in,out]  argc        A pointer to the number of arguments
   * @param[in,out]  argv        A pointer to the argument list
   * @return A handle to the ComponentApplication
   */
  static ComponentApplication New( int* argc, char **argv[] );

  /**
   * @brief This is the constructor for component applications with a stylesheet
   *
   * @param[in,out]  argc        A pointer to the number of arguments
   * @param[in,out]  argv        A pointer to the argument list
   * @param[in]      stylesheet  The path to user defined theme file
   * @return A handle to the ComponentApplication
   */
  static ComponentApplication New( int* argc, char **argv[], const std::string& stylesheet );

  /**
   * @brief The default constructor.
   */
  ComponentApplication() = default;

  /**
   * @brief Copy Constructor.
   *
   * @param[in] componentApplication Handle to an object
   */
  ComponentApplication( const ComponentApplication& componentApplication ) = default;

  /**
   * @brief Assignment operator.
   *
   * @param[in] componentApplication Handle to an object
   * @return A reference to this
   */
  ComponentApplication& operator=( const ComponentApplication& componentApplication ) = default;

 /**
   * @brief Destructor
   */
  ~ComponentApplication() = default;

 /**
  * @brief The user should connect to this signal to determine when they should initialize
  * their application.
  * The callback function is called before the main loop of the application starts.
  * @return The signal to connect to
  */
  CreateSignalType& CreateSignal();

public:
  /// @cond internal
  /**
   * @brief Internal constructor.
   */
  explicit DALI_INTERNAL ComponentApplication(Internal::Adaptor::ComponentApplication* componentApplication);
  /// @endcond
};
} // namespace Dali

#endif // DALI_COMPONENT_APPLICATION_H

