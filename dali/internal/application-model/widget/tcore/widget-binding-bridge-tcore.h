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

#ifndef DALI_INTERNAL_APPLICATION_MODEL_TCORE_WIDGET_BINDING_BRIDGE_H
#define DALI_INTERNAL_APPLICATION_MODEL_TCORE_WIDGET_BINDING_BRIDGE_H

// EXTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
#include <string>
#include <widget_base.hh>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

enum class WidgetBackendType
{
  ECORE,
  TCORE
};

struct WidgetBindingResult
{
  bool        ok{false};
  std::string reason;
};

class IWidgetBindingBackend
{
public:
  virtual ~IWidgetBindingBackend() = default;
  virtual WidgetBindingResult Bind(void* instance, const char* id, Dali::Window window) = 0;
};

class WidgetBindingBridge
{
public:
  static WidgetBindingBridge& Instance();

  void SetBackend(WidgetBackendType backendType);

  WidgetBindingResult BindInstanceWindow(tizen_cpp::WidgetContext* instance, const char* id, Dali::Window window);

private:
  WidgetBindingBridge();
  ~WidgetBindingBridge();
  WidgetBindingBridge(const WidgetBindingBridge&)            = delete;
  WidgetBindingBridge& operator=(const WidgetBindingBridge&) = delete;

private:
  class Impl;
  Impl* mImpl;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_APPLICATION_MODEL_TCORE_WIDGET_BINDING_BRIDGE_H

