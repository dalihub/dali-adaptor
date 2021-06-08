#ifndef DALI_CAPTURE_DEVEL_H
#define DALI_CAPTURE_DEVEL_H

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

// INTERNAL INCLUDES
#include <dali/public-api/capture/capture.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

namespace Dali
{

namespace DevelCapture
{

/**
 * @brief Get PixelBuffer of captured image.
 *
 * @return PixelBuffer Captured result
 */
DALI_ADAPTOR_API Dali::Devel::PixelBuffer GetCapturedBuffer(Dali::Capture capture);

} // namespace DevelCapture

} // namespace Dali

#endif // DALI_CAPTURE_DEVEL_H
