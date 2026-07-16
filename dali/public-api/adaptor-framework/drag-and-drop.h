#ifndef DALI_DRAG_AND_DROP_H
#define DALI_DRAG_AND_DROP_H

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
 *
 */

// EXTERNAL INCLUDES
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/common/dali-string.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class DragAndDrop;
class DragEventBuilder;
}
} //namespace Internal DALI_INTERNAL

/**
 * @brief Interface to the platform drag-and-drop service.
 *
 * DragAndDrop starts a drag from an Actor and delivers drag events to registered
 * Actor or Window targets. The supported drag-and-drop behavior depends on the
 * platform backend.
 *
 * @SINCE_2_5.32
 */

class DALI_ADAPTOR_API DragAndDrop : public BaseHandle
{
public:
  /**
   * @brief Enumeration for the drag source event type in the source object
   *
   * @SINCE_2_5.32
   */
  enum class SourceEventType
  {
    START,  ///< Drag and drop is started. @SINCE_2_5.32
    CANCEL, ///< Drag and drop is cancelled. @SINCE_2_5.32
    ACCEPT, ///< Drag and drop is accepted. @SINCE_2_5.32
    FINISH  ///< Drag and drop is finished. @SINCE_2_5.32
  };

  /**
   * @brief Enumeration for the drag event type in the target object
   *
   * @SINCE_2_5.32
   */
  enum class DragType
  {
    ENTER, ///< The drag object has entered the target object. @SINCE_2_5.32
    LEAVE, ///< The drag object has left the target object. @SINCE_2_5.32
    MOVE,  ///< The drag object moves in the target object. @SINCE_2_5.32
    DROP   ///< The drag object dropped in the target object. @SINCE_2_5.32
  };

  /**
   * @brief Describes an event delivered to a drop target.
   *
   * @SINCE_2_5.32
   */
  class DragEvent;

  /**
   * @brief Describes the MIME types and data offered by a drag source.
   *
   * DragData owns the MIME type and data strings added to it.
   *
   * @SINCE_2_5.32
   */
  class DragData;

  /**
   * @brief Signal emitted for drag-source lifecycle events.
   *
   * @SINCE_2_5.32
   */
  using SourceEventSignalType = Signal<void(DragAndDrop, Actor, SourceEventType)>;

  /**
   * @brief Signal emitted for events delivered to Actor drop targets.
   *
   * @SINCE_2_5.32
   */
  using ActorDragEventSignalType = Signal<void(DragAndDrop, Actor, const DragEvent&)>;

  /**
   * @brief Signal emitted for events delivered to Window drop targets.
   *
   * @SINCE_2_5.32
   */
  using WindowDragEventSignalType = Signal<void(DragAndDrop, Window, const DragEvent&)>;

  /**
   * @brief Create an uninitialized DragAndDrop.
   *
   * this can be initialized with one of the derived DragAndDrop's New() methods
   *
   * @SINCE_2_5.32
   */
  DragAndDrop();

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   *
   * @SINCE_2_5.32
   */
  ~DragAndDrop();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @SINCE_2_5.32
   */
  DragAndDrop(const DragAndDrop& handle) = default;

  /**
   * @brief Retrieve a handle to the DragAndDrop instance.
   *
   * @return A handle to the DragAndDrop
   * @SINCE_2_5.32
   */
  static DragAndDrop Get();

  /**
   * @brief Start a drag operation.
   *
   * @param[in] source The drag source Actor.
   * @param[in] shadowWindow The Window displayed while dragging.
   * @param[in] dragData MIME types and corresponding data to offer to targets.
   * @return True if the platform accepted the drag operation.
   *
   * @note Connect SourceEventSignal() before calling this function to receive
   * source lifecycle events. dragData must contain at least one non-empty MIME
   * type.
   * @SINCE_2_5.32
   */
  bool StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const DragData& dragData);

  /**
   * @brief Register an Actor as a drop target.
   *
   * @param[in] target The drop target Actor.
   * @param[in] mimeType MIME type accepted by the target.
   * @return True if the listener is added successfully.
   *
   * Connect ActorDragEventSignal() to receive events. The MIME type is copied
   * during this call.
   * @SINCE_2_5.32
   */
  bool AddListener(Dali::Actor target, const Dali::String& mimeType);

  /**
   * @brief Remove a previously registered Actor drop target.
   *
   * @param[in] target The drop target Actor.
   * @return True if the listener is removed successfully.
   * @SINCE_2_5.32
   */
  bool RemoveListener(Dali::Actor target);

  /**
   * @brief Register a Window as a drop target.
   *
   * @param[in] target The drop target Window.
   * @param[in] mimeType MIME type accepted by the target.
   * @return True if the listener is added successfully.
   *
   * Connect WindowDragEventSignal() to receive events. The MIME type is copied
   * during this call.
   * @SINCE_2_5.32
   */
  bool AddListener(Dali::Window target, const Dali::String& mimeType);

  /**
   * @brief Remove a previously registered Window drop target.
   *
   * @param[in] target The drop target Window.
   * @return True if the listener is removed successfully.
   * @SINCE_2_5.32
   */
  bool RemoveListener(Dali::Window target);

  /**
   * @brief Gets the drag-source lifecycle signal.
   *
   * @SINCE_2_5.32
   */
  SourceEventSignalType& SourceEventSignal();

  /**
   * @brief Gets the Actor drop-target event signal.
   *
   * @SINCE_2_5.32
   */
  ActorDragEventSignalType& ActorDragEventSignal();

  /**
   * @brief Gets the Window drop-target event signal.
   *
   * @SINCE_2_5.32
   */
  WindowDragEventSignalType& WindowDragEventSignal();

public:
  /**
   * @brief This constructor is used by Adaptor::GetDragAndDrop().
   *
   * @param[in] dnd A pointer to the DragAndDrop.
   */
  explicit DALI_INTERNAL DragAndDrop(Internal::Adaptor::DragAndDrop* dnd);
};

} // namespace Dali

#endif // DALI_DRAG_AND_DROP_H
