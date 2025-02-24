#ifndef DALI_INTEGRATION_INTERNAL_SCENEHOLDER_H
#define DALI_INTEGRATION_INTERNAL_SCENEHOLDER_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/graphics-api/graphics-controller.h>
#include <dali/integration-api/events/hover-event-integ.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/point.h>
#include <dali/integration-api/events/touch-event-combiner.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/scene.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/events/hover-event.h>
#include <dali/public-api/events/touch-event.h>
#include <dali/public-api/events/gesture-enumerations.h>
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/public-api/object/base-object.h>
#include <atomic>
#include <memory>
#include <vector>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>

namespace Dali
{
class Any;
class Adaptor;
class Actor;
class Layer;
class WheelEvent;
struct TouchPoint;
class KeyEvent;

namespace Integration
{
class Scene;
struct Point;
struct KeyEvent;
struct WheelEvent;

} // namespace Integration

namespace Internal
{
namespace Adaptor
{
class Adaptor;
class SceneHolder;
using SceneHolderPtr = IntrusivePtr<SceneHolder>;

/**
 * @brief SceneHolder creates a Scene for rendering.
 */
class DALI_ADAPTOR_API SceneHolder : public BaseObject
{
public:
  /**
   * @copydoc Dali::Integration::SceneHolder::Add
   */
  void Add(Dali::Actor actor);

  /**
   * @copydoc Dali::Integration::SceneHolder::Remove
   */
  void Remove(Dali::Actor actor);

  /**
   * @copydoc Dali::Integration::SceneHolder::GetRootLayer
   */
  Dali::Layer GetRootLayer() const;

  /**
   * @copydoc Dali::Integration::SceneHolder::GetOverlayLayer
   */
  Dali::Layer GetOverlayLayer();

  /**
   * @brief Gets the window name.
   * @return The name of the window
   */
  std::string GetName() const;

  /**
   * @brief Retrieve the unique ID of the window.
   * @return The ID
   */
  uint32_t GetId() const;

  /**
   * @brief Retrieve the Scene.
   * @return The Scene
   */
  Dali::Integration::Scene GetScene();

  /**
   * @brief Retrieves the DPI of this sceneholder.
   * @return The DPI.
   */
  Uint16Pair GetDpi() const;

  /**
   * @brief Set the render surface
   * @param[in] surface The render surface
   */
  void SetSurface(Dali::Integration::RenderSurfaceInterface* surface);

  /**
   * @brief Called when the surface set is resized.
   * @param[in] width the resized window's width
   * @param[in] height the resized window's height
   */
  void SurfaceResized(float width, float height);

  /**
   * @brief Get the render surface
   * @return The render surface
   */
  Dali::Integration::RenderSurfaceInterface* GetSurface() const;

  /**
   * @brief Set the adaptor to the scene holder
   * @param[in] adaptor An initialized adaptor
   */
  void SetAdaptor(Dali::Adaptor& adaptor);

  /**
   * @copydoc Dali::Integration::SceneHolder::SetBackgroundColor
   */
  void SetBackgroundColor(const Dali::Vector4& color);

  /**
   * @copydoc Dali::Integration::SceneHolder::GetBackgroundColor
   */
  Vector4 GetBackgroundColor() const;

  /**
   * @brief Pause the rendering of the scene holder.
   */
  void Pause();

  /**
   * @brief Resume the rendering of the scene holder (from pause).
   */
  void Resume();

  /**
   * @brief Informs the scene that the set surface has been rotated.
   *
   * @param[in] width The width of rotated surface
   * @param[in] height The height of rotated surface
   * @param[in] windowOrientation the current window orientation
   * @param[in] screenOrientation the current screen orientation
   */
  void SurfaceRotated(float width, float height, int32_t windowOrientation, int32_t screenOrientation);

  /**
   * @brief Send message to acknowledge window rotation with current window orientation.
   * It is to send message to render thread for completing window rotation by user.
   */
  void SetRotationCompletedAcknowledgement();

  /**
   * @copydoc Dali::Integration::SceneHolder::FeedTouchPoint
   */
  void FeedTouchPoint(Dali::Integration::Point& point, int timeStamp);

  /**
   * @copydoc Dali::Integration::SceneHolder::FeedMouseFrameEvent
   */
  void FeedMouseFrameEvent();

  /**
   * @brief Get the Last Touch Event
   *
   * @return Dali::TouchEvent
   */
  const Dali::TouchEvent& GetLastTouchEvent() const;

  /**
   * @brief Get the Last Hover Event
   *
   * @return Dali::HoverEvent
   */
  const Dali::HoverEvent& GetLastHoverEvent() const;

  /**
   * @brief Gets the last pan gesture state
   *
   * @return Dali::GestureState
   */
  Dali::GestureState GetLastPanGestureState();

  /**
   * @copydoc Dali::Integration::SceneHolder::FeedWheelEvent
   */
  void FeedWheelEvent(Dali::Integration::WheelEvent& wheelEvent);

  /**
   * @copydoc Dali::Integration::SceneHolder::FeedKeyEvent
   */
  void FeedKeyEvent(Dali::Integration::KeyEvent& keyEvent);

  /**
   * @copydoc Dali::Integration::SceneHolder::FeedHoverEvent
   */
  void FeedHoverEvent(Dali::Integration::Point& point);

  /**
   * @copydoc Dali::Integration::SceneHolder::SetGeometryHittestEnabled
   */
  void SetGeometryHittestEnabled(bool enabled);

  /**
   * @copydoc Dali::Integration::SceneHolder::IsGeometryHittestEnabled
   */
  bool IsGeometryHittestEnabled();

  /**
   * @copydoc Dali::Integration::SceneHolder::GetNativeId
   */
  int32_t GetNativeId() const;

  /**
   * @brief Adds a callback that is called when the frame rendering is done by the graphics driver.
   *
   * @param[in] callback The function to call
   * @param[in] frameId The Id to specify the frame. It will be passed when the callback is called.
   *
   * @note A callback of the following type may be used:
   * @code
   *   void MyFunction( int frameId );
   * @endcode
   * This callback will be deleted once it is called.
   *
   * @note Ownership of the callback is passed onto this class.
   */
  void AddFrameRenderedCallback(std::unique_ptr<CallbackBase> callback, int32_t frameId);

  /**
   * @brief Adds a callback that is called when the frame rendering is done by the graphics driver.
   *
   * @param[in] callback The function to call
   * @param[in] frameId The Id to specify the frame. It will be passed when the callback is called.
   *
   * @note A callback of the following type may be used:
   * @code
   *   void MyFunction( int frameId );
   * @endcode
   * This callback will be deleted once it is called.
   *
   * @note Ownership of the callback is passed onto this class.
   */
  void AddFramePresentedCallback(std::unique_ptr<CallbackBase> callback, int32_t frameId);

  /**
   * @brief Creates the render target for the surface when the surface is created/resized/replaced.
   */
  void CreateRenderTarget();

  /**
   * @copydoc Dali::Integration::SceneHolder::GetRenderTaskList()
   */
  Dali::RenderTaskList GetRenderTaskList() const;

  /**
   * @copydoc Dali::Integration::SceneHolder::Get()
   */
  static Dali::Integration::SceneHolder Get(Dali::Actor actor);

  /**
   * @copydoc Dali::Integration::SceneHolder::KeyEventSignal()
   */
  Dali::Integration::SceneHolder::KeyEventSignalType& KeyEventSignal()
  {
    return mScene.KeyEventSignal();
  }

  /**
   * @copydoc Dali::Integration::SceneHolder::KeyEventGeneratedSignal()
   */
  Dali::Integration::SceneHolder::KeyEventGeneratedSignalType& KeyEventGeneratedSignal()
  {
    return mScene.KeyEventGeneratedSignal();
  }

  /**
   * @copydoc Dali::Integration::SceneHolder::InterceptKeyEventSignal()
   */
  Dali::Integration::SceneHolder::KeyEventGeneratedSignalType& InterceptKeyEventSignal()
  {
    return mScene.InterceptKeyEventSignal();
  }

  /**
   * @copydoc Dali::Integration::SceneHolder::KeyEventMonitorSignal()
   */
  Dali::Integration::SceneHolder::KeyEventSignalType& KeyEventMonitorSignal()
  {
    return mScene.KeyEventMonitorSignal();
  }

  /**
   * @copydoc Dali::Integration::SceneHolder::TouchedSignal()
   */
  Dali::Integration::SceneHolder::TouchEventSignalType& TouchedSignal()
  {
    return mScene.TouchedSignal();
  }

  /**
   * @copydoc Dali::Integration::SceneHolder::WheelEventSignal()
   */
  Dali::Integration::SceneHolder::WheelEventSignalType& WheelEventSignal()
  {
    return mScene.WheelEventSignal();
  }

  /**
   * @copydoc Dali::Integration::SceneHolder::WheelEventGeneratedSignal()
   */
  Dali::Integration::SceneHolder::WheelEventGeneratedSignalType& WheelEventGeneratedSignal()
  {
    return mScene.WheelEventGeneratedSignal();
  }

public: // The following methods can be overridden if required
  /**
   * @brief Returns whether the Scene is visible or not.
   * @return True if the Scene is visible, false otherwise.
   */
  virtual bool IsVisible() const;

public: // The following methods must be overridden
  /**
   * @copydoc Dali::Integration::SceneHolder::GetNativeHandle
   */
  virtual Dali::Any GetNativeHandle() const = 0;

protected:
  // Constructor
  SceneHolder();

  // Undefined
  SceneHolder(const SceneHolder&) = delete;

  // Undefined
  SceneHolder& operator=(const SceneHolder& rhs) = delete;

  /**
   * virtual destructor
   */
  ~SceneHolder() override;

private: // The following methods can be overridden if required
  /**
   * @brief Called by the base class to inform deriving classes that the adaptor has been set.
   * @param[in] adaptor The adaptor
   */
  virtual void OnAdaptorSet(Dali::Adaptor& adaptor){};

  /**
   * @brief Called by the base class to inform deriving classes that a new surface has been set.
   * @param[in] surface The new render surface
   */
  virtual void OnSurfaceSet(Dali::Integration::RenderSurfaceInterface* surface){};

  /**
   * @brief Called by the base class to inform deriving classes that we are being paused.
   */
  virtual void OnPause(){};

  /**
   * @brief Called by the base class to inform deriving classes that we are resuming from a paused state.
   */
  virtual void OnResume(){};

  /**
   * Recalculate the position if required
   * @param[in] position The screen position
   * @return converted position by oriention
   */
  virtual Vector2 RecalculatePosition(const Vector2& position)
  {
    return position;
  };

private:
  /**
   * Resets the event handling.
   */
  void Reset();

  /**
   * Initializes the DPI for this object.
   */
  void InitializeDpi();

private:
  static uint32_t mSceneHolderCounter; ///< A counter to track the SceneHolder creation

  class SceneHolderLifeCycleObserver;
  std::unique_ptr<SceneHolderLifeCycleObserver> mLifeCycleObserver; ///< The adaptor life cycle observer
  Dali::TouchEvent                              mLastTouchEvent;
  Dali::HoverEvent                              mLastHoverEvent;

protected:
  uint32_t                 mId;    ///< A unique ID to identify the SceneHolder starting from 0
  Dali::Integration::Scene mScene; ///< The Scene
  std::string              mName;  ///< The name of the SceneHolder

  std::unique_ptr<Dali::Integration::RenderSurfaceInterface> mSurface; ///< The window rendering surface
  Adaptor*                                      mAdaptor; ///< The adaptor

  Dali::Integration::TouchEventCombiner mCombiner; ///< Combines multi-touch events.

  Uint16Pair mDpi; ///< The DPI for this SceneHolder.

  bool                                               mAdaptorStarted; ///< Whether the adaptor has started or not
  bool                                               mVisible : 1;    ///< Whether the scene is visible or not
  bool                                               mHandledMultiTouch : 1;
  Integration::TouchEvent                            mPreviousTouchEvent;
  Integration::HoverEvent                            mPreviousHoverEvent;
  Integration::TouchEventCombiner::EventDispatchType mPreviousType;
};

} // namespace Adaptor

} // namespace Internal

// Get impl of handle
inline Internal::Adaptor::SceneHolder& GetImplementation(Dali::Integration::SceneHolder& sceneHolder)
{
  DALI_ASSERT_ALWAYS(sceneHolder && "SceneHolder handle is empty");
  Dali::RefObject& object = sceneHolder.GetBaseObject();
  return static_cast<Internal::Adaptor::SceneHolder&>(object);
}

inline const Internal::Adaptor::SceneHolder& GetImplementation(const Dali::Integration::SceneHolder& sceneHolder)
{
  DALI_ASSERT_ALWAYS(sceneHolder && "SceneHolder handle is empty");
  const Dali::RefObject& object = sceneHolder.GetBaseObject();
  return static_cast<const Internal::Adaptor::SceneHolder&>(object);
}

} // namespace Dali

#endif // DALI_INTEGRATION_INTERNAL_SCENEHOLDER_H
