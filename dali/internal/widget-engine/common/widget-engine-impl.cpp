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
 */

// CLASS HEADER
#include <dali/internal/widget-engine/common/widget-engine-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dlfcn.h>
#include <memory>
#include <utility>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/lifecycle-observer.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr const char* WIDGET_ENGINE_PLUGIN_SO = DALI_LIBRARY_PREFIX "widget-engine-plugin.so";
// Tests override the plugin path to load a test backend or exercise load failures.
constexpr const char* WIDGET_ENGINE_PLUGIN_ENV = "DALI_WIDGET_ENGINE_PLUGIN";

const std::string EMPTY_STRING;

// Owns the plugin library and instances on the event thread. Released instances
// remain alive until deferred cleanup finishes; the library stays loaded until
// process teardown so queued callbacks can still execute plugin code.
class WidgetEnginePluginObject : public LifeCycleObserver
{
public:
  static WidgetEnginePluginObject& GetInstance()
  {
    static WidgetEnginePluginObject instance;
    return instance;
  }

  Dali::WidgetEnginePlugin* Create(const std::string& applicationId)
  {
    if(mStopping)
    {
      return nullptr;
    }
    ObserveAdaptor();
    CollectReleasedPlugins();
    if(!mPlugin)
    {
      const char* path = Dali::EnvironmentVariable::GetEnvironmentVariable(WIDGET_ENGINE_PLUGIN_ENV);
      if(!path || !*path)
      {
        path = WIDGET_ENGINE_PLUGIN_SO;
      }

      auto widgetPlugin = std::make_unique<WidgetPlugin>(path);
      if(!widgetPlugin->create || !widgetPlugin->destroy)
      {
        return nullptr;
      }
      mPlugin = std::move(widgetPlugin);
    }

    auto& widgetPlugin = *mPlugin;
    auto* plugin       = widgetPlugin.create();
    if(!plugin)
    {
      DALI_LOG_ERROR("WidgetEngine: failed to create plugin object.\n");
      return nullptr;
    }

    mPlugins.emplace_back(new Plugin(widgetPlugin, plugin, mAdaptor));
    plugin->SetDestroyReadyCallback(ScheduleCollection);
    if(!plugin->Initialize(applicationId))
    {
      DALI_LOG_ERROR("WidgetEngine: plugin initialization failed for %s\n", applicationId.c_str());
      Release(plugin);
      return nullptr;
    }
    return plugin;
  }

  void Release(Dali::WidgetEnginePlugin* plugin)
  {
    for(auto& entry : mPlugins)
    {
      if(entry->plugin == plugin)
      {
        entry->released = true;
        break;
      }
    }
    CollectReleasedPlugins();
  }

  void OnStart() override
  {
  }
  void OnPause() override
  {
  }
  void OnResume() override
  {
  }

  void OnStop() override
  {
    if(mStopping)
    {
      return;
    }
    mStopping = true;
    VisitAdaptorPlugins(false);
  }

  void OnDestroy() override
  {
    // Detach resources belonging to this adaptor. Clear the association so a
    // later adaptor cannot notify surviving handles. The adaptor is iterating
    // its observer list, so leave observer removal to its teardown.
    mStopping = true;
    VisitAdaptorPlugins(true);
    mAdaptor  = nullptr;
    mStopping = false;
  }

private:
  void ObserveAdaptor()
  {
    if(!mAdaptor && Dali::Adaptor::IsAvailable())
    {
      mAdaptor = &Adaptor::GetImplementation(Dali::Adaptor::Get());
      mAdaptor->AddObserver(*this);
    }
  }

  void VisitAdaptorPlugins(bool destroying)
  {
    mReleasingResources = true;
    for(auto& entry : mPlugins)
    {
      if(entry->adaptor == mAdaptor)
      {
        entry->plugin->Stop();
        if(destroying)
        {
          entry->plugin->DetachFromAdaptor();
          entry->adaptor = nullptr;
        }
      }
    }
    mReleasingResources = false;
    CollectReleasedPlugins();
  }

private:
  struct WidgetPlugin
  {
    using CreateFunction  = Dali::WidgetEnginePlugin* (*)();
    using DestroyFunction = void (*)(Dali::WidgetEnginePlugin*);

    explicit WidgetPlugin(const char* path)
    : handle(dlopen(path, RTLD_LAZY))
    {
      if(!handle)
      {
        DALI_LOG_ERROR("WidgetEngine: failed to load %s: %s\n", path, dlerror());
        return;
      }
      create  = reinterpret_cast<CreateFunction>(dlsym(handle, "CreateWidgetEnginePlugin"));
      destroy = reinterpret_cast<DestroyFunction>(dlsym(handle, "DestroyWidgetEnginePlugin"));
      if(!create || !destroy)
      {
        DALI_LOG_ERROR("WidgetEngine: required plugin symbols are missing: %s\n", dlerror());
      }
    }

    ~WidgetPlugin()
    {
      if(handle)
      {
        dlclose(handle);
      }
    }

    WidgetPlugin(const WidgetPlugin&)            = delete;
    WidgetPlugin& operator=(const WidgetPlugin&) = delete;

    void*           handle{nullptr};
    CreateFunction  create{nullptr};
    DestroyFunction destroy{nullptr};
  };

  struct Plugin
  {
    Plugin(WidgetPlugin& widgetPlugin, Dali::WidgetEnginePlugin* plugin, Adaptor* adaptor)
    : widgetPlugin(widgetPlugin),
      plugin(plugin),
      adaptor(adaptor)
    {
    }

    ~Plugin()
    {
      plugin->SetDestroyReadyCallback({});
      widgetPlugin.destroy(plugin);
    }

    WidgetPlugin&             widgetPlugin;
    Dali::WidgetEnginePlugin* plugin;
    Adaptor*                  adaptor;
    bool                      released{false};
  };

  WidgetEnginePluginObject() = default;
  ~WidgetEnginePluginObject()
  {
    if(mAdaptor)
    {
      mAdaptor->RemoveObserver(*this);
    }
    mReleasingResources = true;
    // Remove membership before plugin destruction, including static teardown.
    while(!mPlugins.empty())
    {
      auto plugin = std::move(mPlugins.back());
      mPlugins.pop_back();
      plugin.reset();
    }
    // The library is destroyed afterward, never before plugin objects.
  }

  WidgetEnginePluginObject(const WidgetEnginePluginObject&)            = delete;
  WidgetEnginePluginObject& operator=(const WidgetEnginePluginObject&) = delete;

  static void Collect()
  {
    GetInstance().CollectReleasedPlugins();
  }

  static void ScheduleCollection()
  {
    // Destroy ready plugins after their completion callback has returned.
    if(Dali::Adaptor::IsAvailable())
    {
      // AddIdle takes ownership even if registration fails.
      Dali::Adaptor::Get().AddIdle(Dali::MakeCallback(&Collect), false);
    }
  }

  void CollectReleasedPlugins()
  {
    if(mReleasingResources)
    {
      return;
    }
    for(size_t index = 0; index < mPlugins.size();)
    {
      if(mPlugins[index]->released && mPlugins[index]->plugin->IsDestroyReady())
      {
        auto plugin = std::move(mPlugins[index]);
        mPlugins.erase(mPlugins.begin() + index);
        plugin.reset();
        index = 0;
      }
      else
      {
        ++index;
      }
    }
  }

  // Select the library on the first successful load. Declaration order keeps
  // it alive until all plugin instances have been destroyed.
  std::unique_ptr<WidgetPlugin>        mPlugin;
  std::vector<std::unique_ptr<Plugin>> mPlugins;
  Adaptor*                             mAdaptor{nullptr};
  bool                                 mStopping{false};
  bool                                 mReleasingResources{false};
};
} //namespace

WidgetEnginePtr WidgetEngine::New(const std::string& applicationId)
{
  WidgetEnginePtr engine = new WidgetEngine();
  if(!engine->Initialize(applicationId))
  {
    return nullptr;
  }
  return engine;
}

WidgetEngine::WidgetEngine()
: mPlugin(nullptr)
{
}

WidgetEngine::~WidgetEngine()
{
  if(mPlugin)
  {
    WidgetEnginePluginObject::GetInstance().Release(mPlugin);
  }
}

bool WidgetEngine::Initialize(const std::string& applicationId)
{
  mApplicationId = applicationId;
  mPlugin        = WidgetEnginePluginObject::GetInstance().Create(applicationId);
  return mPlugin != nullptr;
}

WidgetEngineInstancePtr WidgetEngine::CreateWidget(const std::string& widgetId,
                                                   const std::string& contentInfo,
                                                   int                width,
                                                   int                height,
                                                   float              updatePeriod)
{
  if(!mPlugin)
  {
    return nullptr;
  }

  Dali::WidgetEngineInstancePlugin* instance = mPlugin->CreateWidget(widgetId, contentInfo, width, height, updatePeriod);
  if(!instance)
  {
    return nullptr;
  }

  return WidgetEngineInstance::New(this, instance);
}

bool WidgetEngine::IsAvailable() const
{
  return mPlugin != nullptr;
}

void WidgetEngine::DestroyWidget(Dali::WidgetEngineInstancePlugin* instance)
{
  if(mPlugin && instance)
  {
    mPlugin->DestroyWidget(instance);
  }
}

WidgetEngineInstancePtr WidgetEngineInstance::New(WidgetEnginePtr engine, Dali::WidgetEngineInstancePlugin* plugin)
{
  return new WidgetEngineInstance(std::move(engine), plugin);
}

WidgetEngineInstance::WidgetEngineInstance(WidgetEnginePtr engine, Dali::WidgetEngineInstancePlugin* plugin)
: mEngine(std::move(engine)),
  mPlugin(plugin)
{
}

WidgetEngineInstance::~WidgetEngineInstance()
{
  if(mPlugin)
  {
    mPlugin->ResetCallbacks();
    mEngine->DestroyWidget(mPlugin);
    mPlugin = nullptr;
  }
}

const std::string& WidgetEngineInstance::GetWidgetId() const
{
  return mPlugin ? mPlugin->GetWidgetId() : EMPTY_STRING;
}
const std::string& WidgetEngineInstance::GetInstanceId() const
{
  return mPlugin ? mPlugin->GetInstanceId() : EMPTY_STRING;
}
const std::string& WidgetEngineInstance::GetContentInfo() const
{
  return mPlugin ? mPlugin->GetContentInfo() : EMPTY_STRING;
}
const std::string& WidgetEngineInstance::GetTitle() const
{
  return mPlugin ? mPlugin->GetTitle() : EMPTY_STRING;
}
std::string WidgetEngineInstance::GetPreviewImagePath()
{
  return mPlugin ? mPlugin->GetPreviewImagePath() : std::string();
}
int WidgetEngineInstance::GetProcessId() const
{
  return mPlugin ? mPlugin->GetProcessId() : -1;
}
float WidgetEngineInstance::GetUpdatePeriod() const
{
  return mPlugin ? mPlugin->GetUpdatePeriod() : 0.0f;
}
bool WidgetEngineInstance::IsFaulted() const
{
  return mPlugin && mPlugin->IsFaulted();
}
bool WidgetEngineInstance::Pause()
{
  return mPlugin && mPlugin->Pause();
}
bool WidgetEngineInstance::Resume()
{
  return mPlugin && mPlugin->Resume();
}
bool WidgetEngineInstance::CancelTouchEvent()
{
  return mPlugin && mPlugin->CancelTouchEvent();
}
void WidgetEngineInstance::ActivateFaultedWidget()
{
  if(mPlugin) mPlugin->ActivateFaultedWidget();
}
void WidgetEngineInstance::Resize(int width, int height)
{
  if(mPlugin) mPlugin->Resize(width, height);
}
void WidgetEngineInstance::SetVisible(bool visible)
{
  if(mPlugin) mPlugin->SetVisible(visible);
}
bool WidgetEngineInstance::FeedTouchEvent(const Dali::TouchEvent& event)
{
  return mPlugin && mPlugin->FeedTouchEvent(event);
}
bool WidgetEngineInstance::FeedKeyEvent(const Dali::KeyEvent& event)
{
  return mPlugin && mPlugin->FeedKeyEvent(event);
}
bool WidgetEngineInstance::FeedWheelEvent(const Dali::WheelEvent& event)
{
  return mPlugin && mPlugin->FeedWheelEvent(event);
}
void WidgetEngineInstance::SetEventCallback(Dali::WidgetEngineInstancePlugin::EventCallback callback)
{
  if(mPlugin) mPlugin->SetEventCallback(std::move(callback));
}
void WidgetEngineInstance::SetFrameUpdatedCallback(Dali::WidgetEngineInstancePlugin::FrameUpdatedCallback callback)
{
  if(mPlugin) mPlugin->SetFrameUpdatedCallback(std::move(callback));
}
void WidgetEngineInstance::ResetCallbacks()
{
  if(mPlugin) mPlugin->ResetCallbacks();
}

} // namespace Adaptor
} // namespace Internal
} // namespace DALI_NAMESPACE
