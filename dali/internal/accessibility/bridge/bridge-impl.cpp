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

// CLASS HEADER

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/actors/layer.h>
#include <iostream>
#include <unordered_map>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/internal/accessibility/bridge/bridge-accessible.h>
#include <dali/internal/accessibility/bridge/bridge-action.h>
#include <dali/internal/accessibility/bridge/bridge-collection.h>
#include <dali/internal/accessibility/bridge/bridge-component.h>
#include <dali/internal/accessibility/bridge/bridge-editable-text.h>
#include <dali/internal/accessibility/bridge/bridge-object.h>
#include <dali/internal/accessibility/bridge/bridge-selection.h>
#include <dali/internal/accessibility/bridge/bridge-text.h>
#include <dali/internal/accessibility/bridge/bridge-value.h>
#include <dali/internal/accessibility/bridge/bridge-application.h>
#include <dali/internal/accessibility/bridge/dummy-atspi.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/environment-variables.h>

using namespace Dali::Accessibility;

class BridgeImpl : public virtual BridgeBase,
                   public BridgeAccessible,
                   public BridgeObject,
                   public BridgeComponent,
                   public BridgeCollection,
                   public BridgeAction,
                   public BridgeValue,
                   public BridgeText,
                   public BridgeEditableText,
                   public BridgeSelection,
                   public BridgeApplication
{
  DBus::DBusClient                                              listenOnAtspiEnabledSignalClient;
  DBus::DBusClient                                              registryClient, directReadingClient;
  bool                                                          screenReaderEnabled = false;
  bool                                                          isEnabled           = false;
  bool                                                          isShown             = false;
  std::unordered_map<int32_t, std::function<void(std::string)>> directReadingCallbacks;
  Dali::Actor                                                   highlightedActor;
  std::function<void(Dali::Actor)>                              highlightClearAction;

public:
  BridgeImpl()
  {
    listenOnAtspiEnabledSignalClient = DBus::DBusClient{A11yDbusName, A11yDbusPath, A11yDbusStatusInterface, DBus::ConnectionType::SESSION};

    listenOnAtspiEnabledSignalClient.addPropertyChangedEvent<bool>("ScreenReaderEnabled", [this](bool res) {
      screenReaderEnabled = res;
      if(screenReaderEnabled || isEnabled)
      {
        ForceUp();
      }
      else
      {
        ForceDown();
      }
    });

    listenOnAtspiEnabledSignalClient.addPropertyChangedEvent<bool>("IsEnabled", [this](bool res) {
      isEnabled = res;
      if(screenReaderEnabled || isEnabled)
      {
        ForceUp();
      }
      else
      {
        ForceDown();
      }
    });
  }

  Consumed Emit(KeyEventType type, unsigned int keyCode, const std::string& keyName, unsigned int timeStamp, bool isText) override
  {
    if(!IsUp())
    {
      return Consumed::NO;
    }

    unsigned int keyType = 0;

    switch(type)
    {
      case KeyEventType::KEY_PRESSED:
      {
        keyType = 0;
        break;
      }
      case KeyEventType::KEY_RELEASED:
      {
        keyType = 1;
        break;
      }
      default:
      {
        return Consumed::NO;
      }
    }
    auto m      = registryClient.method<bool(std::tuple<uint32_t, int32_t, int32_t, int32_t, int32_t, std::string, bool>)>("NotifyListenersSync");
    auto result = m.call(std::tuple<uint32_t, int32_t, int32_t, int32_t, int32_t, std::string, bool>{keyType, 0, static_cast<int32_t>(keyCode), 0, static_cast<int32_t>(timeStamp), keyName, isText ? 1 : 0});
    if(!result)
    {
      LOG() << result.getError().message;
      return Consumed::NO;
    }
    return std::get<0>(result) ? Consumed::YES : Consumed::NO;
  }

  void Pause() override
  {
    if(!IsUp())
    {
      return;
    }

    directReadingClient.method<DBus::ValueOrError<void>(bool)>("PauseResume").asyncCall([](DBus::ValueOrError<void> msg) {
      if(!msg)
      {
        LOG() << "Direct reading command failed (" << msg.getError().message << ")";
      }
    },
                                                                                        true);
  }

  void Resume() override
  {
    if(!IsUp())
    {
      return;
    }

    directReadingClient.method<DBus::ValueOrError<void>(bool)>("PauseResume").asyncCall([](DBus::ValueOrError<void> msg) {
      if(!msg)
      {
        LOG() << "Direct reading command failed (" << msg.getError().message << ")";
      }
    },
                                                                                        false);
  }

  void StopReading(bool alsoNonDiscardable) override
  {
    if(!IsUp())
    {
      return;
    }

    directReadingClient.method<DBus::ValueOrError<void>(bool)>("StopReading").asyncCall([](DBus::ValueOrError<void> msg) {
      if(!msg)
      {
        LOG() << "Direct reading command failed (" << msg.getError().message << ")";
      }
    },
                                                                                        alsoNonDiscardable);
  }

  void Say(const std::string& text, bool discardable, std::function<void(std::string)> callback) override
  {
    if(!IsUp())
    {
      return;
    }

    directReadingClient.method<DBus::ValueOrError<std::string, bool, int32_t>(std::string, bool)>("ReadCommand").asyncCall([=](DBus::ValueOrError<std::string, bool, int32_t> msg) {
      if(!msg)
      {
        LOG() << "Direct reading command failed (" << msg.getError().message << ")";
      }
      else if(callback)
      {
        directReadingCallbacks.emplace(std::get<2>(msg), callback);
      }
    },
                                                                                                                           text,
                                                                                                                           discardable);
  }

  void ForceDown() override
  {
    if(mData)
    {
      if(mData->mCurrentlyHighlightedActor && mData->mHighlightActor)
      {
        mData->mCurrentlyHighlightedActor.Remove(mData->mHighlightActor);
      }
      mData->mCurrentlyHighlightedActor = {};
      mData->mHighlightActor            = {};
    }
    highlightedActor     = {};
    highlightClearAction = {};
    BridgeAccessible::ForceDown();
    registryClient      = {};
    directReadingClient = {};
    directReadingCallbacks.clear();
  }

  void Terminate() override
  {
    if(mData)
    {
      mData->mCurrentlyHighlightedActor = {};
      mData->mHighlightActor            = {};
    }
    ForceDown();
    listenOnAtspiEnabledSignalClient = {};
    dbusServer                       = {};
    con                              = {};
  }

  ForceUpResult ForceUp() override
  {
    if(BridgeAccessible::ForceUp() == ForceUpResult::ALREADY_UP)
    {
      return ForceUpResult::ALREADY_UP;
    }

    BridgeObject::RegisterInterfaces();
    BridgeAccessible::RegisterInterfaces();
    BridgeComponent::RegisterInterfaces();
    BridgeCollection::RegisterInterfaces();
    BridgeAction::RegisterInterfaces();
    BridgeValue::RegisterInterfaces();
    BridgeText::RegisterInterfaces();
    BridgeEditableText::RegisterInterfaces();
    BridgeSelection::RegisterInterfaces();
    BridgeApplication::RegisterInterfaces();

    RegisterOnBridge(&application);

    registryClient      = {AtspiDbusNameRegistry, AtspiDbusPathDec, AtspiDbusInterfaceDec, con};
    directReadingClient = DBus::DBusClient{DirectReadingDBusName, DirectReadingDBusPath, DirectReadingDBusInterface, con};
    directReadingClient.addSignal<void(int32_t, std::string)>("ReadingStateChanged", [=](int32_t id, std::string readingState) {
      auto it = directReadingCallbacks.find(id);
      if(it != directReadingCallbacks.end())
      {
        it->second(readingState);
        if(readingState != "ReadingPaused" && readingState != "ReadingResumed" && readingState != "ReadingStarted")
          directReadingCallbacks.erase(it);
      }
    });

    auto    proxy = DBus::DBusClient{AtspiDbusNameRegistry, AtspiDbusPathRoot, AtspiDbusInterfaceSocket, con};
    Address root{"", "root"};
    auto    res = proxy.method<Address(Address)>("Embed").call(root);
    if(!res)
    {
      LOG() << "Call to Embed failed: " << res.getError().message;
    }
    assert(res);
    application.parent.SetAddress(std::move(std::get<0>(res)));
    if(isShown)
    {
      EmitActivate();
    }
    return ForceUpResult::JUST_STARTED;
  }

  void EmitActivate()
  {
    auto win = application.getActiveWindow();
    if(win)
    {
      win->Emit(WindowEvent::ACTIVATE, 0);
    }
  }

  void EmitDeactivate()
  {
    auto win = application.getActiveWindow();
    if(win)
    {
      win->Emit(WindowEvent::DEACTIVATE, 0);
    }
  }

  void WindowHidden() override
  {
    if(isShown && IsUp())
    {
      EmitDeactivate();
    }
    isShown = false;
  }

  void WindowShown() override
  {
    if(!isShown && IsUp())
    {
      EmitActivate();
    }
    isShown = true;
  }

  void Initialize() override
  {
    auto dbusClient = DBus::DBusClient{A11yDbusName, A11yDbusPath, A11yDbusStatusInterface, DBus::ConnectionType::SESSION};
    auto enabled = dbusClient.property<bool>("ScreenReaderEnabled").get();
    if(enabled)
    {
      screenReaderEnabled = std::get<0>(enabled);
    }

    enabled = dbusClient.property<bool>("IsEnabled").get();
    if(enabled)
    {
      isEnabled = std::get<0>(enabled);
    }

    if(screenReaderEnabled || isEnabled)
    {
      ForceUp();
    }
  }

  bool GetScreenReaderEnabled()
  {
    return screenReaderEnabled;
  }

  bool IsEnabled()
  {
    return isEnabled;
  }
};

static bool bridgeInitialized;

static Bridge* CreateBridge()
{
  bridgeInitialized = true;

  try
  {
    /* check environment variable first */
    const char* envAtspiDisabled = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_DISABLE_ATSPI);
    if(envAtspiDisabled && std::atoi(envAtspiDisabled) != 0)
    {
      return Dali::Accessibility::DummyBridge::GetInstance();
    }

    return new BridgeImpl;
  }
  catch(const std::exception&)
  {
    DALI_LOG_ERROR("Failed to initialize AT-SPI bridge");
    return Dali::Accessibility::DummyBridge::GetInstance();
  }
}

Bridge* Bridge::GetCurrentBridge()
{
  static Bridge* bridge;

  if(bridge)
  {
    return bridge;
  }
  else if(autoInitState == AutoInitState::ENABLED)
  {
    bridge = CreateBridge();

    /* check environment variable for suppressing screen-reader */
    const char* envSuppressScreenReader = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_SUPPRESS_SCREEN_READER);
    if(envSuppressScreenReader && std::atoi(envSuppressScreenReader) != 0)
    {
      bridge->SuppressScreenReader(true);
    }

    return bridge;
  }

  return Dali::Accessibility::DummyBridge::GetInstance();
}

void Bridge::DisableAutoInit()
{
  if(bridgeInitialized)
  {
    DALI_LOG_ERROR("Bridge::DisableAutoInit() called after bridge auto-initialization");
  }

  autoInitState = AutoInitState::DISABLED;
}

void Bridge::EnableAutoInit()
{
  autoInitState = AutoInitState::ENABLED;

  if(bridgeInitialized)
  {
    return;
  }

  auto rootLayer       = Dali::Stage::GetCurrent().GetRootLayer();
  auto window          = Dali::DevelWindow::Get(rootLayer);
  auto applicationName = Dali::Internal::Adaptor::Adaptor::GetApplicationPackageName();

  auto* bridge = Bridge::GetCurrentBridge();
  bridge->AddTopLevelWindow(Dali::Accessibility::Accessible::Get(rootLayer, true));
  bridge->SetApplicationName(applicationName);
  bridge->Initialize();

  if(window && window.IsVisible())
  {
    bridge->WindowShown();
  }
}
