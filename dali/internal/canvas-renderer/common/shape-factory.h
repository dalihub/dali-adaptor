#ifndef DALI_INTERNAL_SHAPE_FACTORY_H
#define DALI_INTERNAL_SHAPE_FACTORY_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/canvas-renderer/canvas-renderer-shape.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace ShapeFactory
{
/**
 * @brief Creates new instance of Shape implementation
 * @return pointer to Shape implementation instance
 */
Dali::Internal::Adaptor::Shape* New();

} // namespace ShapeFactory

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_SHAPE_FACTORY_H
