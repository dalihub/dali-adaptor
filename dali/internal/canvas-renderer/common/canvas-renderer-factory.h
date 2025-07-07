#ifndef DALI_INTERNAL_CANVAS_RENDERER_FACTORY_H
#define DALI_INTERNAL_CANVAS_RENDERER_FACTORY_H

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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer.h>
#include <dali/public-api/common/intrusive-ptr.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class CanvasRenderer;
typedef IntrusivePtr<CanvasRenderer> CanvasRendererPtr;

namespace CanvasRendererFactory
{
/**
 * @brief Creates new instance of CanvasRenderer implementation
 * @param[in] viewBox The viewBox of canvas.
 * @return pointer to CanvasRenderer implementation instance
 */
Dali::Internal::Adaptor::CanvasRendererPtr New(const Vector2& viewBox);

} // namespace CanvasRendererFactory

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_CANVAS_RENDERER_FACTORY_H
