/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/common/egl-image-extensions.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-implementation.h>



namespace Dali
{

namespace Internal
{

namespace Adaptor
{

EglImageExtensions::EglImageExtensions(EglImplementation* eglImpl)
: mEglImplementation(eglImpl),
  mImageKHRInitialized(false),
  mImageKHRInitializeFailed(false)
{
  DALI_ASSERT_ALWAYS( eglImpl && "EGL Implementation not instantiated" );
}

EglImageExtensions::~EglImageExtensions()
{
}

void* EglImageExtensions::CreateImageKHR(EGLClientBuffer clientBuffer)
{
  DALI_LOG_ERROR(" does not support CreateImageKHR\n");
  return NULL;
}

void EglImageExtensions::DestroyImageKHR(void* eglImageKHR)
{
  DALI_LOG_ERROR(" does not support DestroyImageKHR\n");
}

void EglImageExtensions::TargetTextureKHR(void* eglImageKHR)
{
  DALI_LOG_ERROR(" does not support TargetTextureKHR\n");
}

void EglImageExtensions::InitializeEglImageKHR()
{
  DALI_LOG_ERROR(" does not support InitializeEglImageKHR\n");
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
