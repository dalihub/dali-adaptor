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

// CLASS HEADER
#include <dali/internal/canvas-renderer/generic/canvas-renderer-impl-generic.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace CanvasRendererFactory
{
Dali::Internal::Adaptor::CanvasRendererPtr New(const Vector2& viewBox)
{
#ifdef THORVG_SUPPORT
  return Dali::Internal::Adaptor::CanvasRendererGeneric::New(viewBox);
#else
  DALI_LOG_ERROR("CanvasRenderer is not supported!\n");
  return nullptr;
#endif
}

} // namespace CanvasRendererFactory

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
