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
#include <dali/internal/input/ubuntu-x11/autofill-item-impl-x.h>

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


AutofillItemX::AutofillItemX( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool sensitiveData )
: mId( id ),
  mLabel( label ),
  mHint( hint ),
  mSensitiveData( sensitiveData ),
  mValue(""),
  mPresentationTextList(),
  mValueList()
{
}

AutofillItemX::~AutofillItemX()
{
}


Dali::AutofillItem AutofillItemX::New( const std::string& id, const std::string& label, Dali::AutofillItem::Hint hint, bool sensitiveData )
{
  Dali::Internal::Adaptor::AutofillItem* item = new Dali::Internal::Adaptor::AutofillItemX( id, label, hint, sensitiveData );
  Dali::AutofillItem handle = Dali::AutofillItem( item );

  item->Initialize();

  return handle;
}

void AutofillItemX::Initialize()
{
}

const std::string& AutofillItemX::GetId() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillItemX::GetId \n" );
  return mId;
}

const std::string& AutofillItemX::GetLabel() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillItemX::GetLabel \n" );
  return mLabel;
}

Dali::AutofillItem::Hint AutofillItemX::GetHint() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillItemX::GetHint \n" );
  return mHint;
}

bool AutofillItemX::IsSensitiveData() const
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "AutofillItemX::IsSensitiveData \n" );
  return mSensitiveData;
}

void AutofillItemX::SetSaveValue( const std::string& value )
{
  mValue = value;
}

const std::string& AutofillItemX::GetSaveValue() const
{
  return mValue;
}

void AutofillItemX::AddPresentationList( const std::string& presentationText )
{
  mPresentationTextList.push_back( presentationText );
}

void AutofillItemX::AddFillValueList( const std::string& fillValue )
{
  mValueList.push_back( fillValue );
}

const std::string& AutofillItemX::GetPresentationText( int index ) const
{
  return mPresentationTextList[index];
}

const std::string& AutofillItemX::GetFillValue( int index ) const
{
  return mValueList[index];
}

void AutofillItemX::ClearPresentationTextList()
{
  mPresentationTextList.clear();
}

void AutofillItemX::ClearFillValueList()
{
  mValueList.clear();
}

unsigned int AutofillItemX::GetFillValueCount()
{
  return mValueList.size();
}

} // Adaptor

} // Internal

} // Dali
