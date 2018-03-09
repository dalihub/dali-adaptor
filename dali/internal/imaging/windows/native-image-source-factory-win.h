#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_X_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_X_H

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

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-source-factory.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class NativeImageSourceFactoryWin : public NativeImageSourceFactory
{
public:

  std::unique_ptr< NativeImageSource > CreateNativeImageSource( unsigned int width, unsigned int height,
                                                                Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource ) override;

  std::unique_ptr< NativeImageSourceQueue > CreateNativeImageSourceQueue( unsigned int width, unsigned int height,
                                                                          Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue ) override;

};

} // Adaptor
} // Internal
} // Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_X_H
