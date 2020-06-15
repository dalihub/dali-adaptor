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
#include <dali/internal/input/tizen-wayland/autofill-group-impl-ecore-wl.h>

// EXTERNAL INCLUDES
#include <cstring> // for strcmp
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/input/tizen-wayland/autofill-manager-impl-ecore-wl.h>

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

BaseHandle Create()
{
  return Dali::Internal::Adaptor::AutofillGroup::New( "" );
}

Dali::TypeRegistration type( typeid(Dali::AutofillGroup), typeid(Dali::BaseHandle), Create );

} // unnamed namespace


AutofillGroupEcoreWl::AutofillGroupEcoreWl( const std::string groupId )
: mAutofillItemList(),
  mGroupId( groupId )
{
#ifdef CAPI_AUTOFILL_SUPPORT
  mAutofillGroupHandle =  NULL;
  mAutofillSaveGroupHandle = NULL;
#endif // CAPI_AUTOFILL_SUPPORT
}

AutofillGroupEcoreWl::~AutofillGroupEcoreWl()
{
#ifdef CAPI_AUTOFILL_SUPPORT
  if( mAutofillGroupHandle )
  {
    autofill_view_info_destroy( mAutofillGroupHandle );
    mAutofillGroupHandle = NULL;
  }
  if( mAutofillSaveGroupHandle )
  {
    autofill_save_view_info_destroy( mAutofillSaveGroupHandle );
    mAutofillSaveGroupHandle = NULL;
  }
#endif // CAPI_AUTOFILL_SUPPORT
}


Dali::AutofillGroup AutofillGroupEcoreWl::New( const std::string& groupId )
{
  Dali::Internal::Adaptor::AutofillGroup* group = new Dali::Internal::Adaptor::AutofillGroupEcoreWl( groupId );
  Dali::AutofillGroup handle = Dali::AutofillGroup( group );
  group->Initialize();

  return handle;
}

void AutofillGroupEcoreWl::Initialize()
{
#ifdef CAPI_AUTOFILL_SUPPORT
  int ret = autofill_view_info_create( &mAutofillGroupHandle );
  if( ret != AUTOFILL_ERROR_NONE )
  {
    DALI_LOG_ERROR( "Failed to create autofill group info handle : %d \n", ret );
    return;
  }

  autofill_view_info_set_view_id( mAutofillGroupHandle, mGroupId.c_str() );

#endif // CAPI_AUTOFILL_SUPPORT
}

const std::string& AutofillGroupEcoreWl::GetId() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillGroupEcoreWl::GetId \n" );
  return mGroupId;
}

#ifdef CAPI_AUTOFILL_SUPPORT
autofill_view_info_h AutofillGroupEcoreWl::GetAutofillGroupHandle()
{
  return mAutofillGroupHandle;
}

autofill_save_view_info_h AutofillGroupEcoreWl::GetAutofillSaveGroupHandle()
{
  return mAutofillSaveGroupHandle;
}
#endif // CAPI_AUTOFILL_SUPPORT

void AutofillGroupEcoreWl::AddAutofillItem( Dali::AutofillItem item )
{
#ifdef CAPI_AUTOFILL_SUPPORT
  Internal::Adaptor::AutofillItem& itemImpl = Internal::Adaptor::GetImplementation( item );
  Internal::Adaptor::AutofillItemEcorewWl& itemImplWl = static_cast<Internal::Adaptor::AutofillItemEcorewWl&>( itemImpl );

  if( mAutofillGroupHandle && itemImplWl.GetAutofillItemHandle() )
  {
    autofill_view_info_add_item( mAutofillGroupHandle, itemImplWl.GetAutofillItemHandle() );
  }
#endif // CAPI_AUTOFILL_SUPPORT

  // Pushes back an AutofillItem to the ItemList of AutofillGroup.
  mAutofillItemList.push_back( item );
}

Dali::AutofillItem AutofillGroupEcoreWl::GetAutofillItem( const std::string& id )
{
  Dali::AutofillItem item = Dali::AutofillItem();
  for( std::vector<Dali::AutofillItem>::iterator iter = mAutofillItemList.begin(), endIter = mAutofillItemList.end(); iter !=  endIter; ++iter )
  {
    const std::string& itemId = ( *iter ).GetId();

    if( itemId.compare( id ) == 0 )
    {
      item = ( *iter );
    }
  }
  return item;
}

void AutofillGroupEcoreWl::ClearAutofillItemList()
{
  for( std::vector<Dali::AutofillItem>::iterator iter = mAutofillItemList.begin(), endIter = mAutofillItemList.end(); iter !=  endIter; ++iter )
  {
    ( *iter ).ClearPresentationTextList();
    ( *iter ).ClearFillValueList();
  }
}

void AutofillGroupEcoreWl::SaveAutofillData()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillGroupEcoreWl::SaveAutofillData\n" );
#ifdef CAPI_AUTOFILL_SUPPORT
  // Creates autofill save view info handle.
  autofill_save_view_info_create( &mAutofillSaveGroupHandle );
  autofill_save_view_info_set_view_id( mAutofillSaveGroupHandle, mGroupId.c_str() );

  for( std::vector<Dali::AutofillItem>::iterator iter = mAutofillItemList.begin(), endIter = mAutofillItemList.end(); iter !=  endIter; ++iter )
  {
    Internal::Adaptor::AutofillItem& itemImpl = Internal::Adaptor::GetImplementation( *iter );
    Internal::Adaptor::AutofillItemEcorewWl& itemImplWl = static_cast<Internal::Adaptor::AutofillItemEcorewWl&>( itemImpl );

    // Appends autofill save item in autofill save view.
    autofill_save_view_info_add_item( mAutofillSaveGroupHandle, itemImplWl.GetAutofillSaveItemHandle() );
  }
#endif // CAPI_AUTOFILL_SUPPORT
}

// If Autofill service sends authentication signal, AutofillManagerEcoreWl::ReceiveAuthInfo() would be called.
void AutofillGroupEcoreWl::RequestAuthentication()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillGroupEcoreWl::RequestAuthentication\n" );

#ifdef CAPI_AUTOFILL_SUPPORT
  Dali::AutofillManager manager = Dali::AutofillManager::Get();
  Internal::Adaptor::AutofillManager& managerImpl = Internal::Adaptor::GetImplementation( manager );
  Internal::Adaptor::AutofillManagerEcoreWl& managerImplWl = static_cast<Internal::Adaptor::AutofillManagerEcoreWl&>( managerImpl );

  // Requests to invoke the authentication information.
  // After requests of authentication, AutofillManagerEcoreWl::AuthInfoCallback would be called.
  int ret = autofill_auth_info_request( managerImplWl.GetAutofillHandle(), mAutofillGroupHandle );
  if( ret != AUTOFILL_ERROR_NONE )
  {
    DALI_LOG_ERROR( "Failed to request auth info. error : %d \n", ret );
  }
#endif // CAPI_AUTOFILL_SUPPORT
}

// If Autofill service sends fill response signal, AutofillManagerEcoreWl::FillGroupItem() or
// AutofillManagerEcoreWl::FillMultipleGroupItem() would be called according to the number of group count.
void AutofillGroupEcoreWl::SendFillRequest()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillGroupEcoreWl::SendFillRequest\n" );

#ifdef CAPI_AUTOFILL_SUPPORT
  Dali::AutofillManager manager = Dali::AutofillManager::Get();
  Internal::Adaptor::AutofillManager& managerImpl = Internal::Adaptor::GetImplementation( manager );
  Internal::Adaptor::AutofillManagerEcoreWl& managerImplWl = static_cast<Internal::Adaptor::AutofillManagerEcoreWl&>( managerImpl );

  // Removes all elements of each AutofillItem in AutofillGroup
  ClearAutofillItemList();

  // Sends fill request to fill out each input form.
  // After request of fill data, AutofillManagerEcoreWl::FillResponseCallback would be called.
  int ret = autofill_fill_request( managerImplWl.GetAutofillHandle(), mAutofillGroupHandle );
  if( ret != AUTOFILL_ERROR_NONE )
  {
    DALI_LOG_ERROR( "Failed to request fill : %d \n", ret );
  }
#endif // CAPI_AUTOFILL_SUPPORT
}

} // Adaptor

} // Internal

} // Dali
