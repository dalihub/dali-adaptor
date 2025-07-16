/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/hover-event-integ.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/touch-integ.h>
#include <dali/integration-api/events/wheel-event-integ.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/render-tasks/render-task-list.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/lifecycle-observer.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/input/common/physical-keyboard-impl.h>
#include <dali/internal/system/common/time-service.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gSceneHolderLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_SCENE_HOLDER");
#endif

const uint32_t MAX_PRESSED_POINT_COUNT = 2;
} // unnamed namespace

uint32_t SceneHolder::mSceneHolderCounter = 0;

class SceneHolder::SceneHolderLifeCycleObserver : public LifeCycleObserver
{
public:
  SceneHolderLifeCycleObserver(Adaptor*& adaptor, bool& adaptorStarted)
  : mAdaptor(adaptor),
    mAdaptorStarted(adaptorStarted)
  {
  }

private: // Adaptor::LifeCycleObserver interface
  void OnStart() override
  {
    mAdaptorStarted = true;
  };
  void OnPause() override {};
  void OnResume() override {};
  void OnStop() override
  {
    // Mark adaptor as stopped;
    mAdaptorStarted = false;
  };
  void OnDestroy() override
  {
    mAdaptor = nullptr;
  };

private:
  Adaptor*& mAdaptor;
  bool&     mAdaptorStarted;
};

SceneHolder::SceneHolder()
: mLifeCycleObserver(new SceneHolderLifeCycleObserver(mAdaptor, mAdaptorStarted)),
  mLastTouchEvent(),
  mLastHoverEvent(),
  mId(mSceneHolderCounter++),
  mSurface(nullptr),
  mAdaptor(nullptr),
  mDpi(),
  mAdaptorStarted(false),
  mVisible(true),
  mHandledMultiTouch(false),
  mPreviousTouchEvent(),
  mPreviousHoverEvent(),
  mPreviousType(Integration::TouchEventCombiner::DISPATCH_NONE)
{
}

SceneHolder::~SceneHolder()
{
  if(mScene)
  {
    // The scene graph object should be removed first.
    mScene.RemoveSceneObject();
  }

  if(mAdaptor)
  {
    mAdaptor->RemoveObserver(*mLifeCycleObserver.get());
    mAdaptor->RemoveWindow(this);

    if(mAdaptorStarted)
    {
      // The event queue is flushed and we wait for the completion of the surface removal
      // Note : we don't need to delete surface when adaptor is stopped now.
      mAdaptor->DeleteSurface(*mSurface.get());
    }

    mAdaptor = nullptr;
  }

  if(mScene)
  {
    // We should remove the surface from the Core last
    mScene.Discard();
  }
}

void SceneHolder::Add(Dali::Actor actor)
{
  if(mScene)
  {
    mScene.Add(actor);
  }
}

void SceneHolder::Remove(Dali::Actor actor)
{
  if(mScene)
  {
    mScene.Remove(actor);
  }
}

Dali::Layer SceneHolder::GetRootLayer() const
{
  return mScene ? mScene.GetRootLayer() : Dali::Layer();
}

Dali::Layer SceneHolder::GetOverlayLayer()
{
  return mScene ? mScene.GetOverlayLayer() : Dali::Layer();
}

uint32_t SceneHolder::GetId() const
{
  return mId;
}

std::string SceneHolder::GetName() const
{
  return mName;
}

bool SceneHolder::IsVisible() const
{
  return mVisible;
}

Dali::Integration::Scene SceneHolder::GetScene()
{
  return mScene;
}

Uint16Pair SceneHolder::GetDpi() const
{
  return mDpi;
}

void SceneHolder::SetSurface(Dali::Integration::RenderSurfaceInterface* surface)
{
  mSurface.reset(surface);

  mScene.SurfaceReplaced();

  PositionSize surfacePositionSize = surface->GetPositionSize();

  SurfaceResized(static_cast<float>(surfacePositionSize.width), static_cast<float>(surfacePositionSize.height));

  InitializeDpi();

  mSurface->SetAdaptor(*mAdaptor);
  mSurface->SetScene(mScene);

  // Recreate the render target
  CreateRenderTarget();

  OnSurfaceSet(surface);
}

void SceneHolder::SurfaceResized(float width, float height)
{
  mScene.SurfaceResized(width, height);

  mSurface->SetFullSwapNextFrame();

  // Recreate the render target
  CreateRenderTarget();
}

Dali::Integration::RenderSurfaceInterface* SceneHolder::GetSurface() const
{
  return mSurface.get();
}

void SceneHolder::SetBackgroundColor(const Vector4& color)
{
  if(mScene)
  {
    mScene.SetBackgroundColor(color);

    mSurface->SetFullSwapNextFrame();
  }
}

Vector4 SceneHolder::GetBackgroundColor() const
{
  return mScene ? mScene.GetBackgroundColor() : Color::BLACK;
}

void SceneHolder::SetAdaptor(Dali::Adaptor& adaptor)
{
  // Avoid doing this more than once
  if(mAdaptorStarted)
  {
    return;
  }

  DALI_ASSERT_DEBUG(mSurface && "Surface needs to be set before calling this method\n");

  // We can assume that current adaptor is already started now.
  mAdaptorStarted = true;

  // Create the scene
  PositionSize surfacePositionSize = mSurface->GetPositionSize();
  int          windowOrientation   = mSurface->GetSurfaceOrientation();
  int          screenOrientation   = mSurface->GetScreenOrientation();

  mScene = Dali::Integration::Scene::New(Size(static_cast<float>(surfacePositionSize.width), static_cast<float>(surfacePositionSize.height)), windowOrientation, screenOrientation);

  Internal::Adaptor::Adaptor& adaptorImpl = Internal::Adaptor::Adaptor::GetImplementation(adaptor);
  mAdaptor                                = &adaptorImpl;

  // Create an observer for the adaptor lifecycle
  mAdaptor->AddObserver(*mLifeCycleObserver);

  InitializeDpi();

  mSurface->SetAdaptor(*mAdaptor);
  mSurface->SetScene(mScene);

  // Create the render target
  CreateRenderTarget();

  OnAdaptorSet(adaptor);

  // Scene is newly created. Let we increase resize counter
  mAdaptor->IncreaseSurfaceResizeCounter();
}

void SceneHolder::CreateRenderTarget()
{
  Graphics::RenderTargetCreateInfo rtInfo{};
  rtInfo
    .SetSurface(mSurface.get())
    .SetExtent({static_cast<uint32_t>(mSurface->GetPositionSize().width), static_cast<uint32_t>(mSurface->GetPositionSize().height)})
    .SetPreTransform(0 | Graphics::RenderTargetTransformFlagBits::TRANSFORM_IDENTITY_BIT);

  mScene.SetSurfaceRenderTarget(rtInfo);
}

void SceneHolder::Pause()
{
  Reset();

  OnPause();
}

void SceneHolder::Resume()
{
  Reset();

  OnResume();
}

void SceneHolder::SurfaceRotated(float width, float height, int32_t windowOrientation, int32_t screenOrientation)
{
  mScene.SurfaceRotated(width, height, windowOrientation, screenOrientation);
}

void SceneHolder::SetRotationCompletedAcknowledgement()
{
  mScene.SetRotationCompletedAcknowledgement();
}

void SceneHolder::FeedTouchPoint(Dali::Integration::Point& point, int timeStamp)
{
  if(DALI_UNLIKELY(!mAdaptorStarted))
  {
    DALI_LOG_ERROR("Adaptor is stopped, or not be started yet. Ignore this feed.\n");
    return;
  }

  if(timeStamp < 1)
  {
    timeStamp = TimeService::GetMilliSeconds();
  }
  Vector2 convertedPosition = RecalculatePosition(point.GetScreenPosition());
  point.SetScreenPosition(convertedPosition);

  Integration::TouchEvent                            touchEvent;
  Integration::HoverEvent                            hoverEvent;
  Integration::TouchEventCombiner::EventDispatchType type = mCombiner.GetNextTouchEvent(point, timeStamp, touchEvent, hoverEvent, mHandledMultiTouch);
  if(type != Integration::TouchEventCombiner::DISPATCH_NONE)
  {
    DALI_LOG_INFO(gSceneHolderLogFilter, Debug::Verbose, "%d: Device %d: Button state %d (%.2f, %.2f)\n", timeStamp, point.GetDeviceId(), point.GetState(), point.GetScreenPosition().x, point.GetScreenPosition().y);

    // Signals can be emitted while processing core events, and the scene holder could be deleted in the signal callback.
    // Keep the handle alive until the core events are processed.
    Dali::BaseHandle sceneHolder(this);

    uint32_t pointCount = touchEvent.GetPointCount();
    if(pointCount > MAX_PRESSED_POINT_COUNT)
    {
      mPreviousTouchEvent = touchEvent;
      mPreviousHoverEvent = hoverEvent;
      if(mPreviousType == Integration::TouchEventCombiner::DISPATCH_NONE)
      {
        mPreviousType = type;
      }
      else if(mPreviousType != type)
      {
        mPreviousType = Integration::TouchEventCombiner::DISPATCH_BOTH;
      }
      mHandledMultiTouch = true;
    }

    if(type == Integration::TouchEventCombiner::DISPATCH_TOUCH || type == Integration::TouchEventCombiner::DISPATCH_BOTH)
    {
      mLastTouchEvent = Dali::Integration::NewTouchEvent(timeStamp, point);
    }

    if(type == Integration::TouchEventCombiner::DISPATCH_HOVER || type == Integration::TouchEventCombiner::DISPATCH_BOTH)
    {
      mLastHoverEvent = Dali::Integration::NewHoverEvent(timeStamp, point);
    }

    // Next the events are processed with a single call into Core
    if(pointCount <= MAX_PRESSED_POINT_COUNT || (point.GetState() != PointState::MOTION))
    {
      mHandledMultiTouch = false;
      mPreviousType      = Integration::TouchEventCombiner::DISPATCH_NONE;

      // First the touch and/or hover event & related gesture events are queued
      if(type == Integration::TouchEventCombiner::DISPATCH_TOUCH || type == Integration::TouchEventCombiner::DISPATCH_BOTH)
      {
        mScene.QueueEvent(touchEvent);
      }

      if(type == Integration::TouchEventCombiner::DISPATCH_HOVER || type == Integration::TouchEventCombiner::DISPATCH_BOTH)
      {
        mScene.QueueEvent(hoverEvent);
      }
      mAdaptor->ProcessCoreEvents();
    }
  }
}

void SceneHolder::FeedMouseFrameEvent()
{
  if(DALI_UNLIKELY(!mAdaptorStarted))
  {
    DALI_LOG_ERROR("Adaptor is stopped, or not be started yet. Ignore this feed.\n");
    return;
  }

  if(mPreviousType == Integration::TouchEventCombiner::DISPATCH_TOUCH || mPreviousType == Integration::TouchEventCombiner::DISPATCH_BOTH)
  {
    mScene.QueueEvent(mPreviousTouchEvent);
  }

  if(mPreviousType == Integration::TouchEventCombiner::DISPATCH_HOVER || mPreviousType == Integration::TouchEventCombiner::DISPATCH_BOTH)
  {
    mScene.QueueEvent(mPreviousHoverEvent);
  }

  if(mPreviousType != Integration::TouchEventCombiner::DISPATCH_NONE)
  {
    mAdaptor->ProcessCoreEvents();
  }

  mHandledMultiTouch = false;
  mPreviousType      = Integration::TouchEventCombiner::DISPATCH_NONE;
}

const Dali::TouchEvent& SceneHolder::GetLastTouchEvent() const
{
  return mLastTouchEvent;
}

const Dali::HoverEvent& SceneHolder::GetLastHoverEvent() const
{
  return mLastHoverEvent;
}

Dali::GestureState SceneHolder::GetLastPanGestureState()
{
  if(DALI_UNLIKELY(!mAdaptorStarted))
  {
    DALI_LOG_ERROR("Adaptor is stopped, or not be started yet. Ignore this GetLastPanGestureState.\n");
    return Dali::GestureState::CLEAR;
  }

  return mScene.GetLastPanGestureState();
}

void SceneHolder::FeedWheelEvent(Dali::Integration::WheelEvent& wheelEvent)
{
  if(DALI_UNLIKELY(!mAdaptorStarted))
  {
    DALI_LOG_ERROR("Adaptor is stopped, or not be started yet. Ignore this feed.\n");
    return;
  }

  // Signals can be emitted while processing core events, and the scene holder could be deleted in the signal callback.
  // Keep the handle alive until the core events are processed.
  Dali::BaseHandle sceneHolder(this);

  Vector2 convertedPosition = RecalculatePosition(wheelEvent.point);
  wheelEvent.point          = convertedPosition;

  mScene.QueueEvent(wheelEvent);
  mAdaptor->ProcessCoreEvents();
}

void SceneHolder::FeedKeyEvent(Dali::Integration::KeyEvent& keyEvent)
{
  if(DALI_UNLIKELY(!mAdaptorStarted))
  {
    DALI_LOG_ERROR("Adaptor is stopped, or not be started yet. Ignore this feed.\n");
    return;
  }

  Dali::PhysicalKeyboard physicalKeyboard = PhysicalKeyboard::Get();
  if(physicalKeyboard)
  {
    if(!KeyLookup::IsDeviceButton(keyEvent.keyName.c_str()))
    {
      GetImplementation(physicalKeyboard).KeyReceived(keyEvent.time > 1);
    }
  }

  // Signals can be emitted while processing core events, and the scene holder could be deleted in the signal callback.
  // Keep the handle alive until the core events are processed.
  Dali::BaseHandle sceneHolder(this);

  // Create send KeyEvent to Core.
  mScene.QueueEvent(keyEvent);
  mAdaptor->ProcessCoreEvents();
}

void SceneHolder::FeedHoverEvent(Dali::Integration::Point& point)
{
  if(DALI_UNLIKELY(!mAdaptorStarted))
  {
    DALI_LOG_ERROR("Adaptor is stopped, or not be started yet. Ignore this feed.\n");
    return;
  }

  Vector2 convertedPosition = RecalculatePosition(point.GetScreenPosition());
  point.SetScreenPosition(convertedPosition);

  Integration::HoverEvent hoverEvent;

  // Signals can be emitted while processing core events, and the scene holder could be deleted in the signal callback.
  // Keep the handle alive until the core events are processed.
  Dali::BaseHandle sceneHolder(this);

  // Create send HoverEvent to Core.
  hoverEvent.time = TimeService::GetMilliSeconds();
  hoverEvent.AddPoint(point);

  mScene.QueueEvent(hoverEvent);
  mAdaptor->ProcessCoreEvents();
}

void SceneHolder::SetGeometryHittestEnabled(bool enabled)
{
  mScene.SetGeometryHittestEnabled(enabled);
}

bool SceneHolder::IsGeometryHittestEnabled()
{
  return mScene.IsGeometryHittestEnabled();
}

int32_t SceneHolder::GetNativeId() const
{
  return mScene.GetNativeId();
}

void SceneHolder::AddFrameRenderedCallback(std::unique_ptr<CallbackBase> callback, int32_t frameId)
{
  mScene.AddFrameRenderedCallback(std::move(callback), frameId);

  DALI_LOG_RELEASE_INFO("SceneHolder::AddFrameRenderedCallback:: Added [%d]\n", frameId);
}

void SceneHolder::AddFramePresentedCallback(std::unique_ptr<CallbackBase> callback, int32_t frameId)
{
  mScene.AddFramePresentedCallback(std::move(callback), frameId);

  DALI_LOG_RELEASE_INFO("SceneHolder::AddFramePresentedCallback:: Added [%d]\n", frameId);
}

Dali::RenderTaskList SceneHolder::GetRenderTaskList() const
{
  return mScene.GetRenderTaskList();
}

Dali::Integration::SceneHolder SceneHolder::Get(Dali::Actor actor)
{
  SceneHolder* sceneHolderImpl = nullptr;

  if(Internal::Adaptor::Adaptor::IsAvailable())
  {
    Dali::Internal::Adaptor::Adaptor& adaptor = Internal::Adaptor::Adaptor::GetImplementation(Internal::Adaptor::Adaptor::Get());
    sceneHolderImpl                           = adaptor.GetWindow(actor);
  }

  return Dali::Integration::SceneHolder(sceneHolderImpl);
}

void SceneHolder::Reset()
{
  DALI_ASSERT_ALWAYS(mAdaptorStarted && "Adaptor is stopped, or not be started yet!");

  mCombiner.Reset();

  // Any touch listeners should be told of the interruption.
  Integration::TouchEvent event;
  Integration::Point      point;
  point.SetState(PointState::INTERRUPTED);
  event.AddPoint(point);

  // First the touch event & related gesture events are queued
  mScene.QueueEvent(event);

  // Any hover listeners should be told of the interruption.
  Integration::HoverEvent hoverEvent;
  Integration::Point      hoverPoint;
  hoverPoint.SetState(PointState::INTERRUPTED);
  hoverEvent.AddPoint(hoverPoint);

  // First the hover event & related gesture events are queued
  mScene.QueueEvent(hoverEvent);

  // Next the events are processed with a single call into Core
  mHandledMultiTouch = false;
  mPreviousType      = Integration::TouchEventCombiner::DISPATCH_NONE;
  mAdaptor->ProcessCoreEvents();
}

void SceneHolder::InitializeDpi()
{
  unsigned int dpiHorizontal, dpiVertical;
  dpiHorizontal = dpiVertical = 0;

  mSurface->GetDpi(dpiHorizontal, dpiVertical);
  mScene.SetDpi(Vector2(static_cast<float>(dpiHorizontal), static_cast<float>(dpiVertical)));

  mDpi.SetX(dpiHorizontal);
  mDpi.SetY(dpiVertical);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
