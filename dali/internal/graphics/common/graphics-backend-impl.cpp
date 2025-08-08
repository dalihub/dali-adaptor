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

// HEADER
#include <dali/internal/graphics/common/graphics-backend-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-factory.h> ///< for Dali::Internal::Adaptor::GetCurrentGraphicsLibraryBackend()

namespace Dali::Graphics::Internal
{
namespace
{
Dali::Graphics::Backend gCurrentGraphicsBackend = Dali::Graphics::Backend::DEFAULT;
std::string             gBackendInformation;

enum BackendState
{
  UNSET,
  PREFERRED,
  FIXED,
  RESET_GRAPHICS_REQUIRED,
};
BackendState gBackendState = BackendState::UNSET;

static constexpr std::string_view GRAPHICS_BACKEND_NAME[] = {"GLES", "VULKAN"};
static constexpr std::string_view BACKEND_STATE_NAME[]    = {"UNSET", "PREFERRED", "FIXED", "RESET_GRAPHICS_REQUIRED"};
} // namespace

Backend GetCurrentGraphicsBackend()
{
  DALI_LOG_DEBUG_INFO("Current backend : %s state : %s\n", GRAPHICS_BACKEND_NAME[static_cast<int>(gCurrentGraphicsBackend)].data(), BACKEND_STATE_NAME[gBackendState].data());
  return gCurrentGraphicsBackend;
}

void SetGraphicsBackend(Backend backend)
{
  DALI_LOG_DEBUG_INFO("current state: %s\n", BACKEND_STATE_NAME[gBackendState].data());
  if(!IsGraphicsBackendSet())
  {
    gCurrentGraphicsBackend = backend;
    if(DALI_UNLIKELY(gBackendState == BackendState::PREFERRED && gCurrentGraphicsBackend != Dali::Internal::Adaptor::GetCurrentGraphicsLibraryBackend()))
    {
      gBackendState = BackendState::RESET_GRAPHICS_REQUIRED;
      DALI_LOG_DEBUG_INFO("Graphics backend not matched as PREFERRED! Reset loaded graphics library to: %s\n", GRAPHICS_BACKEND_NAME[static_cast<int>(gCurrentGraphicsBackend)].data());
    }
    else
    {
      gBackendState = BackendState::FIXED;
      DALI_LOG_DEBUG_INFO("Graphics backend set to: %s\n", GRAPHICS_BACKEND_NAME[static_cast<int>(gCurrentGraphicsBackend)].data());
    }
  }
  else if(backend != gCurrentGraphicsBackend)
  {
    DALI_LOG_ERROR("Graphics backend already set to: %s\n", GRAPHICS_BACKEND_NAME[static_cast<int>(gCurrentGraphicsBackend)].data());
  }
  DALI_LOG_DEBUG_INFO("changed state: %s\n", BACKEND_STATE_NAME[gBackendState].data());
}

bool IsGraphicsBackendSet()
{
  return gBackendState == BackendState::FIXED || gBackendState == BackendState::RESET_GRAPHICS_REQUIRED;
}

bool IsGraphicsResetRequired()
{
  return gBackendState == BackendState::RESET_GRAPHICS_REQUIRED;
}

void GraphicsResetCompleted()
{
  if(gBackendState == BackendState::RESET_GRAPHICS_REQUIRED)
  {
    DALI_LOG_DEBUG_INFO("state: %s -> %s\n", BACKEND_STATE_NAME[gBackendState].data(), BACKEND_STATE_NAME[BackendState::FIXED].data());
    gBackendState = BackendState::FIXED;
  }
}

void SetPreferredGraphicsBackend(Backend backend)
{
  DALI_LOG_DEBUG_INFO("current state: %s\n", BACKEND_STATE_NAME[gBackendState].data());
  if(!IsGraphicsBackendSet())
  {
    gBackendState           = BackendState::PREFERRED;
    gCurrentGraphicsBackend = backend;
    DALI_LOG_DEBUG_INFO("Preference Graphics backend set to: %s\n", GRAPHICS_BACKEND_NAME[static_cast<int>(gCurrentGraphicsBackend)].data());
  }
  else if(backend != gCurrentGraphicsBackend)
  {
    DALI_LOG_ERROR("Graphics backend already set to: %s\n", GRAPHICS_BACKEND_NAME[static_cast<int>(gCurrentGraphicsBackend)].data());
  }
  DALI_LOG_DEBUG_INFO("changed state: %s\n", BACKEND_STATE_NAME[gBackendState].data());
}

const std::string& GetBackendInformation()
{
  return gBackendInformation;
}

void SetBackendInformation(std::string&& backendInformation)
{
  gBackendInformation = std::move(backendInformation);
}

} // namespace Dali::Graphics::Internal
