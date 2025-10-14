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
#include <dali/public-api/common/dali-common.h> ///< For DALI_ASSERT_ALWAYS()
#include <memory>                               ///< For std::shared_ptr and std::unique_ptr

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-interface.h> ///< For Destroy()

namespace Dali::Internal::Adaptor
{
/**
 * Interface of graphics library handle.
 * It will hold ownership of Graphics::GraphicsInterface.
 * For dynamic backend case, load matched graphics library what we need, and hold library handle reference.
 * (See DynamicGraphicsLibraryHandle implements)
 * other (GLES only, or Vulkan only), do nothing.
 */
class GraphicsLibraryHandleBase
{
public:
  /**
   * Constructor.
   */
  GraphicsLibraryHandleBase() = default;

  /**
   * Virtual destructor.
   */
  virtual ~GraphicsLibraryHandleBase()
  {
    if(DALI_LIKELY(mGraphics))
    {
      mGraphics->Destroy();
      mGraphics.reset();
    }
  }

  /**
   * @brief Set the graphics interface to hold ownership.
   * @note Must call this function only one time per each lifecycle.
   * @param[in] graphics The graphics interface created by graphics factory.
   */
  void SetGraphicsInterface(std::unique_ptr<Dali::Graphics::GraphicsInterface> graphics)
  {
    DALI_ASSERT_ALWAYS(!mGraphics && "Graphics Should be nullptr");
    mGraphics = std::move(graphics);
  }

  /**
   * @brief Get the graphics interface what we hold ownership.
   * @return The graphics interface.
   */
  Dali::Graphics::GraphicsInterface& GetGraphicsInterface()
  {
    DALI_ASSERT_ALWAYS(mGraphics && "Graphics Should not be nullptr");
    return *mGraphics;
  }

  /**
   * @brief Get the graphics interface what we hold ownership.
   * @return The graphics interface.
   */
  const Dali::Graphics::GraphicsInterface& GraphicsInterface() const
  {
    DALI_ASSERT_ALWAYS(mGraphics && "Graphics Should not be nullptr");
    return *mGraphics;
  }

protected:
  std::unique_ptr<Dali::Graphics::GraphicsInterface> mGraphics;
};

using GraphicsLibraryHandlePtr = std::shared_ptr<GraphicsLibraryHandleBase>;

} // namespace Dali::Internal::Adaptor

#endif // DALI_INTERNAL_GRAPHICS_LIBRARY_HANDLE_H
