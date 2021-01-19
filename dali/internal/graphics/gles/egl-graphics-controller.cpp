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
 */

// CLASS HEADER
#include <dali/internal/graphics/gles/egl-graphics-controller.h>

// INTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>

namespace Dali
{
namespace Graphics
{
void EglGraphicsController::InitializeGLES(Integration::GlAbstraction& glAbstraction)
{
  mGlAbstraction = &glAbstraction;
}

void EglGraphicsController::Initialize(Integration::GlSyncAbstraction&          glSyncAbstraction,
                                       Integration::GlContextHelperAbstraction& glContextHelperAbstraction)
{
  mGlSyncAbstraction          = &glSyncAbstraction;
  mGlContextHelperAbstraction = &glContextHelperAbstraction;
}

Integration::GlAbstraction& EglGraphicsController::GetGlAbstraction()
{
  DALI_ASSERT_DEBUG(mGlAbstraction && "Graphics controller not initialized");
  return *mGlAbstraction;
}

Integration::GlSyncAbstraction& EglGraphicsController::GetGlSyncAbstraction()
{
  DALI_ASSERT_DEBUG(mGlSyncAbstraction && "Graphics controller not initialized");
  return *mGlSyncAbstraction;
}

Integration::GlContextHelperAbstraction& EglGraphicsController::GetGlContextHelperAbstraction()
{
  DALI_ASSERT_DEBUG(mGlContextHelperAbstraction && "Graphics controller not initialized");
  return *mGlContextHelperAbstraction;
}

} // namespace Graphics
} // namespace Dali
