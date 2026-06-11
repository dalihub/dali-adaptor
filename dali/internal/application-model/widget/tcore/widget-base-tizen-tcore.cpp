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
#include <dali/internal/application-model/widget/tcore/widget-base-tizen-tcore.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/events/key-event-devel.h>
#include <dali/integration-api/string-utils.h>
#include <dali/public-api/adaptor-framework/widget-impl.h>
#include <dali/public-api/adaptor-framework/widget.h>
#include <dlog.h>
#include <tizen.h>
#include <widget_base.hh>
#include <bundle_cpp.h>

// INTERNAL INCLUDES
#include <dali/internal/application-model/widget/tcore/widget-binding-bridge-tcore.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/tizen/widget-application-impl-tizen.h>
#include <dali/internal/system/tizen/widget-controller-tizen.h>

using Dali::Integration::ToDaliString;

#define DEBUG_PRINTF(fmt, arg...) LOGD(" " fmt, ##arg)

namespace Dali
{
namespace Internal
{
namespace
{

class WidgetContext : public tizen_cpp::WidgetContext
{
 public:
  class Factory : public tizen_cpp::AppCoreMultiWindowBase::Context::IFactory
  {
   public:
    Factory(std::string widgetId, Dali::Internal::Adaptor::WidgetApplicationTizen* application)
    : mWidgetId(std::move(widgetId)),
      mApplication(application)
    {
    }

    std::unique_ptr<Context> Create(std::string instanceId,
                                    tizen_cpp::AppCoreMultiWindowBase* app) override
    {
      print_log(DLOG_INFO, "DALI", "WidgetContext::Factory::Create called (instanceId:%s, app:%p)\n", instanceId.c_str(), app);
      return std::unique_ptr<Context>(new WidgetContext(mWidgetId, instanceId, app, mApplication));
    }

   private:
    std::string                                      mWidgetId;
    Dali::Internal::Adaptor::WidgetApplicationTizen* mApplication;
  };

  WidgetContext(std::string contextId, std::string instanceId, tizen_cpp::AppCoreMultiWindowBase* app, Dali::Internal::Adaptor::WidgetApplicationTizen* application)
     : tizen_cpp::WidgetContext(contextId, instanceId, app),
       mApplication(application)
  {
    print_log(DLOG_INFO, "DALI", "WidgetContext::WidgetContext called (contextId:%s, instanceId:%s, app:%p, application:%p)\n", contextId.c_str(), instanceId.c_str(), app, application);
  }

  bool EnsureApplication(const char* functionName)
  {
    print_log(DLOG_INFO, "DALI", "WidgetContext::EnsureApplication called (function:%s, application:%p)\n", functionName, mApplication);
    if(!mApplication)
    {
      print_log(DLOG_ERROR, "DALI", "WidgetContext::%s: mApplication is null", functionName);
      return false;
    }

    return true;
  }

  bool OnCreate(const tizen_base::Bundle& contents, int w, int h) override
  {
    print_log(DLOG_INFO, "DALI", "WidgetContext::OnCreate called (w:%d, h:%d)\n", w, h);
    if(!EnsureApplication(__FUNCTION__))
    {
      print_log(DLOG_ERROR, "DALI", "WidgetContext::OnCreate: EnsureApplication failed");
      return false;
    }

    std::string id = GetInstId();

    Dali::Window window;
    if(mApplication->GetWidgetCount() == 0)
    {
      window = mApplication->GetWidgetWindow();
      print_log(DLOG_INFO, "DALI", "Widget Instance use default Window(win:%p), so it need to bind widget (%dx%d) (id:%s) \n", &window, w, h, std::string(id).c_str());
    }
    else
    {
      window = Dali::Window::New(PositionSize(0, 0, w, h), "", false);
      if(window)
      {
        print_log(DLOG_INFO, "DALI", "Widget Instance create new Window  (win:%p, cnt:%d) (%dx%d) (id:%s )\n", &window, mApplication->GetWidgetCount(), w, h, std::string(id).c_str());
      }
      else
      {
        print_log(DLOG_ERROR, "DALI", "This device can't support Multi Widget. it means UI may not be properly drawn.");
        window = mApplication->GetWidgetWindow();
      }
    }

    print_log(DLOG_INFO, "DALI", "Widget Instance set window class before bind (id:%s)\n", id.c_str());
    window.SetClass(ToDaliString(id), ToDaliString(id));
    window.AddAuxiliaryHint("wm.policy.win.user.geometry", "1");

    auto bindingResult = Dali::Internal::Adaptor::WidgetBindingBridge::Instance().BindInstanceWindow(this, id.c_str(), window);
    if(!bindingResult.ok)
    {
      print_log(DLOG_ERROR, "DALI", "Widget Instance bind failed(id:%s): %s", std::string(id).c_str(), bindingResult.reason.c_str());
    }
    else
    {
      print_log(DLOG_INFO, "DALI", "Widget Instance bind success(id:%s)\n", std::string(id).c_str());
    }
    window.SetSize(Dali::Window::WindowSize(w, h));

    Dali::Internal::Adaptor::WidgetApplication::CreateWidgetFunctionPair pair           = mApplication->GetWidgetCreatingFunctionPair(std::string(id));
    Dali::WidgetApplication::CreateWidgetFunction                        createFunction = pair.second;

    Dali::Widget widgetInstance = createFunction(ToDaliString(pair.first));

    mApplication->InitializeWidget(this, widgetInstance);

    mApplication->AddWidget(this, widgetInstance, window, std::string(id));

    String encodedContentString = "";

    if(contents.GetCount())
    {
      encodedContentString = (const char*)const_cast<tizen_base::Bundle&>(contents).ToRaw().first.get();
    }

    Internal::Adaptor::GetImplementation(widgetInstance).OnCreate(encodedContentString, window);

    // connect keyEvent for widget
  #ifdef OVER_TIZEN_VERSION_7
    mApplication->ConnectKeyEvent(window);
  #endif

    return true;
  }

  void OnDestroy(DestroyType reason,
      const tizen_base::Bundle& contents) override
  {
    print_log(DLOG_INFO, "DALI", "WidgetContext::OnDestroy called (reason:%d)\n", static_cast<int>(reason));
    if(!EnsureApplication(__FUNCTION__))
    {
      print_log(DLOG_ERROR, "DALI", "WidgetContext::OnDestroy: EnsureApplication failed");
      return;
    }

    // Get Dali::Widget instance.
    Dali::Widget widgetInstance = mApplication->GetWidget(this);

    Dali::Widget::Termination destroyReason = Dali::Widget::Termination::TEMPORARY;

    if(reason == DestroyType::PERMANENT)
    {
      destroyReason = Dali::Widget::Termination::PERMANENT;
    }

    String encodedContentString = "";

    if(contents.GetCount())
    {
      encodedContentString = (const char*)const_cast<tizen_base::Bundle&>(contents).ToRaw().first.get();
    }

    Internal::Adaptor::GetImplementation(widgetInstance).OnTerminate(encodedContentString, destroyReason);

    mApplication->DeleteWidget(this);
  }

  void OnPause() override
  {
    print_log(DLOG_INFO, "DALI", "WidgetContext::OnPause called\n");
    if(!EnsureApplication(__FUNCTION__))
    {
      print_log(DLOG_ERROR, "DALI", "WidgetContext::OnPause: EnsureApplication failed");
      return;
    }

    tizen_cpp::WidgetContext::OnPause();
    // Get Dali::Widget instance.
    Dali::Widget widgetInstance = mApplication->GetWidget(this);
    Internal::Adaptor::GetImplementation(widgetInstance).OnPause();
  }

  void OnResume() override
  {
    print_log(DLOG_INFO, "DALI", "WidgetContext::OnResume called\n");
    if(!EnsureApplication(__FUNCTION__))
    {
      print_log(DLOG_ERROR, "DALI", "WidgetContext::OnResume: EnsureApplication failed");
      return;
    }

    tizen_cpp::WidgetContext::OnResume();

    // Get Dali::Widget instance.
    Dali::Widget widgetInstance = mApplication->GetWidget(this);
    Internal::Adaptor::GetImplementation(widgetInstance).OnResume();
  }

  void OnResize(int w, int h) override
  {
    print_log(DLOG_INFO, "DALI", "WidgetContext::OnResize called (w:%d, h:%d)\n", w, h);
    if(!EnsureApplication(__FUNCTION__))
    {
      print_log(DLOG_ERROR, "DALI", "WidgetContext::OnResize: EnsureApplication failed");
      return;
    }

    tizen_cpp::WidgetContext::OnResize(w, h);

    // Get Dali::Widget instance.
    Dali::Widget widgetInstance = mApplication->GetWidget(this);
    Dali::Window window         = mApplication->GetWindowFromWidget(widgetInstance);
    window.SetSize(Dali::Window::WindowSize(w, h));
    Internal::Adaptor::GetImplementation(widgetInstance).OnResize(window);
  }

  void OnUpdate(const tizen_base::Bundle& contents, bool force) override
  {
    print_log(DLOG_INFO, "DALI", "WidgetContext::OnUpdate called (force:%d)\n", force ? 1 : 0);
    if(!EnsureApplication(__FUNCTION__))
    {
      print_log(DLOG_ERROR, "DALI", "WidgetContext::OnUpdate: EnsureApplication failed");
      return;
    }

    tizen_cpp::WidgetContext::OnUpdate(contents, force);

    // Get Dali::Widget instance.
    Dali::Widget widgetInstance = mApplication->GetWidget(this);

    String encodedContentString = "";

    if(contents.GetCount())
    {
      encodedContentString = (const char*)const_cast<tizen_base::Bundle&>(contents).ToRaw().first.get();
    }
    Internal::Adaptor::GetImplementation(widgetInstance).OnUpdate(encodedContentString, force);
  }

 private:
  Dali::Internal::Adaptor::WidgetApplicationTizen* mApplication = nullptr;
};

} // namespace
namespace Adaptor
{

extern tizen_cpp::WidgetBase* gWidgetBase;

extern "C" DALI_ADAPTOR_API void RegisterWidgetCallback(const char* widgetName, void* data)
{
  print_log(DLOG_INFO, "DALI", "RegisterWidgetCallback called (widgetName:%s, data:%p)\n", widgetName ? widgetName : "null", data);
  if(data == nullptr)
  {
    print_log(DLOG_ERROR, "DALI", "RegisterWidgetCallback: data is null");
  }

  auto* application = static_cast<Dali::Internal::Adaptor::WidgetApplicationTizen*>(data);
  auto factory = std::shared_ptr<tizen_cpp::AppCoreMultiWindowBase::Context::IFactory>(
      new (std::nothrow) WidgetContext::Factory(widgetName, application));
  gWidgetBase->AddContextFactory(std::move(factory), widgetName);
}

extern "C" DALI_ADAPTOR_API void SetContentInfo(void* handle, bundle* bundleData)
{
  print_log(DLOG_INFO, "DALI", "SetContentInfo called (handle:%p, bundleData:%p)\n", handle, bundleData);
  auto* handleInstance = static_cast<WidgetContext*>(handle);
  tizen_base::Bundle b {bundleData, true, true};
  handleInstance->SetContents(b);
}
} // namespace Adaptor
} // namespace Internal
} // namespace Dali
