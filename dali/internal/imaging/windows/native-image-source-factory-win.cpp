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
#include <dali/internal/imaging/windows/native-image-source-factory-win.h>

// INTERNAL HEADERS
#include <dali/internal/imaging/windows/native-image-source-impl-win.h>
#include <dali/internal/imaging/common/native-image-source-queue-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

std::unique_ptr< NativeImageSource > NativeImageSourceFactoryWin::CreateNativeImageSource( unsigned int width, unsigned int height,
                                                                                         Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
{
  return std::unique_ptr< NativeImageSource >( NativeImageSourceWin::New( width, height, depth, nativeImageSource ) );
}

std::unique_ptr< NativeImageSourceQueue > NativeImageSourceFactoryWin::CreateNativeImageSourceQueue( unsigned int width, unsigned int height,
                                                                                                   Dali::NativeImageSourceQueue::ColorDepth depth, Any nativeImageSourceQueue )
{
  return std::unique_ptr< NativeImageSourceQueue >( nullptr );
}

// this should be created from somewhere
std::unique_ptr< NativeImageSourceFactory > GetNativeImageSourceFactory()
{
  // returns native image source factory
  return std::unique_ptr< NativeImageSourceFactoryWin >( new NativeImageSourceFactoryWin() );
}

} // Adaptor
} // Internal
} // Dali
