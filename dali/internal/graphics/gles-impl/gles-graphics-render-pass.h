#ifndef DALI_GRAPHICS_GLES_RENDER_PASS_H
#define DALI_GRAPHICS_GLES_RENDER_PASS_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/graphics-api/graphics-render-pass-create-info.h>
#include <dali/graphics-api/graphics-render-pass.h>

// INTERNAL INCLUDES
#include "gles-graphics-resource.h"

namespace Dali::Graphics::GLES
{
using RenderPassResource = Resource<Graphics::RenderPass, Graphics::RenderPassCreateInfo>;

class RenderPass : public RenderPassResource
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo Valid createInfo structure
   * @param[in] controller Reference to the controller
   */
  RenderPass(const Graphics::RenderPassCreateInfo& createInfo, Graphics::EglGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~RenderPass() override;

  /**
   * @brief Called when GL resources are destroyed
   */
  void DestroyResource() override
  {
    // There is no graphic resource here. do nothing.
  }

  /**
   * @brief Called when initializing the resource
   *
   * @return True on success
   */
  bool InitializeResource() override
  {
    // There is no graphic resource here. return true.
    return true;
  }

  /**
   * @brief Called when UniquePtr<> on client-side dies
   */
  void DiscardResource() override;

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl{nullptr};
};

} // namespace Dali::Graphics::GLES

#endif
