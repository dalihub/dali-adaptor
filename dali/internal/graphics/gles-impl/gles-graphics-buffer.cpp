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

// CLASS HEADER
#include "gles-graphics-buffer.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include "egl-graphics-controller.h"

namespace Dali::Graphics::GLES
{
Buffer::Buffer(const Graphics::BufferCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: BufferResource(createInfo, controller)
{
  controller.AddBuffer(*this);
}

bool Buffer::InitializeResource()
{
  auto gl = mController.GetGL();
  gl->GenBuffers(1, &mBufferId);
  gl->BindBuffer(GL_ARRAY_BUFFER, mBufferId);
  gl->BufferData(GL_ARRAY_BUFFER, mCreateInfo.size, nullptr, GL_STATIC_DRAW);
  return true;
}

void Buffer::DestroyResource()
{
  auto gl = mController.GetGL();
  gl->DeleteBuffers(1, &mBufferId);
}

void Buffer::DiscardResource()
{
  mController.DiscardResource(this);
}

void Buffer::Bind(Graphics::BufferUsage bindingTarget) const
{
  auto gl = mController.GetGL();
  switch(bindingTarget)
  {
    case Graphics::BufferUsage::VERTEX_BUFFER:
    {
      gl->BindBuffer(GL_ARRAY_BUFFER, mBufferId);
      break;
    }
    case Graphics::BufferUsage::INDEX_BUFFER:
    {
      gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId);
      break;
    }
    default:
    {
      //
    }
  }
}

} // namespace Dali::Graphics::GLES