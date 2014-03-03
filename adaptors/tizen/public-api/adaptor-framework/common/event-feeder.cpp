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

// HEADER
#include <public-api/adaptor-framework/common/event-feeder.h>

// INTERNAL INCLUDES
#include <internal/common/adaptor-impl.h>

namespace Dali
{

namespace EventFeeder
{

void FeedTouchPoint( TouchPoint& point, int timeStamp )
{
  if ( Adaptor::IsAvailable() )
  {
    Internal::Adaptor::Adaptor::GetImplementation( Adaptor::Get() ).FeedTouchPoint( point, timeStamp );
  }
}

void FeedWheelEvent( MouseWheelEvent& wheelEvent )
{
  if ( Adaptor::IsAvailable() )
  {
    Internal::Adaptor::Adaptor::GetImplementation( Adaptor::Get() ).FeedWheelEvent( wheelEvent );
  }
}

void FeedKeyEvent( KeyEvent& keyEvent )
{
  if ( Adaptor::IsAvailable() )
  {
    Internal::Adaptor::Adaptor::GetImplementation( Adaptor::Get() ).FeedKeyEvent( keyEvent );
  }
}

} // namespace EventFeeder

} // namespace Dali
