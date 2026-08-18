#ifndef DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_H
#define DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <dali/public-api/math/rect.h>

#include <string>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/// Platform-neutral representation of an application entity exposed as a View.
struct EntityData
{
  /// Annotation attached to the Actor that owns this entity.
  struct Annotation
  {
    std::string entityId;
    std::string entityType;
    std::string entityInfo;
  };

  std::string id;
  std::string type;
  std::string description;
  Dali::Bounds screenBounds{}; ///< Absolute screen coordinates.
  Dali::Bounds windowBounds{}; ///< Coordinates relative to the owning window.
  bool         isFocused{false};
  bool         isEnabled{false};
  Annotation   annotation;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_H
