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

// CLASS HEADER
#include "drag-and-drop-detector-impl.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

Dali::DragAndDropDetector DragAndDropDetector::New()
{
  Dali::DragAndDropDetector detector = Dali::DragAndDropDetector(new DragAndDropDetector());

  return detector;
}

const std::string& DragAndDropDetector::GetContent() const
{
  return mContent;
}

Vector2 DragAndDropDetector::GetCurrentScreenPosition() const
{
  return mScreenPosition;
}

bool DragAndDropDetector::IsEnabled() const
{
  return !mDroppedSignalV2.Empty() || !mEnteredSignalV2.Empty() || !mExitedSignalV2.Empty() || !mMovedSignalV2.Empty() ;
}

void DragAndDropDetector::SetContent( const std::string& content )
{
  mContent = content;
}

void DragAndDropDetector::ClearContent()
{
  mContent.clear();
}

void DragAndDropDetector::SetPosition( Vector2 screenPosition )
{
  mScreenPosition = screenPosition;
}

void DragAndDropDetector::EmitEnteredSignal()
{
  if ( !mEnteredSignalV2.Empty() )
  {
    Dali::DragAndDropDetector handle( this );
    mEnteredSignalV2.Emit( handle );
  }
}

void DragAndDropDetector::EmitExitedSignal()
{
  if ( !mExitedSignalV2.Empty() )
  {
    Dali::DragAndDropDetector handle( this );
    mExitedSignalV2.Emit( handle );
  }
}

void DragAndDropDetector::EmitMovedSignal()
{
  if ( !mMovedSignalV2.Empty() )
  {
    Dali::DragAndDropDetector handle( this );
    mMovedSignalV2.Emit( handle );
  }
}

void DragAndDropDetector::EmitDroppedSignal()
{
  if ( !mDroppedSignalV2.Empty() )
  {
    Dali::DragAndDropDetector handle( this );
    mDroppedSignalV2.Emit( handle );
  }
}

DragAndDropDetector::DragAndDropDetector()
: mContent(),
  mScreenPosition()
{
}

DragAndDropDetector::~DragAndDropDetector()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
