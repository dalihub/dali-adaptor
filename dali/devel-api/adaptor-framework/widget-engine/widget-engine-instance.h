#ifndef DALI_WIDGET_ENGINE_INSTANCE_H
#define DALI_WIDGET_ENGINE_INSTANCE_H

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
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/widget-engine/widget-engine-plugin.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
class WidgetEngineInstance;
}
} //namespace Internal

/**
 * @brief Handle for controlling one platform widget instance.
 *
 * This interface is for UI framework implementation only. Direct use by
 * applications is unsupported. Applications must use Dali::Ui::WidgetView.
 *
 * Releasing the last handle requests removal of the platform instance.
 * Resetting one of several handles only releases that handle's reference.
 */
class DALI_ADAPTOR_API WidgetEngineInstance : public BaseHandle
{
public:
  WidgetEngineInstance();
  ~WidgetEngineInstance();

  WidgetEngineInstance(const WidgetEngineInstance& instance);
  WidgetEngineInstance& operator=(const WidgetEngineInstance& instance);
  WidgetEngineInstance(WidgetEngineInstance&& instance) noexcept;
  WidgetEngineInstance& operator=(WidgetEngineInstance&& instance) noexcept;

  static WidgetEngineInstance DownCast(BaseHandle handle);

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

  bool FeedTouchEvent(const TouchEvent& event);
  bool FeedKeyEvent(const KeyEvent& event);
  bool FeedWheelEvent(const WheelEvent& event);
  /**
   * @brief Sets the event-thread callback. Pending events may be replayed
   * synchronously when the callback is installed.
   */
  void SetEventCallback(WidgetEngineInstancePlugin::EventCallback callback);
  /**
   * @brief Sets the event-thread frame callback. A pending frame may be
   * delivered synchronously when the callback is installed.
   *
   * The UI implementation must consume retained frames through a Texture, or
   * drop unused images before this callback returns. The Tizen backend releases
   * displayed frames through their graphics resource lifecycle. Retaining an
   * image past this callback and later discarding it without creating a Texture
   * is unsupported and leaves its buffer tracked until shutdown.
   */
  void SetFrameUpdatedCallback(WidgetEngineInstancePlugin::FrameUpdatedCallback callback);
  /**
   * @brief Clears both callbacks and pending events/frames.
   */
  void ResetCallbacks();

public: // Not intended for application developers
  explicit DALI_INTERNAL WidgetEngineInstance(Internal::Adaptor::WidgetEngineInstance* implementation);
};

} // namespace DALI_NAMESPACE

#endif // DALI_WIDGET_ENGINE_INSTANCE_H
