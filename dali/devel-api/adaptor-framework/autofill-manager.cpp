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
#include <dali/devel-api/adaptor-framework/autofill-manager.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/autofill-manager-impl.h>

namespace Dali
{

AutofillManager::AutofillManager()
{
}

AutofillManager::~AutofillManager()
{
}

AutofillManager AutofillManager::Get()
{
  return Internal::Adaptor::AutofillManager::Get();
}

AutofillManager::AutofillManager(Internal::Adaptor::AutofillManager *impl)
: BaseHandle(impl)
{
}

/////////////////////////////////////////////// Autofill Item and Group ///////////////////////////////////////////////

Dali::AutofillItem AutofillManager::CreateAutofillItem( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool isSensitive )
{
  return Internal::Adaptor::GetImplementation(*this).CreateAutofillItem( id, label, hint, isSensitive );
}

Dali::AutofillGroup AutofillManager::CreateAutofillGroup( const std::string& groupId )
{
  return Internal::Adaptor::GetImplementation(*this).CreateAutofillGroup( groupId );
}


/////////////////////////////////////////////// Autofill Authentication Information ///////////////////////////////////////////////

bool AutofillManager::IsAutofillDataPresent() const
{
  return Internal::Adaptor::GetImplementation(*this).IsAutofillDataPresent();
}

bool AutofillManager::IsAuthenticationNeeded() const
{
  return Internal::Adaptor::GetImplementation(*this).IsAuthenticationNeeded();
}

const std::string& AutofillManager::GetAuthenticationServiceName() const
{
  return Internal::Adaptor::GetImplementation(*this).GetAuthenticationServiceName();
}

const std::string& AutofillManager::GetAuthenticationServiceMessage() const
{
  return Internal::Adaptor::GetImplementation(*this).GetAuthenticationServiceMessage();
}

const std::string& AutofillManager::GetAuthenticationServiceImagePath() const
{
  return Internal::Adaptor::GetImplementation(*this).GetAuthenticationServiceImagePath();
}

/////////////////////////////////////////////// Autofill Fill Response ///////////////////////////////////////////////

const std::string& AutofillManager::GetFillItemId() const
{
  return Internal::Adaptor::GetImplementation(*this).GetFillItemId();
}

const std::string& AutofillManager::GetFillItemPresentationText() const
{
  return Internal::Adaptor::GetImplementation(*this).GetFillItemPresentationText();
}

const std::string& AutofillManager::GetFillItemValue() const
{
  return Internal::Adaptor::GetImplementation(*this).GetFillItemValue();
}

void AutofillManager::SaveAutofillData( Dali::AutofillGroup group )
{
  Internal::Adaptor::GetImplementation(*this).SaveAutofillData( group );
}


// Signals

AutofillManager::AuthSignalType& AutofillManager::AuthenticationReceivedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).AuthenticationReceivedSignal();
}

AutofillManager::FillSignalType& AutofillManager::FillResponseReceivedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).FillResponseReceivedSignal();
}

AutofillManager::ListSignalType& AutofillManager::ListEventSignal()
{
  return Internal::Adaptor::GetImplementation(*this).ListEventSignal();
}


} // namespace Dali
