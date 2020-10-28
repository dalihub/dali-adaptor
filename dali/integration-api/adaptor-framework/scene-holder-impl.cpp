/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/events/wheel-event-integ.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/common/dali-common.h>
#include <sys/time.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/lifecycle-observer.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/input/common/physical-keyboard-impl.h>

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

// Copied from x server
static uint32_t GetCurrentMilliSeconds(void)
{
  struct timeval tv;

  struct timespec  tp;
  static clockid_t clockid;

  if(!clockid)
  {
#ifdef CLOCK_MONOTONIC_COARSE
    if(clock_getres(CLOCK_MONOTONIC_COARSE, &tp) == 0 &&
       (tp.tv_nsec / 1000) <= 1000 && clock_gettime(CLOCK_MONOTONIC_COARSE, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC_COARSE;
    }
    else
#endif
      if(clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC;
    }
    else
    {
      clockid = ~0L;
    }
  }
  if(clockid != ~0L && clock_gettime(clockid, &tp) == 0)
  {
    return static_cast<uint32_t>((tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L));
  }

  gettimeofday(&tv, NULL);
  return static_cast<uint32_t>((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

} // unnamed namespace

uint32_t SceneHolder::mSceneHolderCounter = 0;

class SceneHolder::SceneHolderLifeCycleObserver : public LifeCycleObserver
{
public:
  SceneHolderLifeCycleObserver(Adaptor*& adaptor)
  : mAdaptor(adaptor){};

private: // Adaptor::LifeCycleObserver interface
  void OnStart() override{};
  void OnPause() override{};
  void OnResume() override{};
  void OnStop() override{};
  void OnDestroy() override
  {
    mAdaptor = nullptr;
  };

private:
  Adaptor*& mAdaptor;
};

SceneHolder::SceneHolder()
: mLifeCycleObserver(new SceneHolderLifeCycleObserver(mAdaptor)),
  mId(mSceneHolderCounter++),
  mSurface(nullptr),
  mAdaptor(nullptr),
  mDpi(),
  mIsBeingDeleted(false),
  mAdaptorStarted(false),
  mVisible(true)
{
}

SceneHolder::~SceneHolder()
{
  if(mAdaptor)
  {
    mAdaptor->RemoveObserver(*mLifeCycleObserver.get());
    mAdaptor->RemoveWindow(this);

    mAdaptor->DeleteSurface(*mSurface.get());

    mAdaptor = nullptr;
  }

  if(mScene)
  {
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

void SceneHolder::SetSurface(Dali::RenderSurfaceInterface* surface)
{
  mSurface.reset(surface);

  mScene.SurfaceReplaced();

  SurfaceResized();

  InitializeDpi();

  mSurface->SetAdaptor(*mAdaptor);
  mSurface->SetScene(mScene);

  OnSurfaceSet(surface);
}

void SceneHolder::SurfaceResized()
{
  PositionSize surfacePositionSize = mSurface->GetPositionSize();
  mScene.SurfaceResized(static_cast<float>(surfacePositionSize.width), static_cast<float>(surfacePositionSize.height));

  mSurface->SetFullSwapNextFrame();
}

Dali::RenderSurfaceInterface* SceneHolder::GetSurface() const
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

  mAdaptorStarted = true;

  // Create the scene
  PositionSize surfacePositionSize = mSurface->GetPositionSize();
  mScene                           = Dali::Integration::Scene::New(Size(static_cast<float>(surfacePositionSize.width), static_cast<float>(surfacePositionSize.height)));

  Internal::Adaptor::Adaptor& adaptorImpl = Internal::Adaptor::Adaptor::GetImplementation(adaptor);
  mAdaptor                                = &adaptorImpl;

  // Create an observer for the adaptor lifecycle
  mAdaptor->AddObserver(*mLifeCycleObserver);

  InitializeDpi();

  mSurface->SetAdaptor(*mAdaptor);
  mSurface->SetScene(mScene);

  OnAdaptorSet(adaptor);
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

void SceneHolder::FeedTouchPoint(Dali::Integration::Point& point, int timeStamp)
{
  if(timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  RecalculateTouchPosition(point);

  Integration::TouchEvent                            touchEvent;
  Integration::HoverEvent                            hoverEvent;
  Integration::TouchEventCombiner::EventDispatchType type = mCombiner.GetNextTouchEvent(point, timeStamp, touchEvent, hoverEvent);
  if(type != Integration::TouchEventCombiner::DISPATCH_NONE)
  {
    DALI_LOG_INFO(gSceneHolderLogFilter, Debug::Verbose, "%d: Device %d: Button state %d (%.2f, %.2f)\n", timeStamp, point.GetDeviceId(), point.GetState(), point.GetScreenPosition().x, point.GetScreenPosition().y);

    // Signals can be emitted while processing core events, and the scene holder could be deleted in the signal callback.
    // Keep the handle alive until the core events are processed.
    Dali::BaseHandle sceneHolder(this);

    // First the touch and/or hover event & related gesture events are queued
    if(type == Integration::TouchEventCombiner::DISPATCH_TOUCH || type == Integration::TouchEventCombiner::DISPATCH_BOTH)
    {
      mScene.QueueEvent(touchEvent);
    }

    if(type == Integration::TouchEventCombiner::DISPATCH_HOVER || type == Integration::TouchEventCombiner::DISPATCH_BOTH)
    {
      mScene.QueueEvent(hoverEvent);
    }

    // Next the events are processed with a single call into Core
    mAdaptor->ProcessCoreEvents();
  }
}

void SceneHolder::FeedWheelEvent(Dali::Integration::WheelEvent& wheelEvent)
{
  // Signals can be emitted while processing core events, and the scene holder could be deleted in the signal callback.
  // Keep the handle alive until the core events are processed.
  Dali::BaseHandle sceneHolder(this);

  mScene.QueueEvent(wheelEvent);
  mAdaptor->ProcessCoreEvents();
}

void SceneHolder::FeedKeyEvent(Dali::Integration::KeyEvent& keyEvent)
{
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

void SceneHolder::AddFrameRenderedCallback(std::unique_ptr<CallbackBase> callback, int32_t frameId)
{
  mScene.AddFrameRenderedCallback(std::move(callback), frameId);

  DALI_LOG_INFO(gSceneHolderLogFilter, Debug::General, "SceneHolder::AddFrameRenderedCallback:: Added [%d]\n", frameId);
}

void SceneHolder::AddFramePresentedCallback(std::unique_ptr<CallbackBase> callback, int32_t frameId)
{
  mScene.AddFramePresentedCallback(std::move(callback), frameId);

  DALI_LOG_INFO(gSceneHolderLogFilter, Debug::General, "SceneHolder::AddFramePresentedCallback:: Added [%d]\n", frameId);
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
  mCombiner.Reset();

  // Any touch listeners should be told of the interruption.
  Integration::TouchEvent event;
  Integration::Point      point;
  point.SetState(PointState::INTERRUPTED);
  event.AddPoint(point);

  // First the touch event & related gesture events are queued
  mScene.QueueEvent(event);

  // Next the events are processed with a single call into Core
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
