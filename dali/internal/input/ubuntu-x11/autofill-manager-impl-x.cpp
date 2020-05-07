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
#include <dali/internal/input/ubuntu-x11/autofill-manager-impl-x.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_AUTOFILL");
#endif

// Signals
const char* const SIGNAL_AUTHENTICATION_RECEIVED = "authenticationReceived";
const char* const SIGNAL_FILL_RESPONSE_RECEIVED = "fillResponseReceived";
const char* const SIGNAL_LIST_RECEIVED = "listReceived";

BaseHandle Create()
{
  return Dali::AutofillManager::Get();
}

Dali::TypeRegistration typeRegistration( typeid(Dali::AutofillManager), typeid(Dali::BaseHandle), Create );

Dali::SignalConnectorType signalConnector1( typeRegistration, SIGNAL_AUTHENTICATION_RECEIVED, Dali::Internal::Adaptor::AutofillManagerX::DoConnectSignal );
Dali::SignalConnectorType signalConnector2( typeRegistration, SIGNAL_FILL_RESPONSE_RECEIVED, Dali::Internal::Adaptor::AutofillManagerX::DoConnectSignal );
Dali::SignalConnectorType signalConnector3( typeRegistration, SIGNAL_LIST_RECEIVED, Dali::Internal::Adaptor::AutofillManagerX::DoConnectSignal );

} // unnamed namespace



Dali::AutofillManager AutofillManagerX::Get()
{
  Dali::AutofillManager autofill;
  AutofillManagerX *autofillPtr = NULL;

  Dali::SingletonService service( SingletonService::Get() );
  if( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::AutofillManager ) );
    if( handle )
    {
      // If so, downcast the handle
      autofillPtr = dynamic_cast< AutofillManagerX* >( handle.GetObjectPtr() );
      autofill = Dali::AutofillManager( autofillPtr );
    }
    else if( Adaptor::IsAvailable() )
    {
      // Create instance and register singleton only if the adaptor is available
      autofillPtr = new AutofillManagerX();
      autofill = Dali::AutofillManager( autofillPtr );
      service.Register( typeid( autofill ), autofill );
    }
  }

  return autofill;
}

AutofillManagerX::AutofillManagerX()
: mAutofillGroup(),
  mAuthenticationServiceName(""),
  mAuthenticationServiceMessage(""),
  mAuthenticationServiceImagePath(""),
  mFillItemId(""),
  mFillItemPresentationText(""),
  mFillItemValue(""),
  mIsDataPresent( false ),
  mIsAuthNeeded( false )
{
}

AutofillManagerX::~AutofillManagerX()
{
  DeleteContext();
}

void AutofillManagerX::ConnectCallbacks()
{
}

void AutofillManagerX::CreateContext()
{
}

void AutofillManagerX::DeleteContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerX::DeleteContext\n" );
  // Do Nothing
}


/////////////////////////////////////////////// Autofill Item and Group ///////////////////////////////////////////////

Dali::AutofillItem AutofillManagerX::CreateAutofillItem( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool isSensitive )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerX::CreateAutofillItem \n" );

  Dali::AutofillItem item = AutofillItem::New( id, label, hint, isSensitive );
  mAutofillItemList.push_back( item );

  return mAutofillItemList.back();
}

Dali::AutofillGroup AutofillManagerX::CreateAutofillGroup( const std::string& groupId )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerX::CreateAutofillGroup \n" );

  Dali::AutofillGroup group = AutofillGroup::New( groupId );
  mAutofillGroupList.push_back( group );

  return mAutofillGroupList.back();
}


/////////////////////////////////////////////// Autofill Authentication Information ///////////////////////////////////////////////

bool AutofillManagerX::IsAutofillDataPresent() const
{
  return mIsDataPresent;
}

bool AutofillManagerX::IsAuthenticationNeeded() const
{
  return mIsAuthNeeded;
}

const std::string& AutofillManagerX::GetAuthenticationServiceName() const
{
  return mAuthenticationServiceName;
}

const std::string& AutofillManagerX::GetAuthenticationServiceMessage() const
{
  return mAuthenticationServiceMessage;
}

const std::string& AutofillManagerX::GetAuthenticationServiceImagePath() const
{
  return mAuthenticationServiceImagePath;
}


/////////////////////////////////////////////// Autofill Fill Response ///////////////////////////////////////////////

const std::string& AutofillManagerX::GetFillItemId() const
{
  return mFillItemId;
}

const std::string& AutofillManagerX::GetFillItemPresentationText() const
{
  return mFillItemPresentationText;
}

const std::string& AutofillManagerX::GetFillItemValue() const
{
  return mFillItemValue;
}

void AutofillManagerX::SaveAutofillData( Dali::AutofillGroup group )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerX::SaveAutofillData\n" );
  // Do Nothing
}

// Signals
AutofillManagerX::AuthSignalType& AutofillManagerX::AuthenticationReceivedSignal()
{
  return mAuthReceivedSignal;
}

AutofillManagerX::FillSignalType& AutofillManagerX::FillResponseReceivedSignal()
{
  return mFillReceivedSignal;
}

AutofillManagerX::ListSignalType& AutofillManagerX::ListEventSignal()
{
  return mListReceivedSignal;
}

bool AutofillManagerX::DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor )
{
  Dali::BaseHandle handle( object );

  bool connected( true );
  AutofillManagerX* manager = dynamic_cast< AutofillManagerX* >( object );

  if( manager )
  {
    if( 0 == signalName.compare( SIGNAL_AUTHENTICATION_RECEIVED ) )
    {
      manager->AuthenticationReceivedSignal().Connect( tracker, functor );
    }
    else if( 0 == signalName.compare( SIGNAL_FILL_RESPONSE_RECEIVED ) )
    {
      manager->FillResponseReceivedSignal().Connect( tracker, functor );
    }
    else if( 0 == signalName.compare( SIGNAL_LIST_RECEIVED ) )
    {
      manager->ListEventSignal().Connect( tracker, functor );
    }
    else
    {
      // signalName does not match any signal
      connected = false;
    }
  }

  return connected;
}



} // Adaptor

} // Internal

} // Dali
