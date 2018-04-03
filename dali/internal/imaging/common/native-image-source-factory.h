#ifndef TIZEN_ORG_NATIVE_IMAGE_SOURCE_FACTORY_H
#define TIZEN_ORG_NATIVE_IMAGE_SOURCE_FACTORY_H

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <memory>
#include <dali/public-api/adaptor-framework/native-image-source.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class NativeImageSource;
namespace NativeImageSourceFactory
{

std::unique_ptr<Internal::Adaptor::NativeImageSource> New(unsigned int width,
                                       unsigned int height,
                                       Dali::NativeImageSource::ColorDepth depth,
                                       Any nativeImageSource);

}

} // Adaptor

} // Internal

} // Dali

#endif //TIZEN_ORG_NATIVE_IMAGE_SOURCE_FACTORY_H
