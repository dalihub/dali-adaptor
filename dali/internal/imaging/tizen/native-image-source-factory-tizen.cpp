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

// CLASS HEADER
#include <dali/internal/imaging/tizen/native-image-source-factory-tizen.h>

// INTERNAL HEADERS
#include <dali/internal/imaging/tizen/native-image-source-impl-tizen.h>
#include <dali/internal/imaging/tizen/native-image-source-queue-impl-tizen.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

std::unique_ptr< NativeImageSource > NativeImageSourceFactoryTizen::CreateNativeImageSource( unsigned int width, unsigned int height,
                                                                                             Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
{
  return std::unique_ptr< NativeImageSource >( NativeImageSourceTizen::New( width, height, depth, nativeImageSource ) );
}

std::unique_ptr< NativeImageSourceQueue > NativeImageSourceFactoryTizen::CreateNativeImageSourceQueue( unsigned int width, unsigned int height,
                                                                                                       Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue )
{
  return std::unique_ptr< NativeImageSourceQueue >( NativeImageSourceQueueTizen::New( width, height, depth, nativeImageSourceQueue ) );
}

// this should be created from somewhere
std::unique_ptr< NativeImageSourceFactory > GetNativeImageSourceFactory()
{
  // returns native image source factory
  return std::unique_ptr< NativeImageSourceFactoryTizen >( new NativeImageSourceFactoryTizen() );
}

} // Adaptor
} // Internal
} // Dali
