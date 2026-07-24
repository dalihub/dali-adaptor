#ifndef DALI_INTERNAL_DRAG_AND_DROP_FACTORY_H
#define DALI_INTERNAL_DRAG_AND_DROP_FACTORY_H

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
#include <memory>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/drag-and-drop.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * @brief Factory interface for creating platform-specific DragAndDrop implementations.
 */
class DragAndDropFactory
{
public:
  DragAndDropFactory()          = default;
  virtual ~DragAndDropFactory() = default;

  /**
   * @brief Create or retrieve a DragAndDrop handle for the current platform.
   */
  virtual Dali::DragAndDrop CreateDragAndDrop() = 0;
};

/**
 * @brief Returns a platform-specific DragAndDrop factory.
 *
 * Implemented per backend (ecore_wl, tcore_wl, generic, ...).
 */
extern std::unique_ptr<DragAndDropFactory> GetDragAndDropFactory();

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_DRAG_AND_DROP_FACTORY_H
