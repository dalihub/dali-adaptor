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
#include <dali/devel-api/adaptor-framework/autofill-group.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/autofill-group-impl.h>

namespace Dali
{

AutofillGroup::AutofillGroup()
{
}

AutofillGroup::AutofillGroup( Internal::Adaptor::AutofillGroup* internal )
: BaseHandle( internal )
{
}

AutofillGroup::~AutofillGroup()
{
}

AutofillGroup::AutofillGroup( const AutofillGroup& group )
: BaseHandle( group )
{
}

AutofillGroup& AutofillGroup::operator=( const AutofillGroup& group )
{
  if( *this != group )
  {
    BaseHandle::operator=( group );
  }
  return *this;
}

AutofillGroup AutofillGroup::DownCast( BaseHandle handle )
{
  return AutofillGroup( dynamic_cast< Internal::Adaptor::AutofillGroup* >( handle.GetObjectPtr() ) );
}

const std::string& AutofillGroup::GetId() const
{
  return Internal::Adaptor::GetImplementation(*this).GetId();
}

void AutofillGroup::AddAutofillItem( Dali::AutofillItem item )
{
  Internal::Adaptor::GetImplementation(*this).AddAutofillItem( item );
}

Dali::AutofillItem AutofillGroup::GetAutofillItem( const std::string& id )
{
  return Internal::Adaptor::GetImplementation(*this).GetAutofillItem( id );
}

void AutofillGroup::SaveAutofillData()
{
  Internal::Adaptor::GetImplementation(*this).SaveAutofillData();
}

void AutofillGroup::RequestAuthentication()
{
  Internal::Adaptor::GetImplementation(*this).RequestAuthentication();
}

void AutofillGroup::SendFillRequest()
{
  Internal::Adaptor::GetImplementation(*this).SendFillRequest();
}

} // namespace Dali
