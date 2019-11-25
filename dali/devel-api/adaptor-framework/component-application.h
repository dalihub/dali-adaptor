#ifndef DALI_COMPONENT_APPLICATION_H
#define DALI_COMPONENT_APPLICATION_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#ifdef COMPONENT_APPLICATION_SUPPORT

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/application.h>
#include <component_based_app.h>
namespace Dali
{
namespace Internal DALI_INTERNAL
{

namespace Adaptor
{
class ComponentApplication;
}

}

class DALI_ADAPTOR_API ComponentApplication : public Application
{

public:
  static ComponentApplication New( );
  static ComponentApplication New( int* argc, char **argv[] );
  static ComponentApplication New( int* argc, char **argv[], const std::string& stylesheet );

  /**
   * @brief The default constructor.
   */
  ComponentApplication();

  /**
   * @brief Copy Constructor.
   *
   * @param[in] componentApplication Handle to an object
   */
  ComponentApplication( const ComponentApplication& componentApplication );

  /**
   * @brief Assignment operator.
   *
   * @param[in] componentApplication Handle to an object
   * @return A reference to this
   */
  ComponentApplication& operator=( const ComponentApplication& componentApplication );

 /**
   * @brief Destructor
   */
  ~ComponentApplication();

 public:
  typedef Signal< component_class_h (void* user_data) > CreateSignalType;

  CreateSignalType& CreateSignal();

public:
  /// @cond internal
  /**
   * @brief Internal constructor.
   */
  explicit DALI_INTERNAL ComponentApplication(Internal::Adaptor::ComponentApplication* componentApplication);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif

#endif // DALI_COMPONENT_APPLICATION_H

