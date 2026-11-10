#ifndef DALI_INTERNAL_WIDGET_ENGINE_IMPL_H
#define DALI_INTERNAL_WIDGET_ENGINE_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/widget-engine/widget-engine.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
class WidgetEngine;
using WidgetEnginePtr = IntrusivePtr<WidgetEngine>;

class WidgetEngineInstance : public BaseObject
{
public:
  static IntrusivePtr<WidgetEngineInstance> New(WidgetEnginePtr engine, Dali::WidgetEngineInstancePlugin* plugin);

  const std::string& GetWidgetId() const;
  const std::string& GetInstanceId() const;
  const std::string& GetContentInfo() const;
  const std::string& GetTitle() const;
  std::string        GetPreviewImagePath();
  int                GetProcessId() const;
  float              GetUpdatePeriod() const;

  bool IsFaulted() const;

  bool Pause();
  bool Resume();
  bool CancelTouchEvent();
  void ActivateFaultedWidget();
  void Resize(int width, int height);
  void SetVisible(bool visible);

  bool FeedTouchEvent(const Dali::TouchEvent& event);
  bool FeedKeyEvent(const Dali::KeyEvent& event);
  bool FeedWheelEvent(const Dali::WheelEvent& event);
  void SetEventCallback(Dali::WidgetEngineInstancePlugin::EventCallback callback);
  void SetFrameUpdatedCallback(Dali::WidgetEngineInstancePlugin::FrameUpdatedCallback callback);
  void ResetCallbacks();

private:
  WidgetEngineInstance(WidgetEnginePtr engine, Dali::WidgetEngineInstancePlugin* plugin);
  ~WidgetEngineInstance() override;

  WidgetEngineInstance(const WidgetEngineInstance&)            = delete;
  WidgetEngineInstance& operator=(const WidgetEngineInstance&) = delete;

private:
  WidgetEnginePtr                   mEngine;
  Dali::WidgetEngineInstancePlugin* mPlugin;
};

using WidgetEngineInstancePtr = IntrusivePtr<WidgetEngineInstance>;

class WidgetEngine : public BaseObject
{
public:
  static WidgetEnginePtr New(const std::string& applicationId);

  WidgetEngineInstancePtr CreateWidget(const std::string& widgetId,
                                       const std::string& contentInfo,
                                       int                width,
                                       int                height,
                                       float              updatePeriod);

  bool               IsAvailable() const;
  const std::string& GetApplicationId() const
  {
    return mApplicationId;
  }
  void DestroyWidget(Dali::WidgetEngineInstancePlugin* instance);

private:
  WidgetEngine();
  ~WidgetEngine() override;

  bool Initialize(const std::string& applicationId);

  WidgetEngine(const WidgetEngine&)            = delete;
  WidgetEngine& operator=(const WidgetEngine&) = delete;

private:
  Dali::WidgetEnginePlugin* mPlugin;
  std::string               mApplicationId;
};

inline WidgetEngine& GetImplementation(Dali::WidgetEngine& engine)
{
  DALI_ASSERT_ALWAYS(engine && "WidgetEngine handle is empty.");
  return static_cast<WidgetEngine&>(engine.GetBaseObject());
}

inline const WidgetEngine& GetImplementation(const Dali::WidgetEngine& engine)
{
  DALI_ASSERT_ALWAYS(engine && "WidgetEngine handle is empty.");
  return static_cast<const WidgetEngine&>(engine.GetBaseObject());
}

inline WidgetEngineInstance& GetImplementation(Dali::WidgetEngineInstance& instance)
{
  DALI_ASSERT_ALWAYS(instance && "WidgetEngineInstance handle is empty.");
  return static_cast<WidgetEngineInstance&>(instance.GetBaseObject());
}

inline const WidgetEngineInstance& GetImplementation(const Dali::WidgetEngineInstance& instance)
{
  DALI_ASSERT_ALWAYS(instance && "WidgetEngineInstance handle is empty.");
  return static_cast<const WidgetEngineInstance&>(instance.GetBaseObject());
}

} // namespace Adaptor
} // namespace Internal
} // namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_WIDGET_ENGINE_IMPL_H
