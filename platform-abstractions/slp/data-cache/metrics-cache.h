#ifndef DALI_SLP_PLATFORM_METRICS_CACHE_H
#define DALI_SLP_PLATFORM_METRICS_CACHE_H

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <dali/integration-api/glyph-set.h>
#include <string>

namespace Dali
{
namespace SlpPlatform
{
namespace MetricsCache
{

/**
 * Read from the metrics file into the global metrics parameter
 * @param[in] fontFamily The name of the font family
 * @param[in] fontStyle The name of the font style
 * @param[out] globalMetrics The data store to write into
 * @return true if the operation succeeded
 */
bool ReadGlobal( std::string fontFamily, std::string fontStyle, Integration::GlobalMetrics& globalMetrics);

/**
 * Write the global metrics parameter to the metrics file
 * @param[in] fontFamily The name of the font family
 * @param[in] fontStyle The name of the font style
 * @param[out] globalMetrics The data store to write
 */
void WriteGlobal( std::string fontFamily, std::string fontStyle, const Integration::GlobalMetrics& globalMetrics);

/**
 * Read the metrics from the given file into the supplied vector
 * @param[in] fontFamily The name of the font family
 * @param[in] fontStyle The name of the font style
 * @param[out] glyphMetricsContainer The vector of metrics to write
 * @return true if the operation succeeded
 */
bool Read( std::string fontFamily, std::string fontStyle, std::vector<Integration::GlyphMetrics>& glyphMetricsContainer );

/**
 * Write the metrics into the given file
 * @param[in] fontFamily The name of the font family
 * @param[in] fontStyle The name of the font style
 * @param[in] glyphSet The set of metrics to write
 */
void Write( std::string fontFamily, std::string fontStyle, const Integration::GlyphSet& glyphSet );

} // MetricsCache
} // SlpPlatform
} // Dali

#endif //DALI_INTEGRATION_METRICS_CACHE_H
