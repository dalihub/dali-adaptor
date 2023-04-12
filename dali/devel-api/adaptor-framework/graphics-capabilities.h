#ifndef DALI_DEVEL_API_ADAPTOR_GRAPHICS_CAPABILITIES_H
#define DALI_DEVEL_API_ADAPTOR_GRAPHICS_CAPABILITIES_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

// EXTERNAL INCLUDES
#include <cstdint>

namespace Dali::GraphicsCapabilities
{
/**
 * @brief Get the total number of combined texture units that can be used by
 * all the shaders in a given program.
 *
 * @return the maximum number of texture units
 */
DALI_ADAPTOR_API uint32_t GetMaxCombinedTextureUnits();

} // namespace Dali::GraphicsCapabilities

#endif //DALI_DEVEL_API_ADAPTOR_GRAPHICS_CAPABILITIES_H
