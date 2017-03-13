#ifndef __DALI_EMSCRIPTEN_CALLBACKS_H__
#define __DALI_EMSCRIPTEN_CALLBACKS_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/platform-abstraction.h>
#include <dali/public-api/common/dali-common.h>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Glyph helper
 *
 * Allows Dali adaptor to request a glyph image from the browser
 *
 * @param[in] fontFamily The font family name
 * @param[in] fontStyle The font style name
 * @param[in] fontSize The font size
 * @param[in] character The character
 *
 */
Dali::Integration::BitmapPtr GetGlyphImage( const std::string& fontFamily, const std::string& fontStyle, float fontSize, uint32_t character );

/**
 * Image meta data helper
 *
 * Allows Dali adaptor to request image metadata from image known to the browser
 *
 * @param[in] filename The image name (possibly not directly a filename due to the browser sandboxing)
 * @param[in] size The image dimensions
 * @param[in] fittingMode The dali fitting mode
 * @param[in] samplingMode The dali sampling mode
 * @param[in] orientationCorrection The orientation correction
 *
 */
Dali::ImageDimensions LoadImageMetadata(const std::string filename,
                                        Dali::ImageDimensions& size,
                                        Dali::FittingMode::Type fittingMode,
                                        Dali::SamplingMode::Type samplingMode,
                                        bool orientationCorrection );

/**
 * Image helper
 *
 * Allows Dali adaptor to request an image from the browser
 *
 * @param[in] size The image dimensionsn
 * @param[in] fittingMode The dali fitting mode
 * @param[in] samplingMode The dali sampling mode
 * @param[in] orientationCorrection The orientation correction
 * @param[in] filename The image name (possibly not directly a filename due to the browser sandboxing)
 *
 */
Dali::Integration::BitmapPtr GetImage(const Dali::ImageDimensions& size,
                                      const Dali::FittingMode::Type& fittingMode,
                                      const Dali::SamplingMode::Type& samplingMode,
                                      const bool orientationCorrection,
                                      const std::string& filename );

/**
 * Debug statistics for the browser
 *
 */
struct Statistics
{
  bool on;
  float frameCount;

  float lastFrameDeltaSeconds;
  unsigned int lastSyncTimeMilliseconds;
  unsigned int nextSyncTimeMilliseconds;

  unsigned int keepUpdating; ///< A bitmask of KeepUpdating values
  bool needsNotification;
  float secondsFromLastFrame;

Statistics() :on(true),
    frameCount(0.0),
    lastFrameDeltaSeconds(0.0),
    lastSyncTimeMilliseconds(0.0),
    nextSyncTimeMilliseconds(0.0),
    keepUpdating(0),
    needsNotification(false),
    secondsFromLastFrame(0.0)
  {};
};

extern Statistics stats;

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
