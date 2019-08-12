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
#include <dali/internal/input/tizen-wayland/autofill-manager-impl-ecore-wl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/singleton-service-impl.h>


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


#ifdef CAPI_AUTOFILL_SUPPORT

// All methods in this range are Static function calls used by ecore 'c' style callback registration
static void ConnectionStatusChangedCallback( autofill_h autofillHandle, autofill_connection_status_e status, void *user_data )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::ConnectionStatusChangedCallback mAutofillHandle : %p \n", autofillHandle );

  switch( status )
  {
    case AUTOFILL_CONNECTION_STATUS_CONNECTED:
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "Autofill Connected.\n" );
      break;
    }
    case AUTOFILL_CONNECTION_STATUS_DISCONNECTED:
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "Autofill Disconnected.\n" );
      break;
    }
    case AUTOFILL_CONNECTION_STATUS_REJECTED:
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "Autofill Rejected.\n" );
      break;
    }
    default:
    {
      // Do nothing
      break;
    }
  }
}

// Callback to receive the authentication information.
static void AuthInfoCallback( autofill_h ah, autofill_auth_info_h authInfoHandle, void *data )
{
  Dali::AutofillManager autofill = AutofillManager::Get();
  Internal::Adaptor::AutofillManager& autofillImpl = Internal::Adaptor::GetImplementation( autofill );
  Internal::Adaptor::AutofillManagerEcoreWl& autofillImplWl = static_cast<Internal::Adaptor::AutofillManagerEcoreWl&>( autofillImpl );
  autofillImplWl.ReceiveAuthInfo( authInfoHandle, data );
}

// If there's an only one fill response group, then this callback is called.
static bool FillResponseItemCallback( autofill_fill_response_item_h itemHandle, void *userData )
{
  Dali::AutofillManager autofill = AutofillManager::Get();
  Internal::Adaptor::AutofillManager& autofillImpl = Internal::Adaptor::GetImplementation( autofill );
  Internal::Adaptor::AutofillManagerEcoreWl& autofillImplWl = static_cast<Internal::Adaptor::AutofillManagerEcoreWl&>( autofillImpl );
  autofillImplWl.FillGroupItem( itemHandle, userData ); // Implementation here
  return true;
}

// If the fill response groups are multiple, then this callback is called.
static bool FillResponseMultipleItemCallback( autofill_fill_response_item_h itemHandle, void *userData )
{
  Dali::AutofillManager autofill = AutofillManager::Get();
  Internal::Adaptor::AutofillManager& autofillImpl = Internal::Adaptor::GetImplementation( autofill );
  Internal::Adaptor::AutofillManagerEcoreWl& autofillImplWl = static_cast<Internal::Adaptor::AutofillManagerEcoreWl&>( autofillImpl );
  autofillImplWl.FillMultipleGroupItem( itemHandle, userData ); // Implementation here
  return true;
}

// This callback is called according to the number of pairs to fill out.
static bool FillResponseGroupCallback( autofill_fill_response_group_h groupHandle, void *userData )
{
  int* count = static_cast<int*>(userData);

  // According to the number of group count, Retrieves all fill response items of each fill response group.
  if( *count == 1 )
  {
    autofill_fill_response_group_foreach_item( groupHandle, FillResponseItemCallback, NULL );
  }
  else if( *count > 1 )
  {
    autofill_fill_response_group_foreach_item( groupHandle, FillResponseMultipleItemCallback, groupHandle );
  }

  return true;
}

// Callback to receive autofill fill response.
static void FillResponseCallback( autofill_h autofillHandle, autofill_fill_response_h fillResponseHandle, void *data )
{
  if( !fillResponseHandle )
  {
    DALI_LOG_ERROR("Fill response handle is empty. \n");
    return;
  }

  // Get fill response group count
  int count = 0;
  autofill_fill_response_get_group_count( fillResponseHandle, &count );
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::FillResponseCallback group count : %d \n", count );

  // Retrieves all groups of each fill response.
  autofill_fill_response_foreach_group( fillResponseHandle, FillResponseGroupCallback, &count );

  if( count > 1 )
  {
    // Emits the signal to make a list of multiple data.
    Dali::AutofillManager autofill = AutofillManager::Get();
    autofill.ListEventSignal().Emit();
  }
}
#endif // CAPI_AUTOFILL_SUPPORT

BaseHandle Create()
{
  return Dali::AutofillManager::Get();
}

Dali::TypeRegistration typeRegistration( typeid(Dali::AutofillManager), typeid(Dali::BaseHandle), Create );

Dali::SignalConnectorType signalConnector1( typeRegistration, SIGNAL_AUTHENTICATION_RECEIVED, Dali::Internal::Adaptor::AutofillManagerEcoreWl::DoConnectSignal );
Dali::SignalConnectorType signalConnector2( typeRegistration, SIGNAL_FILL_RESPONSE_RECEIVED, Dali::Internal::Adaptor::AutofillManagerEcoreWl::DoConnectSignal );
Dali::SignalConnectorType signalConnector3( typeRegistration, SIGNAL_LIST_RECEIVED, Dali::Internal::Adaptor::AutofillManagerEcoreWl::DoConnectSignal );

} // unnamed namespace



Dali::AutofillManager AutofillManagerEcoreWl::Get()
{
  Dali::AutofillManager autofill;
  AutofillManagerEcoreWl *autofillPtr = NULL;

  Dali::SingletonService service( SingletonService::Get() );
  if( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::AutofillManager ) );
    if( handle )
    {
      // If so, downcast the handle
      autofillPtr = dynamic_cast< AutofillManagerEcoreWl* >( handle.GetObjectPtr() );
      autofill = Dali::AutofillManager( autofillPtr );
    }
    else if( Adaptor::IsAvailable() )
    {
      // Create instance and register singleton only if the adaptor is available
      autofillPtr = new AutofillManagerEcoreWl();
      autofill = Dali::AutofillManager( autofillPtr );
      service.Register( typeid( autofill ), autofill );

      // Connect Autofill daemon at the first time
      autofillPtr->CreateContext();
      autofillPtr->ConnectCallbacks();
    }
  }

  return autofill;
}

AutofillManagerEcoreWl::AutofillManagerEcoreWl()
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
#ifdef CAPI_AUTOFILL_SUPPORT
  mAutofillHandle =  NULL;
#endif // CAPI_AUTOFILL_SUPPORT
}

AutofillManagerEcoreWl::~AutofillManagerEcoreWl()
{
  DeleteContext();
}

void AutofillManagerEcoreWl::CreateContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::CreateContext\n" );

#ifdef CAPI_AUTOFILL_SUPPORT
  int ret = autofill_create( &mAutofillHandle );
  if( ret != AUTOFILL_ERROR_NONE )
  {
    DALI_LOG_ERROR( "Failed to create autofill handle : %d \n", ret );
  }
#endif // CAPI_AUTOFILL_SUPPORT
}

void AutofillManagerEcoreWl::DeleteContext()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::DeleteContext\n" );
#ifdef CAPI_AUTOFILL_SUPPORT
  if( mAutofillHandle )
  {
    // Unsets the callback to receive the authentication information.
    autofill_auth_info_unset_received_cb( mAutofillHandle );

    autofill_destroy( mAutofillHandle );
    mAutofillHandle = NULL;
  }
#endif // CAPI_AUTOFILL_SUPPORT
}

// Callbacks for connecting to autofill daemon.
void AutofillManagerEcoreWl::ConnectCallbacks()
{
#ifdef CAPI_AUTOFILL_SUPPORT
  if( mAutofillHandle )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::ConnectCallbacks\n" );

    int ret = autofill_connect( mAutofillHandle, ConnectionStatusChangedCallback, NULL );
    if( ret != AUTOFILL_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Failed to connect : %d \n", ret );
    }

    // Sets the callback to receive the authentication information.
    autofill_auth_info_set_received_cb( mAutofillHandle, AuthInfoCallback, NULL );

    // Sets the callback to receive autofill fill response.
    autofill_fill_response_set_received_cb( mAutofillHandle, FillResponseCallback, NULL );
  }
#endif // CAPI_AUTOFILL_SUPPORT
}


/////////////////////////////////////////////// Autofill Callback implementation ///////////////////////////////////////////////

#ifdef CAPI_AUTOFILL_SUPPORT

autofill_h AutofillManagerEcoreWl::GetAutofillHandle()
{
  return mAutofillHandle;
}

// Implementation to receive the authentication information.
void AutofillManagerEcoreWl::ReceiveAuthInfo( autofill_auth_info_h authInfoHandle, void *data )
{
  bool autofillDataPresent = false;
  bool authenticationNeeded = false;
  char* serviceName = NULL;
  char* serviceMessage = NULL;
  char* serviceLogoImagePath = NULL;
  char* groupId = NULL;

  // Gets the authentication information which is set by Autofill Service framework.
  autofill_auth_info_get_view_id( authInfoHandle, &groupId );
  autofill_auth_info_get_autofill_data_present( authInfoHandle, &autofillDataPresent );
  autofill_auth_info_get_authentication_needed( authInfoHandle, &authenticationNeeded );

  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::ReceiveAuthInfo group id : %s, Is autofill data present ? : %s, Is authentication needed ? : %s \n",
                                              groupId, autofillDataPresent ? "true" : "false", authenticationNeeded ? "true" : "false" );

  for( std::vector<Dali::AutofillGroup>::iterator iter = mAutofillGroupList.begin(), endIter = mAutofillGroupList.end(); iter != endIter; ++iter )
  {
    const std::string id = ( *iter ).GetId();
    if( id.compare( groupId ) == 0 )
    {
      mAutofillGroup = ( *iter );
      break;
    }
  }
  // Sets the 'autofill data present' and 'authentication needed' attributes in autofill authentication information.
  mIsDataPresent = autofillDataPresent;
  mIsAuthNeeded = authenticationNeeded;

  if( groupId )
  {
    free( groupId );
  }

  if( !autofillDataPresent )
  {
    DALI_LOG_ERROR( " -> The autofill data is not present now. \n" );
    return;
  }

  // If autofill authentication is needed, get authentication service information and set to DALi member variables.
  if( authenticationNeeded )
  {
    // Gets the authentication service information which is set by Autofill Service framework.
    autofill_auth_info_get_service_name( authInfoHandle, &serviceName );
    autofill_auth_info_get_service_message( authInfoHandle, &serviceMessage );
    autofill_auth_info_get_service_logo_image_path( authInfoHandle, &serviceLogoImagePath );

    DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::ReceiveAuthInfo service name : %s, logo path : %s, message : '%s' \n",
                                                serviceName, serviceLogoImagePath, serviceMessage );

    // Sets the authentication service information in order to use in other components.
    if( serviceName )
    {
      mAuthenticationServiceName = serviceName;
      free( serviceName );
    }

    if( serviceMessage )
    {
      mAuthenticationServiceMessage = serviceMessage;
      free( serviceMessage );
    }

    if( serviceLogoImagePath )
    {
      mAuthenticationServiceImagePath = serviceLogoImagePath;
      free( serviceLogoImagePath );
    }

    // Emits the signal to receive the authentication information.
    mAuthReceivedSignal.Emit();
  }
  else
  {
    // If Authentication is not needed, sends fill request directly to fill the data.
    mAutofillGroup.SendFillRequest();
  }
}

// Implementation to fill out the data
void AutofillManagerEcoreWl::FillGroupItem( autofill_fill_response_item_h itemHandle, void *userData )
{
  char* id = NULL;
  char* value = NULL;
  char* presentationText = NULL;

  // Gets the fill response information which is set by Autofill Service framework.
  autofill_fill_response_item_get_id( itemHandle, &id );
  autofill_fill_response_item_get_presentation_text( itemHandle, &presentationText );
  autofill_fill_response_item_get_value( itemHandle, &value );

  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::FillResponseItemCallback item id : %s, value : %s, presentation text : %s\n",
                                              id, value, presentationText );

  // Sets the fill response information in order to use in other components.
  if( id )
  {
    mFillItemId = id;
    free( id );
  }

  if( presentationText )
  {
    mFillItemPresentationText = presentationText;
    free( presentationText );
  }

  if( value )
  {
    mFillItemValue = value;
    free( value );
  }

  Dali::AutofillItem item = mAutofillGroup.GetAutofillItem( mFillItemId );
  Internal::Adaptor::AutofillItem& itemImpl = Internal::Adaptor::GetImplementation( item );
  itemImpl.AddPresentationList( mFillItemPresentationText );
  itemImpl.AddFillValueList( mFillItemValue );

  // Emits the signal to fill the data in text input field.
  mFillReceivedSignal.Emit( item );

}

// Implementation to fill out the data when the group count is more than one.
void AutofillManagerEcoreWl::FillMultipleGroupItem( autofill_fill_response_item_h itemHandle, void *userData )
{
  char* id = NULL;
  char* value = NULL;
  char* presentationText = NULL;

  // Gets the fill response information which is set by Autofill Service framework.
  autofill_fill_response_item_get_id( itemHandle, &id );
  autofill_fill_response_item_get_presentation_text( itemHandle, &presentationText );
  autofill_fill_response_item_get_value( itemHandle, &value );

  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::FillResponseMultipleItemCallback item id : %s, value : %s, presentation text : %s\n",
                                              id, value, presentationText );

  // Sets the fill response information in order to use in other components.
  if( id )
  {
    mFillItemId = id;
    free( id );
  }

  if( presentationText )
  {
    mFillItemPresentationText = presentationText;
    free( presentationText );
  }

  if( value )
  {
    mFillItemValue = value;
    free( value );
  }

  Dali::AutofillItem item = mAutofillGroup.GetAutofillItem( mFillItemId );
  Internal::Adaptor::AutofillItem& itemImpl = Internal::Adaptor::GetImplementation( item );
  itemImpl.AddPresentationList( mFillItemPresentationText );
  itemImpl.AddFillValueList( mFillItemValue );

}
#endif // CAPI_AUTOFILL_SUPPORT


/////////////////////////////////////////////// Autofill Item and Group ///////////////////////////////////////////////

Dali::AutofillItem AutofillManagerEcoreWl::CreateAutofillItem( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool isSensitive )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::CreateAutofillItem \n" );

  Dali::AutofillItem item = AutofillItem::New( id, label, hint, isSensitive );
  mAutofillItemList.push_back( item );

  return mAutofillItemList.back();
}

Dali::AutofillGroup AutofillManagerEcoreWl::CreateAutofillGroup( const std::string& groupId )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::CreateAutofillGroup \n" );

  Dali::AutofillGroup group = AutofillGroup::New( groupId );
  mAutofillGroupList.push_back( group );

  return mAutofillGroupList.back();
}


/////////////////////////////////////////////// Autofill Authentication Information ///////////////////////////////////////////////

bool AutofillManagerEcoreWl::IsAutofillDataPresent() const
{
  return mIsDataPresent;
}

bool AutofillManagerEcoreWl::IsAuthenticationNeeded() const
{
  return mIsAuthNeeded;
}

const std::string& AutofillManagerEcoreWl::GetAuthenticationServiceName() const
{
  return mAuthenticationServiceName;
}

const std::string& AutofillManagerEcoreWl::GetAuthenticationServiceMessage() const
{
  return mAuthenticationServiceMessage;
}

const std::string& AutofillManagerEcoreWl::GetAuthenticationServiceImagePath() const
{
  return mAuthenticationServiceImagePath;
}


/////////////////////////////////////////////// Autofill Fill Response ///////////////////////////////////////////////

const std::string& AutofillManagerEcoreWl::GetFillItemId() const
{
  return mFillItemId;
}

const std::string& AutofillManagerEcoreWl::GetFillItemPresentationText() const
{
  return mFillItemPresentationText;
}

const std::string& AutofillManagerEcoreWl::GetFillItemValue() const
{
  return mFillItemValue;
}

void AutofillManagerEcoreWl::SaveAutofillData( Dali::AutofillGroup group )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillManagerEcoreWl::SaveAutofillData -> Sends request to store data. \n" );

#ifdef CAPI_AUTOFILL_SUPPORT
  Internal::Adaptor::AutofillGroup& groupImpl = Internal::Adaptor::GetImplementation( group );
  Internal::Adaptor::AutofillGroupEcoreWl& groupImplWl = static_cast<Internal::Adaptor::AutofillGroupEcoreWl&>( groupImpl );

  // Sends request to save autofill data.
  int ret = autofill_commit( mAutofillHandle, groupImplWl.GetAutofillSaveGroupHandle() );
  if( ret != AUTOFILL_ERROR_NONE )
  {
    DALI_LOG_ERROR( "Failed to request auth info. error : %d \n", ret );
  }
#endif // CAPI_AUTOFILL_SUPPORT
}

// Signals
AutofillManagerEcoreWl::AuthSignalType& AutofillManagerEcoreWl::AuthenticationReceivedSignal()
{
  return mAuthReceivedSignal;
}

AutofillManagerEcoreWl::FillSignalType& AutofillManagerEcoreWl::FillResponseReceivedSignal()
{
  return mFillReceivedSignal;
}

AutofillManagerEcoreWl::ListSignalType& AutofillManagerEcoreWl::ListEventSignal()
{
  return mListReceivedSignal;
}

bool AutofillManagerEcoreWl::DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor )
{
  Dali::BaseHandle handle( object );

  bool connected( true );
  AutofillManagerEcoreWl* manager = dynamic_cast< AutofillManagerEcoreWl* >( object );

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
