/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/integration-api/native-render-surface.h>
#include <dali/internal/window-system/common/render-surface-factory.h>

namespace Dali
{

DALI_ADAPTOR_API NativeRenderSurface* CreateNativeSurface( PositionSize positionSize, bool isTransparent )
{
  auto renderSurfaceFactory = Dali::Internal::Adaptor::GetRenderSurfaceFactory();
  auto nativeRenderSurface =  renderSurfaceFactory->CreateNativeRenderSurface( positionSize, isTransparent );
  return nativeRenderSurface.release();
}

} // namespace Dali
