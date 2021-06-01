#ifndef DALI_INTERNAL_DRAWABLE_GROUP_FACTORY_H
#define DALI_INTERNAL_DRAWABLE_GROUP_FACTORY_H

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
#include <dali/devel-api/adaptor-framework/canvas-renderer-drawable-group.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace DrawableGroupFactory
{
/**
   * @brief Creates new instance of DrawableGroup implementation
   * @return pointer to DrawableGroup implementation instance
   */
Dali::Internal::Adaptor::DrawableGroup* New();

} // namespace DrawableGroupFactory

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_DRAWABLE_GROUP_FACTORY_H
