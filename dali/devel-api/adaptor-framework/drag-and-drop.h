#ifndef DALI_DRAG_AND_DROP_H
#define DALI_DRAG_AND_DROP_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
} // namespace DALI_INTERNAL

/**
 * @brief Interface to the device's drag and drop.
 *
 * DragAndDrop class supports the drag and drop functionality for multi-window.
 */

class DALI_ADAPTOR_API DragAndDrop : public BaseHandle
{
public:
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
      this->mimeType = nullptr;
      this->data = nullptr;
    }
    DragEvent(DragType type, Dali::Vector2 position, const char* mimeType = nullptr, char* data = nullptr)
    {
      this->type     = type;
      this->position = position;
      this->mimeType = mimeType;
      this->data     = data;
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
    void SetMimeType(const char* mimeType)
    {
      this->mimeType = mimeType;
    }
    const char* GetMimeType()
    {
      return mimeType;
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
    const char*   mimeType;             ///< The mime type of drag object.
    char*         data{nullptr};        ///< The data of drag object.
  };

  /**
   * @brief Structure that contains information about the drag data information.
   */
  struct DragData
  {
     void SetMimeType(const char* mimeType)
     {
       this->mimeType = mimeType;
     }
     const char* GetMimeType() const
     {
       return mimeType;
     }
     void SetData(const char* data)
     {
       this->data = data;
     }
     const char* GetData() const
     {
       return data;
     }

  private:
     const char* mimeType{nullptr}; ///<The mime type of drag data.
     const char* data{nullptr};     ///<The drag data.
  };

  using DragAndDropFunction = std::function<void(const DragEvent&)>;

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
   * @param[in] shadow The shadow object for drag object.
   * @param[in] dragData The data to send to target object.
   * @return bool true if the drag operation is started successfully.
   */
  bool StartDragAndDrop(Dali::Actor source, Dali::Actor shadow, const DragData& dragData);

  /**
   * @brief Add the listener for receiving the drag and drop events.
   *
   * @param[in] target The drop target object.
   * @param[in] callback A drag and drop event callback.
   * @return bool true if the listener is added successfully.
   */
  bool AddListener(Dali::Actor target, DragAndDropFunction callback);

  /**
   * @brief Remove the listener.
   *
   * @param[in] target The drop target object.
   * @return bool true if the listener is removed successfully.
   */
  bool RemoveListener(Dali::Actor target);

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
