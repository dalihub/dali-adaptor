#pragma once

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
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/graphics-backend.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali::Graphics::Internal
{
/**
 * @copydoc Dali::Graphics::GetCurrentGraphicsBackend()
 */
Dali::Graphics::Backend GetCurrentGraphicsBackend();

/**
 * @copydoc Dali::Graphics::SetGraphicsBackend()
 */
void SetGraphicsBackend(Dali::Graphics::Backend backend);

/**
 * @brief Get whether graphics backend setter called or not.
 * @return True if graphics backend setter called. False otherwise.
 */
bool IsGraphicsBackendSet();

/**
 * @brief Get whether we have to re-create graphics or not.
 * It will true if preference backend is not matched with user-side call, or environment values.
 * @return True if we have to re-create graphics. False otherwise.
 */
bool IsGraphicsResetRequired();

/**
 * @brief Mark as recreate graphics completed.
 */
void GraphicsResetCompleted();

/**
 * @brief Sets the preferred graphics backend. Should be called at PreInitialize case.
 * GetCurrentGraphicsBackend() return as preferenced backend.
 * @note If GraphicsBackend changed at application initialize time, IsGraphicsResetRequired() will return true.
 */
void SetPreferredGraphicsBackend(Backend preferenceBackend);

/**
 * @copydoc Dali::Graphics::GetBackendInformation()
 */
const std::string& GetBackendInformation();

/**
 * @brief To be called by the graphics backend to sets the backend information.
 * @param[in] backendInformation A string representation of the graphics backend.
 */
DALI_ADAPTOR_API void SetBackendInformation(std::string&& backendInformation);
} // namespace Dali::Graphics::Internal
