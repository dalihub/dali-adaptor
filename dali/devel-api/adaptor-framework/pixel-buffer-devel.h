#ifndef DALI_ADAPTOR_PIXEL_BUFFER_DEVEL_H
#define DALI_ADAPTOR_PIXEL_BUFFER_DEVEL_H

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

// DevelPixelBuffer APIs have moved to dali-core or DevelPixelBufferOperations.
// This header preserves the old include path and namespace for source compatibility.
// Include <dali/devel-api/images/pixel-buffer-devel.h> for the dali-core APIs,
// or <dali/devel-api/adaptor-framework/pixel-buffer-operations-devel.h> for image operations.

// EXTERNAL INCLUDES
#include <dali/devel-api/images/pixel-buffer-devel.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer-operations-devel.h>

namespace DALI_NAMESPACE
{
namespace DevelPixelBuffer
{
using DevelPixelBufferOperations::ApplyCenterCrop;
using DevelPixelBufferOperations::ApplyLetterbox;
using DevelPixelBufferOperations::ApplyMask;
using DevelPixelBufferOperations::Crop;
using DevelPixelBufferOperations::GetBrightness;
using DevelPixelBufferOperations::MultiplyColorByAlpha;
using DevelPixelBufferOperations::Rotate;

} // namespace DevelPixelBuffer

} // namespace DALI_NAMESPACE

#endif // DALI_ADAPTOR_PIXEL_BUFFER_DEVEL_H
