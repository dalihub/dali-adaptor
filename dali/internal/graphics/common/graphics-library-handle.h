#ifndef DALI_INTERNAL_GRAPHICS_LIBRARY_HANDLE_H
#define DALI_INTERNAL_GRAPHICS_LIBRARY_HANDLE_H

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
#include <memory> ///< for std::shared_ptr

namespace Dali::Internal::Adaptor
{
/**
 * Interface of graphics library handle.
 * Only be used for dynamic graphics backend.
 */
class GraphicsLibraryHandleBase
{
public:
  /**
   * Constructor.
   */
  GraphicsLibraryHandleBase() = default;

  /**
   * Virtual destructor - no deletion through this interface.
   */
  virtual ~GraphicsLibraryHandleBase() = default;
};

using GraphicsLibraryHandlePtr = std::shared_ptr<GraphicsLibraryHandleBase>;

} // namespace Dali::Internal::Adaptor

#endif // DALI_INTERNAL_GRAPHICS_LIBRARY_HANDLE_H
