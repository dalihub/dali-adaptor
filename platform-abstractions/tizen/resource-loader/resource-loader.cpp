/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include "resource-loader.h"

// EXTERNAL HEADERS
#include <iostream>
#include <fstream>
#include <queue>
#include <cstring>
#include <dali/devel-api/common/map-wrapper.h>
#include <dali/devel-api/threading/mutex.h>

// INTERNAL HEADERS
#include <dali/integration-api/bitmap.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/devel-api/common/set-wrapper.h>
#include <dali/public-api/math/vector2.h>
#include "debug/resource-loader-debug.h"

using namespace Dali::Integration;

namespace Dali
{

namespace TizenPlatform
{

/********************************************************************************/
/****************************   RESOURCE LOADER METHODS  ************************/
/********************************************************************************/
ResourceLoader::ResourceLoader()
{
}

ResourceLoader::~ResourceLoader()
{
}

bool ResourceLoader::SaveFile(const std::string& filename, const unsigned char * buffer, unsigned int numBytes )
{
  DALI_LOG_TRACE_METHOD(gLoaderFilter);

  DALI_ASSERT_DEBUG( 0 != filename.length());

  bool result = false;

  std::filebuf buf;
  buf.open(filename.c_str(), std::ios::out | std::ios_base::trunc | std::ios::binary);
  if( buf.is_open() )
  {
    std::ostream stream(&buf);

    // determine size of buffer
    int length = static_cast<int>(numBytes);

    // write contents of buffer to the file
    stream.write(reinterpret_cast<const char*>(buffer), length);

    if( !stream.bad() )
    {
      DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader::SaveFile(%s) - wrote %d bytes\n", filename.c_str(), length);
      result = true;
    }
  }

#if defined(DEBUG_BUILD)
  if( !result )
  {
    DALI_LOG_INFO(gLoaderFilter, Debug::Verbose, "ResourceLoader::SaveFile(%s) - failed to load\n", filename.c_str());
  }
#endif

  return result;
}

} // namespace TizenPlatform

} // namespace Dali
