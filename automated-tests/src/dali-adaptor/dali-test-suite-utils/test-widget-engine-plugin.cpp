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
#include <utility>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/widget-engine/widget-engine-plugin.h>

namespace
{
int gEngineDestroyedCount   = 0;
int gInstanceDestroyedCount = 0;
int gCallbacksResetCount    = 0;
class TestWidgetEngine;
TestWidgetEngine* gDeferredEngine = nullptr;

class TestWidgetEngineInstance : public Dali::WidgetEngineInstancePlugin
{
public:
  TestWidgetEngineInstance(std::string widgetId, std::string contentInfo, float updatePeriod)
  : mWidgetId(std::move(widgetId)),
    mInstanceId("test-instance"),
    mContentInfo(std::move(contentInfo)),
    mTitle("Test widget"),
    mUpdatePeriod(updatePeriod)
  {
  }

  const std::string& GetWidgetId() const override
  {
    return mWidgetId;
  }

  const std::string& GetInstanceId() const override
  {
    return mInstanceId;
  }

  const std::string& GetContentInfo() const override
  {
    return mContentInfo;
  }

  const std::string& GetTitle() const override
  {
    return mTitle;
  }

  std::string GetPreviewImagePath() override
  {
    return "/tmp/test-widget-preview.png";
  }

  int GetProcessId() const override
  {
    return 1234;
  }

  float GetUpdatePeriod() const override
  {
    return mUpdatePeriod;
  }

  bool IsFaulted() const override
  {
    return false;
  }

  bool Pause() override
  {
    return true;
  }

  bool Resume() override
  {
    return true;
  }

  bool CancelTouchEvent() override
  {
    return true;
  }

  void ActivateFaultedWidget() override
  {
  }

  void Resize(int width, int height) override
  {
  }

  void SetVisible(bool visible) override
  {
  }

  bool FeedTouchEvent(const Dali::TouchEvent& event) override
  {
    return true;
  }

  bool FeedKeyEvent(const Dali::KeyEvent& event) override
  {
    return true;
  }

  bool FeedWheelEvent(const Dali::WheelEvent& event) override
  {
    return true;
  }

  void SetEventCallback(EventCallback callback) override
  {
    mEventCallback = std::move(callback);
    if(mEventCallback)
    {
      mEventCallback(EventType::ADDED);
    }
  }

  void SetFrameUpdatedCallback(FrameUpdatedCallback callback) override
  {
    mFrameUpdatedCallback = std::move(callback);
    if(mFrameUpdatedCallback)
    {
      mFrameUpdatedCallback(Dali::NativeImagePtr());
    }
  }

  void ResetCallbacks() override
  {
    mEventCallback        = nullptr;
    mFrameUpdatedCallback = nullptr;
    ++gCallbacksResetCount;
  }

private:
  std::string          mWidgetId;
  std::string          mInstanceId;
  std::string          mContentInfo;
  std::string          mTitle;
  EventCallback        mEventCallback;
  FrameUpdatedCallback mFrameUpdatedCallback;
  float                mUpdatePeriod;
};

class TestWidgetEngine : public Dali::WidgetEnginePlugin
{
public:
  ~TestWidgetEngine() override
  {
    if(gDeferredEngine == this)
    {
      gDeferredEngine = nullptr;
    }
  }

  bool Initialize(const std::string& applicationId) override
  {
    if(applicationId == "test.widget.viewer.deferred")
    {
      mPending        = true;
      gDeferredEngine = this;
      return true;
    }
    return applicationId == "test.widget.viewer";
  }

  bool IsDestroyReady() const override
  {
    return !mPending;
  }

  void SetDestroyReadyCallback(std::function<void()> callback) override
  {
    mDestroyReadyCallback = std::move(callback);
  }

  void DetachFromAdaptor() override
  {
    if(mPending)
    {
      mPending = false;
      if(mDestroyReadyCallback)
      {
        mDestroyReadyCallback();
      }
    }
  }

  Dali::WidgetEngineInstancePlugin* CreateWidget(const std::string& widgetId,
                                                 const std::string& contentInfo,
                                                 int                width,
                                                 int                height,
                                                 float              updatePeriod) override
  {
    return new TestWidgetEngineInstance(widgetId, contentInfo, updatePeriod);
  }

  void DestroyWidget(Dali::WidgetEngineInstancePlugin* instance) override
  {
    delete instance;
    ++gInstanceDestroyedCount;
  }

private:
  std::function<void()> mDestroyReadyCallback;
  bool                  mPending{false};
};
} // namespace

extern "C" Dali::WidgetEnginePlugin* CreateWidgetEnginePlugin()
{
  return new TestWidgetEngine();
}

extern "C" void DestroyWidgetEnginePlugin(Dali::WidgetEnginePlugin* plugin)
{
  delete plugin;
  ++gEngineDestroyedCount;
}

extern "C" void ResetWidgetEngineTestCounts()
{
  gEngineDestroyedCount   = 0;
  gInstanceDestroyedCount = 0;
  gCallbacksResetCount    = 0;
}

extern "C" int GetWidgetEngineDestroyedCount()
{
  return gEngineDestroyedCount;
}

extern "C" int GetWidgetEngineInstanceDestroyedCount()
{
  return gInstanceDestroyedCount;
}

extern "C" int GetWidgetEngineCallbacksResetCount()
{
  return gCallbacksResetCount;
}

extern "C" void CompleteWidgetEngineTestRelease()
{
  if(gDeferredEngine)
  {
    gDeferredEngine->DetachFromAdaptor();
  }
}
