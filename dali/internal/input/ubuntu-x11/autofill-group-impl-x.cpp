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
#include <dali/internal/input/ubuntu-x11/autofill-group-impl-x.h>

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
  return Dali::Internal::Adaptor::AutofillGroup::New( "" );
}

Dali::TypeRegistration type( typeid(Dali::AutofillGroup), typeid(Dali::BaseHandle), Create );

} // unnamed namespace


AutofillGroupX::AutofillGroupX( const std::string groupId )
: mAutofillItemList(),
  mGroupId( groupId )
{
}

AutofillGroupX::~AutofillGroupX()
{
}

Dali::AutofillGroup AutofillGroupX::New( const std::string& groupId )
{
  Dali::Internal::Adaptor::AutofillGroup* group = new Dali::Internal::Adaptor::AutofillGroupX( groupId );
  Dali::AutofillGroup handle = Dali::AutofillGroup( group );

  group->Initialize();

  return handle;
}

void AutofillGroupX::Initialize()
{
}

const std::string& AutofillGroupX::GetId() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillGroupX::GetId \n" );
  return mGroupId;
}

void AutofillGroupX::AddAutofillItem( Dali::AutofillItem item )
{
  // Pushes back an AutofillItem to the ItemList of AutofillGroupX.
  mAutofillItemList.push_back( item );
}

Dali::AutofillItem AutofillGroupX::GetAutofillItem( const std::string& id )
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

void AutofillGroupX::ClearAutofillItemList()
{
  for( std::vector<Dali::AutofillItem>::iterator iter = mAutofillItemList.begin(), endIter = mAutofillItemList.end(); iter !=  endIter; ++iter )
  {
    ( *iter ).ClearPresentationTextList();
    ( *iter ).ClearFillValueList();
  }
}

void AutofillGroupX::SaveAutofillData()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillGroupX::SaveAutofillData\n" );
  // Do Nothing
}

void AutofillGroupX::RequestAuthentication()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillGroupX::RequestAuthentication\n" );
  // Do Nothing
}

void AutofillGroupX::SendFillRequest()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillGroupX::SendFillRequest\n" );
  // Do Nothing
}

} // Adaptor

} // Internal

} // Dali
