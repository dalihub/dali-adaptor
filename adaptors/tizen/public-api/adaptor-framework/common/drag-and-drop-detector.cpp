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

// CLASS HEADER
#include <dali/public-api/adaptor-framework/common/drag-and-drop-detector.h>

// INTERNAL INCLUDES
#include <internal/common/drag-and-drop-detector-impl.h>

namespace Dali
{

const char* const DragAndDropDetector::SIGNAL_ENTERED( "drag-and-drop-entered" );
const char* const DragAndDropDetector::SIGNAL_EXITED( "drag-and-drop-exited" );
const char* const DragAndDropDetector::SIGNAL_MOVED( "drag-and-drop-moved" );
const char* const DragAndDropDetector::SIGNAL_DROPPED( "drag-and-drop-dropped" );

DragAndDropDetector::DragAndDropDetector()
{
}

DragAndDropDetector::~DragAndDropDetector()
{
}

const std::string& DragAndDropDetector::GetContent() const
{
  return GetImplementation(*this).GetContent();
}

Vector2 DragAndDropDetector::GetCurrentScreenPosition() const
{
  return GetImplementation(*this).GetCurrentScreenPosition();
}

DragAndDropDetector::DragAndDropSignalV2& DragAndDropDetector::EnteredSignal()
{
  return GetImplementation(*this).EnteredSignal();
}

DragAndDropDetector::DragAndDropSignalV2& DragAndDropDetector::ExitedSignal()
{
  return GetImplementation(*this).ExitedSignal();
}

DragAndDropDetector::DragAndDropSignalV2& DragAndDropDetector::MovedSignal()
{
  return GetImplementation(*this).MovedSignal();
}

DragAndDropDetector::DragAndDropSignalV2& DragAndDropDetector::DroppedSignal()
{
  return GetImplementation(*this).DroppedSignal();
}

DragAndDropDetector::DragAndDropDetector( Internal::Adaptor::DragAndDropDetector* detector )
: BaseHandle( detector )
{
}

} // namespace Dali
