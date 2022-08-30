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
#include <dali/internal/window-system/windows/native-image-surface-impl-win.h>

using namespace Dali::Internal::Adaptor;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
NativeImageSurfaceWin::NativeImageSurfaceWin(Dali::NativeImageSourceQueuePtr queue)
{
}

NativeImageSurfaceWin::~NativeImageSurfaceWin()
{
}

Any NativeImageSurfaceWin::GetNativeRenderable()
{
  return Any();
}

void NativeImageSurfaceWin::InitializeGraphics()
{
}

void NativeImageSurfaceWin::TerminateGraphics()
{
}

void NativeImageSurfaceWin::PreRender()
{
}

void NativeImageSurfaceWin::PostRender()
{
}

bool NativeImageSurfaceWin::CanRender()
{
  return false;
}

bool NativeImageSurfaceWin::SetGraphicsConfig(bool depth, bool stencil, int msaa, int version)
{
  return false;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
