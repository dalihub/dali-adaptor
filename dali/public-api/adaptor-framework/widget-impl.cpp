/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include "widget-impl.h"

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/internal/system/common/widget-controller.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

WidgetPtr Widget::New()
{
  return new Widget();
}

Widget::Widget()
: mImpl( nullptr )
{
}

Widget::~Widget()
{
  if( mImpl != nullptr )
  {
    delete mImpl;
  }
}

void Widget::OnCreate( const std::string& contentInfo, Dali::Window window )
{
}

void Widget::OnTerminate( const std::string& contentInfo, Dali::Widget::Termination type )
{
}

void Widget::OnPause()
{
}

void Widget::OnResume()
{
}

void Widget::OnResize( Dali::Window window )
{
}

void Widget::OnUpdate( const std::string& contentInfo, int force )
{
}

void Widget::SignalConnected( SlotObserver* slotObserver, CallbackBase* callback )
{
  mImpl->SignalConnected( slotObserver, callback );
}

void Widget::SignalDisconnected( SlotObserver* slotObserver, CallbackBase* callback )
{
  mImpl->SignalDisconnected( slotObserver, callback );
}

void Widget::SetContentInfo( const std::string& contentInfo )
{
  if( mImpl != nullptr )
  {
    mImpl->SetContentInfo( contentInfo );
  }
}

void Widget::SetImpl( Impl* impl )
{
  mImpl = impl;
}

Internal::Adaptor::Widget& GetImplementation(Dali::Widget& widget)
{
  DALI_ASSERT_ALWAYS(widget && "widget handle is empty");

  BaseObject& handle = widget.GetBaseObject();

  return static_cast<Internal::Adaptor::Widget&>(handle);
}

const Internal::Adaptor::Widget& GetImplementation(const Dali::Widget& widget)
{
  const BaseObject& handle = widget.GetBaseObject();

  return static_cast<const Internal::Adaptor::Widget&>(handle);
}

} // Adaptor

} // Internal

} // Dali
