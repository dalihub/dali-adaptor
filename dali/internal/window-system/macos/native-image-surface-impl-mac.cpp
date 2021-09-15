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
#include <dali/internal/window-system/macos/native-image-surface-impl-mac.h>

using namespace Dali::Internal::Adaptor;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
NativeImageSurfaceCocoa::NativeImageSurfaceCocoa(Dali::NativeImageSourceQueuePtr queue)
{
}

NativeImageSurfaceCocoa::~NativeImageSurfaceCocoa()
{
}

Any NativeImageSurfaceCocoa::GetNativeRenderable()
{
  return Any();
}

void NativeImageSurfaceCocoa::InitializeGraphics()
{
}

void NativeImageSurfaceCocoa::TerminateGraphics()
{
}

void NativeImageSurfaceCocoa::PreRender()
{
}

void NativeImageSurfaceCocoa::PostRender()
{
}

bool NativeImageSurfaceCocoa::CanRender()
{
  return false;
}

bool NativeImageSurfaceCocoa::SetGraphicsConfig(bool depth, bool stencil, int msaa, int version)
{
  return false;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
