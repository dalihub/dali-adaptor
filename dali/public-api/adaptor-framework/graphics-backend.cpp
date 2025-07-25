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
#include <dali/public-api/adaptor-framework/graphics-backend.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-backend-impl.h>

namespace Dali::Graphics
{
namespace
{
Backend     gCurrentGraphicsBackend = Backend::DEFAULT;
std::string gBackendInformation;
} // namespace

Backend GetCurrentGraphicsBackend()
{
  return gCurrentGraphicsBackend;
}

void SetGraphicsBackend(Backend backend)
{
  static bool setOnce = false;
  if(!setOnce)
  {
    setOnce                 = true;
    gCurrentGraphicsBackend = backend;
  }
  else if(backend != gCurrentGraphicsBackend)
  {
    DALI_LOG_ERROR("Graphics backend already set to: %s\n", gCurrentGraphicsBackend == Backend::GLES ? "GLES" : "VULKAN");
  }
}

const std::string& GetBackendInformation()
{
  return Internal::GetBackendInformation();
}

} // namespace Dali::Graphics
