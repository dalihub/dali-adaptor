#ifndef DALI_INTERNAL_HEIF_DECODER_H
#define DALI_INTERNAL_HEIF_DECODER_H

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

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/loader-heic.h>
#include <vector>

namespace DALI_NAMESPACE
{
namespace TizenPlatform
{
namespace HeifDecoder
{
/** Read dimensions using the selected HEIF backend. */
bool ReadHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height);

/** Decode the primary image using the selected HEIF backend. */
bool Decode(const Dali::ImageLoader::Input& input, Dali::PixelBuffer& bitmap);

/** Decode the primary image into Y, U and V pixel buffers when supported. */
bool DecodePlanes(const Dali::ImageLoader::Input& input, std::vector<Dali::PixelBuffer>& pixelBuffers);
} // namespace HeifDecoder
} // namespace TizenPlatform
} // namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_HEIF_DECODER_H
