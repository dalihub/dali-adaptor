/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/capture/capture.h>

// INTERNAL HEADER
#include <dali/internal/system/common/capture-impl.h>

namespace Dali
{

Capture::Capture()
{
}

Capture Capture::New()
{
  Internal::Adaptor::CapturePtr internal = Internal::Adaptor::Capture::New();

  return Capture( internal.Get() );
}

Capture Capture::New( Dali::CameraActor cameraActor )
{
  Internal::Adaptor::CapturePtr internal = Internal::Adaptor::Capture::New( cameraActor );

  return Capture( internal.Get() );
}

Capture Capture::DownCast( BaseHandle handle )
{
  return Capture( dynamic_cast< Internal::Adaptor::Capture* >( handle.GetObjectPtr() ) );
}

Capture::~Capture()
{
}

Capture::Capture( const Capture& copy )
: BaseHandle(copy)
{
}

Capture& Capture::operator=( const Capture& rhs )
{
  BaseHandle::operator=( rhs );
  return *this;
}

void Capture::Start( Actor source, const Vector2& size, const std::string &path, const Vector4& clearColor, const uint32_t quality )
{
  GetImpl( *this ).Start( source, size, path, clearColor, quality );
}

void Capture::Start( Actor source, const Vector2& size, const std::string &path, const Vector4& clearColor )
{
  GetImpl( *this ).Start( source, size, path, clearColor );
}

void Capture::Start( Actor source, const Vector2& size, const std::string &path )
{
  GetImpl( *this ).Start( source, size, path, Dali::Color::TRANSPARENT );
}

Dali::NativeImageSourcePtr Capture::GetNativeImageSource() const
{
  return GetImpl( *this ).GetNativeImageSource();
}

Capture::CaptureFinishedSignalType& Capture::FinishedSignal()
{
  return GetImpl( *this ).FinishedSignal();
}

Capture::Capture( Internal::Adaptor::Capture* internal )
: BaseHandle( internal )
{
}

} // namespace Dali
