#ifndef DALI_CAPTURE_DEVEL_H
#define DALI_CAPTURE_DEVEL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/rendering/texture.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/capture.h>

namespace DALI_NAMESPACE
{

namespace DevelCapture
{

/**
 * @brief Gets the texture the capture was rendered into.
 *
 * The capture result stays in this texture on the GPU, so it can be rendered
 * again without reading the pixels back.
 *
 * @param[in] capture The capture
 * @return Texture of the captured result
 * @note GetTexture is only available inside FinishedSignal.
 */
DALI_ADAPTOR_API Dali::Texture GetTexture(Dali::Capture capture);

} // namespace DevelCapture

} //namespace DALI_NAMESPACE

#endif // DALI_CAPTURE_DEVEL_H
