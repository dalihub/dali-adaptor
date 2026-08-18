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

#ifndef DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_HOST_H
#define DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_HOST_H

#include <dali/internal/app-entity/common/entity-data-service.h>

#include <memory>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class EntityDataHost
{
public:
  virtual ~EntityDataHost() = default;
  virtual EntityDataService& GetService() = 0;
  virtual bool IsListening() const = 0;
};

// Creates the platform backend for entity-data actions. Common code uses this
// factory instead of including a platform-specific backend directly. Returns
// nullptr when the current platform does not support the entity-data service.
extern DALI_ADAPTOR_API std::unique_ptr<EntityDataHost> CreateEntityDataHost(Dali::Integration::FocusedActorProvider* provider = nullptr);

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_HOST_H
