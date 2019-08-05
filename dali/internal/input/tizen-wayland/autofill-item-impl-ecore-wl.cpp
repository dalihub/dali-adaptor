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
#include <dali/internal/input/tizen-wayland/autofill-item-impl-ecore-wl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/type-registry.h>
#include <dali/integration-api/debug.h>

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
  return Dali::Internal::Adaptor::AutofillItem::New( "", "", Dali::AutofillItem::Hint::ID, false );
}

Dali::TypeRegistration type( typeid(Dali::AutofillItem), typeid(Dali::BaseHandle), Create );

} // unnamed namespace


AutofillItemEcorewWl::AutofillItemEcorewWl( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool sensitiveData )
: mId( id ),
  mLabel( label ),
  mHint( hint ),
  mSensitiveData( sensitiveData ),
  mValue(""),
  mPresentationTextList(),
  mValueList()
{
#ifdef CAPI_AUTOFILL_SUPPORT
  mAutofillItemHandle = NULL;
  mAutofillSaveItemHandle = NULL;
#endif // CAPI_AUTOFILL_SUPPORT
}

AutofillItemEcorewWl::~AutofillItemEcorewWl()
{
#ifdef CAPI_AUTOFILL_SUPPORT
  if( mAutofillItemHandle )
  {
    autofill_item_destroy( mAutofillItemHandle );
    mAutofillItemHandle = NULL;
  }

  if( mAutofillSaveItemHandle )
  {
    autofill_save_item_destroy( mAutofillSaveItemHandle );
    mAutofillSaveItemHandle = NULL;
  }
#endif // CAPI_AUTOFILL_SUPPORT
}


Dali::AutofillItem AutofillItemEcorewWl::New( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool sensitiveData )
{
  Dali::Internal::Adaptor::AutofillItem* item = new Dali::Internal::Adaptor::AutofillItemEcorewWl( id, label, hint, sensitiveData );
  Dali::AutofillItem handle = Dali::AutofillItem( item );
  item->Initialize();

  return handle;
}

void AutofillItemEcorewWl::Initialize()
{
#ifdef CAPI_AUTOFILL_SUPPORT
  int ret = autofill_item_create( &mAutofillItemHandle );
  if( ret != AUTOFILL_ERROR_NONE )
  {
    DALI_LOG_ERROR( "Failed to create autofill item handle : %d \n", ret );
    return;
  }

  autofill_item_set_id( mAutofillItemHandle, mId.c_str() );
  autofill_item_set_label( mAutofillItemHandle, mLabel.c_str() );
  autofill_item_set_sensitive_data( mAutofillItemHandle, mSensitiveData );

  // Create autofill save item handle for save.
  autofill_save_item_create( &mAutofillSaveItemHandle );
  autofill_save_item_set_id( mAutofillSaveItemHandle, mId.c_str() );
  autofill_save_item_set_label( mAutofillSaveItemHandle, mLabel.c_str() );
  autofill_save_item_set_sensitive_data( mAutofillSaveItemHandle, mSensitiveData );

  autofill_hint_e value = static_cast<autofill_hint_e>(mHint);
  autofill_item_set_autofill_hint( mAutofillItemHandle, value );
  autofill_save_item_set_autofill_hint( mAutofillSaveItemHandle, value);
#endif // CAPI_AUTOFILL_SUPPORT
}

const std::string& AutofillItemEcorewWl::GetId() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillItemEcorewWl::GetId \n" );
  return mId;
}

const std::string& AutofillItemEcorewWl::GetLabel() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillItemEcorewWl::GetLabel \n" );
  return mLabel;
}


Dali::AutofillItem::Hint AutofillItemEcorewWl::GetHint() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillItemEcorewWl::GetHint \n" );
  return mHint;
}

bool AutofillItemEcorewWl::IsSensitiveData() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillItemEcorewWl::IsSensitiveData \n" );
  return mSensitiveData;
}

void AutofillItemEcorewWl::SetSaveValue( const std::string& value )
{
  mValue = value;
#ifdef CAPI_AUTOFILL_SUPPORT
  autofill_save_item_set_value( mAutofillSaveItemHandle, mValue.c_str() );
#endif // CAPI_AUTOFILL_SUPPORT
}

const std::string& AutofillItemEcorewWl::GetSaveValue() const
{
  return mValue;
}

#ifdef CAPI_AUTOFILL_SUPPORT
autofill_item_h AutofillItemEcorewWl::GetAutofillItemHandle()
{
  return mAutofillItemHandle;
}

autofill_save_item_h AutofillItemEcorewWl::GetAutofillSaveItemHandle()
{
  return mAutofillSaveItemHandle;
}
#endif // CAPI_AUTOFILL_SUPPORT

void AutofillItemEcorewWl::AddPresentationList( const std::string& presentationText )
{
  mPresentationTextList.push_back( presentationText );
}

void AutofillItemEcorewWl::AddFillValueList( const std::string& fillValue )
{
  mValueList.push_back( fillValue );
}

const std::string& AutofillItemEcorewWl::GetPresentationText( int index ) const
{
  return mPresentationTextList[index];
}

const std::string& AutofillItemEcorewWl::GetFillValue( int index ) const
{
  return mValueList[index];
}

void AutofillItemEcorewWl::ClearPresentationTextList()
{
  mPresentationTextList.clear();
}

void AutofillItemEcorewWl::ClearFillValueList()
{
  mValueList.clear();
}

unsigned int AutofillItemEcorewWl::GetFillValueCount()
{
  return mValueList.size();
}

} // Adaptor

} // Internal

} // Dali
