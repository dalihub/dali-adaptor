#ifndef DALI_INTERNAL_COMPONENT_APPLICATION_H
#define DALI_INTERNAL_COMPONENT_APPLICATION_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/component-application.h>
#include <dali/internal/adaptor/common/application-impl.h>

namespace Dali
{
class Adaptor;

namespace Internal
{
namespace Adaptor
{
class ComponentApplication;
typedef IntrusivePtr<ComponentApplication> ComponentApplicationPtr;

/**
 * Implementation of the ComponentApplication class.
 */
class ComponentApplication : public Application
{
public:
  typedef Dali::ComponentApplication::CreateSignalType CreateSignal;

public:
  /**
   * Create a new component application
   * @param[in]  argc         A pointer to the number of arguments
   * @param[in]  argv         A pointer to the argument list
   * @param[in]  stylesheet   The path to user defined theme file
   * @param[in]  windowMode   A member of WINDOW_MODE
   * @return A handle to the ComponentApplication
   */
  static ComponentApplicationPtr New(int* argc, char** argv[], const std::string& stylesheet, WINDOW_MODE windowMode);

  /**
   * @brief The default constructor.
   */
  ComponentApplication(int* argc, char** argv[], const std::string& stylesheet, WINDOW_MODE windowMode);

  /**
   * @brief Undefined copy constructor.
   */
  ComponentApplication(const ComponentApplication&) = default;

  /**
   * @brief Destructor
   */
  virtual ~ComponentApplication() = default;

  /**
   *@brief Undefined assignment operator.
   */
  ComponentApplication& operator=(const ComponentApplication&) = delete;

public: // From Framework::Observer
  /**
   * Called when the framework is Component Application Created.
   */
  Any OnCreate() override;

public:
  CreateSignal mCreateSignal;
};

inline ComponentApplication& GetImplementation(Dali::ComponentApplication& application)
{
  DALI_ASSERT_ALWAYS(application && "application handle is empty");

  BaseObject& handle = application.GetBaseObject();

  return static_cast<Internal::Adaptor::ComponentApplication&>(handle);
}

inline const ComponentApplication& GetImplementation(const Dali::ComponentApplication& application)
{
  DALI_ASSERT_ALWAYS(application && "application handle is empty");

  const BaseObject& handle = application.GetBaseObject();

  return static_cast<const Internal::Adaptor::ComponentApplication&>(handle);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
#endif
