#ifndef DALI_INTEGRATION_INTERNAL_SCENEHOLDER_H
#define DALI_INTEGRATION_INTERNAL_SCENEHOLDER_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <memory>
#include <vector>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/integration-api/scene.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/point.h>
#include <dali/integration-api/events/touch-event-combiner.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>

namespace Dali
{

class Any;
class Adaptor;
class Actor;
class Layer;
struct TouchPoint;
struct WheelEvent;
struct KeyEvent;

namespace Integration
{

class Scene;
struct Point;
struct KeyEvent;
struct WheelEvent;

}

namespace Internal
{

namespace Adaptor
{

class Adaptor;
class SceneHolder;
using SceneHolderPtr = IntrusivePtr< SceneHolder >;

/**
 * @brief SceneHolder creates a Scene for rendering.
 */
class DALI_ADAPTOR_API SceneHolder : public BaseObject
{

public:

  /**
   * @copydoc Dali::Integration::SceneHolder::Add
   */
  void Add( Dali::Actor actor );

  /**
   * @copydoc Dali::Integration::SceneHolder::Remove
   */
  void Remove( Dali::Actor actor );

  /**
   * @copydoc Dali::Integration::SceneHolder::GetRootLayer
   */
  Dali::Layer GetRootLayer() const;

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
   * @brief Set the render surface
   * @param[in] surface The render surface
   */
  void SetSurface( Dali::RenderSurfaceInterface* surface );

  /**
   * @brief Called when the surface set is resized.
   */
  void SurfaceResized();

  /**
   * @brief Get the render surface
   * @return The render surface
   */
  Dali::RenderSurfaceInterface* GetSurface() const;

  /**
   * @brief Set the adaptor to the scene holder
   * @param[in] adaptor An initialized adaptor
   */
  void SetAdaptor( Dali::Adaptor& adaptor );

  /**
   * @copydoc Dali::Integration::SceneHolder::SetBackgroundColor
   */
  void SetBackgroundColor( const Dali::Vector4& color );

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
   * @copydoc Dali::Integration::SceneHolder::FeedTouchPoint
   */
  void FeedTouchPoint( Dali::Integration::Point& point, int timeStamp );

  /**
   * @copydoc Dali::Integration::SceneHolder::FeedWheelEvent
   */
  void FeedWheelEvent( Dali::Integration::WheelEvent& wheelEvent );

  /**
   * @copydoc Dali::Integration::SceneHolder::FeedKeyEvent
   */
  void FeedKeyEvent( Dali::Integration::KeyEvent& keyEvent );

  /**
   * @copydoc Dali::Integration::SceneHolder::Get()
   */
  static Dali::Integration::SceneHolder Get( Dali::Actor actor );

  /**
   * @copydoc Dali::Integration::SceneHolder::KeyEventSignal()
   */
  Dali::Integration::SceneHolder::KeyEventSignalType& KeyEventSignal() { return mScene.KeyEventSignal(); }

  /**
   * @copydoc Dali::Integration::SceneHolder::KeyEventGeneratedSignal()
   */
  Dali::Integration::SceneHolder::KeyEventGeneratedSignalType& KeyEventGeneratedSignal() { return mScene.KeyEventGeneratedSignal(); }

  /**
   * @copydoc Dali::Integration::SceneHolder::TouchSignal()
   */
  Dali::Integration::SceneHolder::TouchSignalType& TouchSignal() { return mScene.TouchSignal(); }

  /**
   * @copydoc Dali::Integration::SceneHolder::WheelEventSignal()
   */
  Dali::Integration::SceneHolder::WheelEventSignalType& WheelEventSignal() { return mScene.WheelEventSignal(); }

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
  virtual ~SceneHolder();

private: // The following methods can be overridden if required

  /**
   * @brief Called by the base class to inform deriving classes that the adaptor has been set.
   * @param[in] adaptor The adaptor
   */
  virtual void OnAdaptorSet( Dali::Adaptor& adaptor ) {};

  /**
   * @brief Called by the base class to inform deriving classes that a new surface has been set.
   * @param[in] surface The new render surface
   */
  virtual void OnSurfaceSet( Dali::RenderSurfaceInterface* surface ) {};

  /**
   * @brief Called by the base class to inform deriving classes that we are being paused.
   */
  virtual void OnPause() {};

  /**
   * @brief Called by the base class to inform deriving classes that we are resuming from a paused state.
   */
  virtual void OnResume() {};

  /**
   * Recalculate the touch position if required
   * @param[in,out] point The touch point
   */
  virtual void RecalculateTouchPosition( Integration::Point& point ) {};

private:

  /**
   * Resets the event handling.
   */
  void Reset();

private:

  static uint32_t                                 mSceneHolderCounter; ///< A counter to track the SceneHolder creation

  class SceneHolderLifeCycleObserver;
  std::unique_ptr< SceneHolderLifeCycleObserver > mLifeCycleObserver;  ///< The adaptor life cycle observer

protected:

  uint32_t                                        mId;                 ///< A unique ID to identify the SceneHolder starting from 0
  Dali::Integration::Scene                        mScene;              ///< The Scene
  std::string                                     mName;               ///< The name of the SceneHolder

  std::unique_ptr< Dali::RenderSurfaceInterface > mSurface;            ///< The window rendering surface
  Adaptor*                                        mAdaptor;            ///< The adaptor

  Dali::Integration::TouchEventCombiner           mCombiner;           ///< Combines multi-touch events.

  bool                                            mAdaptorStarted:1;   ///< Whether the adaptor has started or not
  bool                                            mVisible:1;          ///< Whether the scene is visible or not
};

} // Adaptor

} // Internal

// Get impl of handle
inline Internal::Adaptor::SceneHolder& GetImplementation( Dali::Integration::SceneHolder& sceneHolder )
{
  DALI_ASSERT_ALWAYS( sceneHolder && "SceneHolder handle is empty" );
  Dali::RefObject& object = sceneHolder.GetBaseObject();
  return static_cast<Internal::Adaptor::SceneHolder&>( object );
}

inline const Internal::Adaptor::SceneHolder& GetImplementation( const Dali::Integration::SceneHolder& sceneHolder )
{
  DALI_ASSERT_ALWAYS( sceneHolder && "SceneHolder handle is empty" );
  const Dali::RefObject& object = sceneHolder.GetBaseObject();
  return static_cast<const Internal::Adaptor::SceneHolder&>( object );
}

} // Dali

#endif // DALI_INTEGRATION_INTERNAL_SCENEHOLDER_H
