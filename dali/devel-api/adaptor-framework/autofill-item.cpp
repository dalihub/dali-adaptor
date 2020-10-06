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
#include <dali/devel-api/adaptor-framework/autofill-item.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/autofill-item-impl.h>

namespace Dali
{

AutofillItem::AutofillItem()
{
}

AutofillItem::AutofillItem( Internal::Adaptor::AutofillItem* internal )
: BaseHandle( internal )
{
}

AutofillItem::~AutofillItem()
{
}

AutofillItem::AutofillItem( const AutofillItem& item )
: BaseHandle( item )
{
}

AutofillItem& AutofillItem::operator=( const AutofillItem& item )
{
  if( *this != item )
  {
    BaseHandle::operator=( item );
  }
  return *this;
}

AutofillItem AutofillItem::DownCast( BaseHandle handle )
{
  return AutofillItem( dynamic_cast< Internal::Adaptor::AutofillItem* >( handle.GetObjectPtr() ) );
}


const std::string& AutofillItem::GetId() const
{
  return Internal::Adaptor::GetImplementation(*this).GetId();
}

const std::string& AutofillItem::GetLabel() const
{
  return Internal::Adaptor::GetImplementation(*this).GetLabel();
}

Dali::AutofillItem::Hint AutofillItem::GetHint() const
{
  return Internal::Adaptor::GetImplementation(*this).GetHint();
}

bool AutofillItem::IsSensitiveData() const
{
  return Internal::Adaptor::GetImplementation(*this).IsSensitiveData();
}

void AutofillItem::SetSaveValue( const std::string& value )
{
  Internal::Adaptor::GetImplementation(*this).SetSaveValue( value );
}

const std::string& AutofillItem::GetSaveValue() const
{
  return Internal::Adaptor::GetImplementation(*this).GetSaveValue();
}

const std::string& AutofillItem::GetPresentationText( int index ) const
{
  return Internal::Adaptor::GetImplementation(*this).GetPresentationText( index );
}

const std::string& AutofillItem::GetFillValue( int index ) const
{
  return Internal::Adaptor::GetImplementation(*this).GetFillValue( index );
}

void AutofillItem::ClearPresentationTextList()
{
  Internal::Adaptor::GetImplementation(*this).ClearPresentationTextList();
}

void AutofillItem::ClearFillValueList()
{
  Internal::Adaptor::GetImplementation(*this).ClearFillValueList();
}

unsigned int AutofillItem::GetFillValueCount()
{
  return Internal::Adaptor::GetImplementation(*this).GetFillValueCount();
}

} // namespace Dali
