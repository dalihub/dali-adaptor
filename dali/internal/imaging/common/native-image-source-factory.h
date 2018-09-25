#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_H

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

// EXTERNAL INCLUDES
#include <memory>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class NativeImageSource;
class NativeImageSourceQueue;

class NativeImageSourceFactory
{
public:

  NativeImageSourceFactory() = default;
  virtual ~NativeImageSourceFactory() = default;

  virtual std::unique_ptr< NativeImageSource > CreateNativeImageSource( unsigned int width, unsigned int height,
                                                                        Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource ) = 0;

  virtual std::unique_ptr< NativeImageSourceQueue > CreateNativeImageSourceQueue( unsigned int width, unsigned int height,
                                                                                  Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue ) = 0;

};

extern std::unique_ptr< NativeImageSourceFactory > GetNativeImageSourceFactory();

} // Adaptor
} // Internal
} // Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_H
