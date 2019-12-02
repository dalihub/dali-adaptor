/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/android/native-image-source-factory-android.h>

// INTERNAL HEADERS
#include <dali/internal/imaging/android/native-image-source-impl-android.h>
#include <dali/internal/imaging/android/native-image-source-queue-impl-android.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

std::unique_ptr< NativeImageSource > NativeImageSourceFactoryAndroid::CreateNativeImageSource( uint32_t width, uint32_t height,
                                                                                         Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
{
  return std::unique_ptr< NativeImageSource >( NativeImageSourceAndroid::New( width, height, depth, nativeImageSource ) );
}

std::unique_ptr< NativeImageSourceQueue > NativeImageSourceFactoryAndroid::CreateNativeImageSourceQueue( uint32_t width, uint32_t height,
                                                                                                   Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue )
{
  return std::unique_ptr< NativeImageSourceQueue >( NativeImageSourceQueueAndroid::New( width, height, depth, nativeImageSourceQueue ) );
}

// this should be created from somewhere
std::unique_ptr< NativeImageSourceFactory > GetNativeImageSourceFactory()
{
  // returns native image source factory
  return std::unique_ptr< NativeImageSourceFactoryAndroid >( new NativeImageSourceFactoryAndroid() );
}

} // Adaptor
} // Internal
} // Dali
