#ifndef DALI_INTERNAL_ADAPTOR_ALPHA_MASK_H
#define DALI_INTERNAL_ADAPTOR_ALPHA_MASK_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * Apply the mask to a buffer's alpha channel
 * @param[in] buffer The buffer to apply the mask to
 * @param[in] mask The mask to apply
 */
void ApplyMaskToAlphaChannel( PixelBuffer& buffer, const PixelBuffer& mask );

/**
 * Create a new PixelBuffer with an alpha channel large enough to handle the alpha from
 * the mask, converting the color values to the new size, and either multiplying the mask's
 * alpha into the existing alpha value, or writing the mask's alpha value directly into
 * the new buffer's alpha channel.
 *
 * @param[in] buffer The buffer to apply the mask to
 * @param[in] mask The mask to apply
 * @return A new pixel buffer containing the masked image
 */
PixelBufferPtr CreateNewMaskedBuffer( const PixelBuffer& buffer, const PixelBuffer& mask );

} //namespace Adaptor
} //namespace Internal
} //namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_ALPHA_MASK_H
