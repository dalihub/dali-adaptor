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

#include "gles-graphics-shader.h"
#include <dali/integration-api/gl-abstraction.h>
#include <vector>
#include "egl-graphics-controller.h"

#include <GLES3/gl3.h>

namespace Dali
{
namespace Graphics
{
namespace GLES
{
Shader::Shader(const Graphics::ShaderCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: ShaderResource(createInfo, controller)
{
  if(mCreateInfo.sourceData && mCreateInfo.sourceSize)
  {
    printf("GLES::Shader: stage: %d, sourceMode: %d, size: %u, source:\n\n%s\n", (int)mCreateInfo.pipelineStage, (int)mCreateInfo.sourceMode, mCreateInfo.sourceSize, static_cast<const char*>(mCreateInfo.sourceData));
  }
}

} // namespace GLES
} // namespace Graphics
} // namespace Dali
