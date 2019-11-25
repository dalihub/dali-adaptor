#ifndef DALI_INTERNAL_COMPONENT_APPLICATION_H
#define DALI_INTERNAL_COMPONENT_APPLICATION_H

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
#include <dali/internal/adaptor/tizen-wayland/component-application-impl.h>
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

class ComponentApplication : public Application
{
public:
  typedef Dali::ComponentApplication::CreateSignalType CreateSignal;

  static ComponentApplicationPtr New( int* argc, char **argv[], const std::string& stylesheet, WINDOW_MODE windowMode );

  ComponentApplication( int* argc, char **argv[], const std::string& stylesheet, WINDOW_MODE windowMode );

  /**
   * Destructor
   */
  virtual ~ComponentApplication();
  virtual component_class_h OnCreate(void*user_data);

private:

  // @brief Undefined copy constructor.
  ComponentApplication( const ComponentApplication& );

  // @brief Undefined assignment operator.
  ComponentApplication& operator=( const ComponentApplication& );

public:
  CreateSignal                           mCreateSignal;
};

inline ComponentApplication& GetImplementation(Dali::ComponentApplication& application)
{
  DALI_ASSERT_ALWAYS(application && "application handle is empty");

  BaseObject& handle = application.GetBaseObject();

  return static_cast<Internal::Adaptor::ComponentApplication&>(handle);
}

inline const ComponentApplication& GetImplementation(const Dali::ComponentApplication& application)
{
  DALI_ASSERT_ALWAYS(application && "Time handle is empty");

  const BaseObject& handle = application.GetBaseObject();

  return static_cast<const Internal::Adaptor::ComponentApplication&>(handle);
}


} // namespace Adaptor

} // namespace Internal

} // namespace Dali
#endif
#endif // DALI_INTERNAL_COMPONENT_APPLICATION_H
