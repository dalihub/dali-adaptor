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
#include <dali/internal/window-system/common/orientation-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

Orientation* Orientation::New(Window* window)
{
  Orientation* orientation = new Orientation(window);

  return orientation;
}

Orientation::Orientation(Window* window)
: mWindow(window),
  mOrientation(0),
  mWindowWidth(0),
  mWindowHeight(0)
{
}

Orientation::~Orientation()
{
  // Note, there is only one orientation object that's owned by window,
  // so it will live longer than adaptor. (hence, no need to remove rotation observer)
}

int Orientation::GetDegrees() const
{
  return mOrientation;
}

float Orientation::GetRadians() const
{
  return Math::PI * float(mOrientation) / 180.0f;
}

Orientation::OrientationSignalType& Orientation::ChangedSignal()
{
  return mChangedSignal;
}

void Orientation::OnOrientationChange( const RotationEvent& rotation )
{
  mOrientation  = rotation.angle;
  mWindowWidth  = rotation.width;
  mWindowHeight = rotation.height;

  // Emit signal
  if( !mChangedSignal.Empty() )
  {
    Dali::Orientation handle( this );
    mChangedSignal.Emit( handle );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
