#ifndef DALI_DRAG_AND_DROP_H
#define DALI_DRAG_AND_DROP_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/object/base-handle.h>
#include <functional>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class DragAndDrop;
}
} //namespace Internal DALI_INTERNAL

/**
 * @brief Interface to the device's drag and drop.
 *
 * DragAndDrop class supports the drag and drop functionality for multi-window.
 */

class DALI_ADAPTOR_API DragAndDrop : public BaseHandle
{
public:
  /**
   * @brief Enumeration for the drag source event type in the source object
   */
  enum class SourceEventType
  {
    START,  ///< Drag and drop is started.
    CANCEL, ///< Drag and drop is cancelled.
    ACCEPT, ///< Drag and drop is accepted.
    FINISH  ///< Drag and drop is finished.
  };

  /**
   * @brief Enumeration for the drag event type in the target object
   */
  enum class DragType
  {
    ENTER, ///< The drag object has entered the target object.
    LEAVE, ///< The drag object has left the target object.
    MOVE,  ///< The drag object moves in the target object.
    DROP   ///< The drag object dropped in the target object.
  };

  /**
   * @brief Structure that contains information about the drag event information.
   */
  struct DragEvent
  {
    DragEvent()
    {
      this->mimeTypes = nullptr;
      this->data      = nullptr;
    }
    DragEvent(DragType type, Dali::Vector2 position, const char** mimeTypes = nullptr, int mimeTypesSize = 0, char* data = nullptr)
    {
      this->type          = type;
      this->position      = position;
      this->mimeTypes     = mimeTypes;
      this->mimeTypesSize = mimeTypesSize;
      this->data          = data;
    }

    void SetAction(DragType type)
    {
      this->type = type;
    }
    DragType GetAction()
    {
      return type;
    }
    void SetPosition(Dali::Vector2 position)
    {
      this->position = position;
    }
    Dali::Vector2 GetPosition()
    {
      return position;
    }
    void SetMimeTypes(const char** mimeTypes, int mimeTypeSize)
    {
      this->mimeTypes     = mimeTypes;
      this->mimeTypesSize = mimeTypeSize;
    }
    const char** GetMimeTypes()
    {
      return this->mimeTypes;
    }
    int GetMimeTypesSize() const
    {
      return mimeTypesSize;
    }
    void SetData(char* data)
    {
      this->data = data;
    }
    char* GetData() const
    {
      return data;
    }

  private:
    DragType      type{DragType::DROP}; ///< The drag event type.
    Dali::Vector2 position;             ///< The position of drag object.
    const char**  mimeTypes{nullptr};   ///< The mime types of drag object.
    char*         data{nullptr};        ///< The data of drag object.
    int           mimeTypesSize{0};     ///< The size of mime types array.
  };

  /**
   * @brief Structure that contains information about the drag data information.
   */
  struct DragData
  {
    void SetMimeTypes(const char** mimeTypes, int mimeTypesSize)
    {
      this->mimeTypes     = mimeTypes;
      this->mimeTypesSize = mimeTypesSize;
    }

    const char** GetMimeTypes() const
    {
      return mimeTypes;
    }

    int GetMimeTypesSize() const
    {
      return mimeTypesSize;
    }

    void SetDataSet(const char** dataSet, int dataSetSize)
    {
      this->dataSet     = dataSet;
      this->dataSetSize = dataSetSize;
    }

    const char** GetDataSet() const
    {
      return dataSet;
    }

    int GetDataSetSize() const
    {
      return dataSetSize;
    }

  private:
    const char** mimeTypes{nullptr}; ///< The mime types of drag object.
    int          mimeTypesSize{0};   ///< The size of mime types.
    const char** dataSet{nullptr};   ///<The drag data set.
    int          dataSetSize{0};     ///<The size of data set.
  };

  using DragAndDropFunction = std::function<void(const DragEvent&)>;
  using SourceFunction      = std::function<void(enum SourceEventType)>;

  /**
   * @brief Create an uninitialized DragAndDrop.
   *
   * this can be initialized with one of the derived DragAndDrop's New() methods
   */
  DragAndDrop();

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~DragAndDrop();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   */
  DragAndDrop(const DragAndDrop& handle) = default;

  /**
   * @brief Retrieve a handle to the DragAndDrop instance.
   *
   * @return A handle to the DragAndDrop
   */
  static DragAndDrop Get();

  /**
   * @brief Start the drag operation.
   *
   * @param[in] source The drag source object.
   * @param[in] shadowWindow The shadow window for drag object.
   * @param[in] dragData The data to send to target object.
   * @param[in] callback The drag source event callback.
   * @return bool true if the drag operation is started successfully.
   */
  bool StartDragAndDrop(Dali::Actor source, Dali::Window shadowWindow, const DragData& dragData, Dali::DragAndDrop::SourceFunction callback);

  /**
   * @brief Add the listener for receiving the drag and drop events.
   *
   * @param[in] target The drop target object.
   * @param[in] mimeType The mime type of target object.
   * @param[in] callback A drag and drop event callback.
   * @return bool true if the listener is added successfully.
   */
  bool AddListener(Dali::Actor target, char* mimeType, DragAndDropFunction callback);

  /**
   * @brief Remove the listener.
   *
   * @param[in] target The drop target object.
   * @return bool true if the listener is removed successfully.
   */
  bool RemoveListener(Dali::Actor target);

  /**
   * @brief Add the listener for receiving the drag and drop events.
   *
   * @param[in] target The drop target object.
   * @param[in] mimeType The mime type of target object.
   * @param[in] callback A drag and drop event callback.
   * @return bool true if the listener is added successfully.
   */
  bool AddListener(Dali::Window target, char* mimeType, DragAndDropFunction callback);

  /**
   * @brief Remove the listener.
   *
   * @param[in] target The drop target object.
   * @return bool true if the listener is removed successfully.
   */
  bool RemoveListener(Dali::Window target);

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
