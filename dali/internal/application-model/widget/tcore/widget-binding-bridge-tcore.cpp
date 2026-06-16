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

// CLASS HEADER
#include <dali/internal/application-model/widget/tcore/widget-binding-bridge-tcore.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dlog.h>
#include <memory>
#include <tizen.h>
#include <tizen_core_wl.h>
#include <widget_base.hh>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{

class TcoreWidgetBindingBackend : public IWidgetBindingBackend
{
public:
  WidgetBindingResult Bind(void* instance, const char* id, Dali::Window window) override
  {
    print_log(DLOG_INFO, "DALI", "TcoreWidgetBindingBackend::Bind\n");
    if(!instance || !id || !window)
    {
      return {false, "invalid argument (instance/id/window)"};
    }

    Dali::Any nativeHandle = window.GetNativeHandle();
    if(!nativeHandle.IsType<tizen_core_wl_window_h>())
    {
      return {false, "native handle type is not tizen_core_wl_window_h"};
    }

    tizen_core_wl_window_h wlWindow = AnyCast<tizen_core_wl_window_h>(nativeHandle);
    if(!wlWindow)
    {
      return {false, "AnyCast<tizen_core_wl_window_h> returned null"};
    }

    // NOTE: legacy API is intentionally isolated in this bridge.
    auto* inst = static_cast<tizen_cpp::WidgetContext*>(instance);
    print_log(DLOG_INFO, "DALI", "TcoreWidgetBindingBackend::Bind, WidgetContext->WindowBind id[%s], wlWindow[%p]\n", id, wlWindow);
    int ret = inst->WindowBind(id, wlWindow);
    if(ret != 0)
    {
      return {false, "widget_base_context_window_bind failed on TCORE backend"};
    }

    return {true, ""};
  }
};

} // unnamed namespace

class WidgetBindingBridge::Impl
{
public:
  void SetBackend(WidgetBackendType backendType)
  {
    print_log(DLOG_INFO, "DALI", "WidgetBindingBridge::Impl::SetBackend\n");
    mBackendType = backendType;
    switch(mBackendType)
    {
      case WidgetBackendType::TCORE:
      default:
      {
        mBackend.reset(new TcoreWidgetBindingBackend());
        break;
      }
    }
  }

  WidgetBindingResult BindInstanceWindow(tizen_cpp::WidgetContext* instance, const char* id, Dali::Window window)
  {
    print_log(DLOG_INFO, "DALI", "WidgetBindingBridge::Impl::BindInstanceWindow\n");
    if(!mBackend)
    {
      return {false, "binding backend is not initialized"};
    }
    return mBackend->Bind(instance, id, window);
  }

private:
  WidgetBackendType                        mBackendType{WidgetBackendType::TCORE};
  std::unique_ptr<IWidgetBindingBackend> mBackend;
};

WidgetBindingBridge& WidgetBindingBridge::Instance()
{
  static WidgetBindingBridge bridge;
  print_log(DLOG_INFO, "DALI", "WidgetBindingBridge Instance\n");
  return bridge;
}

WidgetBindingBridge::WidgetBindingBridge()
: mImpl(new Impl())
{
  print_log(DLOG_INFO, "DALI", "WidgetBindingBridge Constructor\n");
  mImpl->SetBackend(WidgetBackendType::TCORE);
}

WidgetBindingBridge::~WidgetBindingBridge()
{
  delete mImpl;
  mImpl = nullptr;
}

void WidgetBindingBridge::SetBackend(WidgetBackendType backendType)
{
  print_log(DLOG_INFO, "DALI", "WidgetBindingBridge::SetBackend\n");
  mImpl->SetBackend(backendType);
}

WidgetBindingResult WidgetBindingBridge::BindInstanceWindow(tizen_cpp::WidgetContext* instance, const char* id, Dali::Window window)
{
  print_log(DLOG_INFO, "DALI", "WidgetBindingBridge::BindInstanceWindow, id[%s], window[%p]\n", id, &window);
  WidgetBindingResult result = mImpl->BindInstanceWindow(instance, id, window);
  if(!result.ok)
  {
    print_log(DLOG_ERROR, "DALI", "WidgetBindingBridge::BindInstanceWindow failed: %s\n", result.reason.c_str());
  }
  return result;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

