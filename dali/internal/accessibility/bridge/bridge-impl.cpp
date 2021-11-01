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

/**
 * @brief The BridgeImpl class is to implement some Bridge functions.
 */
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
  DBus::DBusClient                                              mAccessibilityStatusClient;
  DBus::DBusClient                                              mRegistryClient;
  DBus::DBusClient                                              mDirectReadingClient;
  bool                                                          mIsScreenReaderEnabled = false;
  bool                                                          mIsEnabled             = false;
  bool                                                          mIsShown               = false;
  std::unordered_map<int32_t, std::function<void(std::string)>> mDirectReadingCallbacks;
  Dali::Actor                                                   mHighlightedActor;
  std::function<void(Dali::Actor)>                              mHighlightClearAction;
  Dali::CallbackBase*                                           mIdleCallback          = NULL;

public:
  BridgeImpl()
  {
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::Emit()
   */
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

    auto methodObject = mRegistryClient.method<bool(std::tuple<uint32_t, int32_t, int32_t, int32_t, int32_t, std::string, bool>)>("NotifyListenersSync");
    auto result       = methodObject.call(std::tuple<uint32_t, int32_t, int32_t, int32_t, int32_t, std::string, bool>{keyType, 0, static_cast<int32_t>(keyCode), 0, static_cast<int32_t>(timeStamp), keyName, isText ? 1 : 0});
    if(!result)
    {
      LOG() << result.getError().message;
      return Consumed::NO;
    }
    return std::get<0>(result) ? Consumed::YES : Consumed::NO;
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::Pause()
   */
  void Pause() override
  {
    if(!IsUp())
    {
      return;
    }

    mDirectReadingClient.method<DBus::ValueOrError<void>(bool)>("PauseResume").asyncCall([](DBus::ValueOrError<void> msg) {
      if(!msg)
      {
        LOG() << "Direct reading command failed (" << msg.getError().message << ")";
      }
    },
                                                                                        true);
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::Resume()
   */
  void Resume() override
  {
    if(!IsUp())
    {
      return;
    }

    mDirectReadingClient.method<DBus::ValueOrError<void>(bool)>("PauseResume").asyncCall([](DBus::ValueOrError<void> msg) {
      if(!msg)
      {
        LOG() << "Direct reading command failed (" << msg.getError().message << ")";
      }
    },
                                                                                        false);
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::StopReading()
   */
  void StopReading(bool alsoNonDiscardable) override
  {
    if(!IsUp())
    {
      return;
    }

    mDirectReadingClient.method<DBus::ValueOrError<void>(bool)>("StopReading").asyncCall([](DBus::ValueOrError<void> msg) {
      if(!msg)
      {
        LOG() << "Direct reading command failed (" << msg.getError().message << ")";
      }
    },
                                                                                        alsoNonDiscardable);
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::Say()
   */
  void Say(const std::string& text, bool discardable, std::function<void(std::string)> callback) override
  {
    if(!IsUp())
    {
      return;
    }

    mDirectReadingClient.method<DBus::ValueOrError<std::string, bool, int32_t>(std::string, bool)>("ReadCommand").asyncCall([=](DBus::ValueOrError<std::string, bool, int32_t> msg) {
      if(!msg)
      {
        LOG() << "Direct reading command failed (" << msg.getError().message << ")";
      }
      else if(callback)
      {
        mDirectReadingCallbacks.emplace(std::get<2>(msg), callback);
      }
    },
                                                                                                                           text,
                                                                                                                           discardable);
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::ForceDown()
   */
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
    mHighlightedActor     = {};
    mHighlightClearAction = {};
    BridgeAccessible::ForceDown();
    mRegistryClient       = {};
    mDirectReadingClient  = {};
    mDirectReadingCallbacks.clear();
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::Terminate()
   */
  void Terminate() override
  {
    if(mData)
    {
      mData->mCurrentlyHighlightedActor = {};
      mData->mHighlightActor            = {};
    }
    ForceDown();
    if((NULL != mIdleCallback) && Dali::Adaptor::IsAvailable())
    {
      Dali::Adaptor::Get().RemoveIdle(mIdleCallback);
    }
    mAccessibilityStatusClient        = {};
    mDbusServer                       = {};
    mConnectionPtr                    = {};
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::ForceUp()
   */
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

    RegisterOnBridge(&mApplication);

    mRegistryClient      = {AtspiDbusNameRegistry, AtspiDbusPathDec, AtspiDbusInterfaceDec, mConnectionPtr};
    mDirectReadingClient = DBus::DBusClient{DirectReadingDBusName, DirectReadingDBusPath, DirectReadingDBusInterface, mConnectionPtr};

    mDirectReadingClient.addSignal<void(int32_t, std::string)>("ReadingStateChanged", [=](int32_t id, std::string readingState) {
      auto it = mDirectReadingCallbacks.find(id);
      if(it != mDirectReadingCallbacks.end())
      {
        it->second(readingState);
        if(readingState != "ReadingPaused" && readingState != "ReadingResumed" && readingState != "ReadingStarted")
        {
          mDirectReadingCallbacks.erase(it);
        }
      }
    });

    auto    proxy = DBus::DBusClient{AtspiDbusNameRegistry, AtspiDbusPathRoot, AtspiDbusInterfaceSocket, mConnectionPtr};
    Address root{"", "root"};
    auto    res = proxy.method<Address(Address)>("Embed").call(root);
    if(!res)
    {
      LOG() << "Call to Embed failed: " << res.getError().message;
    }
    assert(res);

    mApplication.mParent.SetAddress(std::move(std::get<0>(res)));
    if(mIsShown)
    {
      EmitActivate();
    }
    return ForceUpResult::JUST_STARTED;
  }

  /**
   * @brief Sends a signal to dbus that the default window is activated.
   *
   * TODO : This is subject to change if/when we implement multi-window support.
   * @see BridgeObject::Emit()
   */
  void EmitActivate()
  {
    auto win = mApplication.GetActiveWindow();
    if(win)
    {
      win->Emit(WindowEvent::ACTIVATE, 0);
    }
  }

  /**
   * @brief Sends a signal to dbus that the default window is deactivated.
   *
   * TODO : This is subject to change if/when we implement multi-window support.
   * @see BridgeObject::Emit()
   */
  void EmitDeactivate()
  {
    auto win = mApplication.GetActiveWindow();
    if(win)
    {
      win->Emit(WindowEvent::DEACTIVATE, 0);
    }
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::WindowHidden()
   */
  void WindowHidden() override
  {
    if(mIsShown && IsUp())
    {
      EmitDeactivate();
    }
    mIsShown = false;
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::WindowShown()
   */
  void WindowShown() override
  {
    if(!mIsShown && IsUp())
    {
      EmitActivate();
    }
    mIsShown = true;
  }

  void ReadAndListenProperty()
  {
    // read property
    auto enabled = mAccessibilityStatusClient.property<bool>("ScreenReaderEnabled").get();
    if(enabled)
    {
      mIsScreenReaderEnabled = std::get<0>(enabled);
    }

    enabled = mAccessibilityStatusClient.property<bool>("IsEnabled").get();
    if(enabled)
    {
      mIsEnabled = std::get<0>(enabled);
    }

    if(mIsScreenReaderEnabled || mIsEnabled)
    {
      ForceUp();
    }

    // listen property change
    mAccessibilityStatusClient.addPropertyChangedEvent<bool>("ScreenReaderEnabled", [this](bool res) {
      mIsScreenReaderEnabled = res;
      if(mIsScreenReaderEnabled || mIsEnabled)
      {
        ForceUp();
      }
      else
      {
        ForceDown();
      }
    });

    mAccessibilityStatusClient.addPropertyChangedEvent<bool>("IsEnabled", [this](bool res) {
      mIsEnabled = res;
      if(mIsScreenReaderEnabled || mIsEnabled)
      {
        ForceUp();
      }
      else
      {
        ForceDown();
      }
    });
  }

  bool InitializeAccessibilityStatusClient()
  {
    mAccessibilityStatusClient = DBus::DBusClient{A11yDbusName, A11yDbusPath, A11yDbusStatusInterface, DBus::ConnectionType::SESSION};

    if (!mAccessibilityStatusClient)
    {
      DALI_LOG_ERROR("Accessibility Status DbusClient is not ready\n");
      return false;
    }

    return true;
  }

  bool OnIdleSignal()
  {
    if ( InitializeAccessibilityStatusClient() )
    {
      ReadAndListenProperty();
      mIdleCallback = NULL;
      return false;
    }

    return true;
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::Initialize()
   */
  void Initialize() override
  {
    if ( InitializeAccessibilityStatusClient() )
    {
      ReadAndListenProperty();
      return;
    }

    // Initialize failed. Try it again on Idle
    if( Dali::Adaptor::IsAvailable() )
    {
      Dali::Adaptor& adaptor = Dali::Adaptor::Get();
      if( NULL == mIdleCallback )
      {
        mIdleCallback = MakeCallback( this, &BridgeImpl::OnIdleSignal );
        adaptor.AddIdle( mIdleCallback, true );
      }
    }
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::GetScreenReaderEnabled()
   */
  bool GetScreenReaderEnabled() override
  {
    return mIsScreenReaderEnabled;
  }

  /**
   * @copydoc Dali::Accessibility::Bridge::IsEnabled()
   */
  bool IsEnabled() override
  {
    return mIsEnabled;
  }
}; // BridgeImpl

namespace // unnamed namespace
{

bool INITIALIZED_BRIDGE = false;

/**
 * @brief Creates BridgeImpl instance.
 *
 * @return The BridgeImpl instance
 * @note This method is to check environment variable first. If ATSPI is disable using env, it returns dummy bridge instance.
 */
Bridge* CreateBridge()
{
  INITIALIZED_BRIDGE = true;

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

} // unnamed namespace

// Dali::Accessibility::Bridge class implementation

Bridge* Bridge::GetCurrentBridge()
{
  static Bridge* bridge;

  if(bridge)
  {
    return bridge;
  }
  else if(mAutoInitState == AutoInitState::ENABLED)
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
  if(INITIALIZED_BRIDGE)
  {
    DALI_LOG_ERROR("Bridge::DisableAutoInit() called after bridge auto-initialization");
  }

  mAutoInitState = AutoInitState::DISABLED;
}

void Bridge::EnableAutoInit()
{
  mAutoInitState = AutoInitState::ENABLED;

  if(INITIALIZED_BRIDGE)
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
