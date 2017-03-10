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
#include <input-method-options.h>

// INTERNAL INCLUDES
#include <input-method-devel.h>

using namespace Dali::InputMethod;
using namespace Dali::InputMethod::Category;

namespace Dali
{

#define TOKEN_STRING(x) #x

struct InputMethodOptions::Impl
{
  Impl()
  {
    mPanelLayout = PanelLayout::NORMAL;
    mAutoCapital = AutoCapital::SENTENCE;
    mActionButton = ActionButtonTitle::DEFAULT;
    mVariation = NormalLayout::NORMAL;
  }

  PanelLayout::Type mPanelLayout;
  AutoCapital::Type mAutoCapital;
  ActionButtonTitle::Type mActionButton;
  int mVariation:4;
};

InputMethodOptions::InputMethodOptions()
{
  mImpl = new Impl( );
}

bool InputMethodOptions::IsPassword()
{
  return (mImpl->mPanelLayout == Dali::InputMethod::PanelLayout::PASSWORD);
}

void InputMethodOptions::ApplyProperty( const Property::Map& settings )
{
  for ( unsigned int i = 0, count = settings.Count(); i < count; ++i )
  {
    Property::Key key = settings.GetKeyAt( i );
    if( key.type == Property::Key::INDEX )
    {
      continue;
    }

    Property::Value item = settings.GetValue(i);

    if( key == TOKEN_STRING( PANEL_LAYOUT ) )
    {
      if( item.GetType() == Property::INTEGER )
      {
        int value = item.Get< int >();
        mImpl->mPanelLayout = static_cast<InputMethod::PanelLayout::Type>(value);
      }
    }
    else if ( key == TOKEN_STRING( AUTO_CAPITALISE ) )
    {
      if ( item.GetType() == Property::INTEGER )
      {
        int value = item.Get< int >();
        mImpl->mAutoCapital = static_cast<InputMethod::AutoCapital::Type>(value);
      }
    }
    else if ( key == TOKEN_STRING( ACTION_BUTTON ) )
    {
      if ( item.GetType() == Property::INTEGER )
      {
        int value = item.Get< int >();
        mImpl->mActionButton = static_cast<InputMethod::ActionButtonTitle::Type>(value);
      }
    }
    else if( key == TOKEN_STRING( VARIATION ) )
    {
      if( item.GetType() == Property::INTEGER )
      {
        int value = item.Get< int >();
        mImpl->mVariation = value;
      }
    }
    else
    {
    }
  }
}

void InputMethodOptions::RetrieveProperty( Property::Map& settings )
{
  settings[TOKEN_STRING( PANEL_LAYOUT )] = mImpl->mPanelLayout;
  settings[TOKEN_STRING( AUTO_CAPITALISE )] = mImpl->mAutoCapital;
  settings[TOKEN_STRING( ACTION_BUTTON )] = mImpl->mActionButton;
  settings[TOKEN_STRING( VARIATION )] = mImpl->mVariation;
}

bool InputMethodOptions::CompareAndSet( InputMethod::Category::Type type, const InputMethodOptions& options, int& index)
{
  bool updated = false;

  switch (type)
  {
    case PANEL_LAYOUT:
    {
      if ( options.mImpl->mPanelLayout != mImpl->mPanelLayout )
      {
        mImpl->mPanelLayout = options.mImpl->mPanelLayout;
        index = static_cast<int>(mImpl->mPanelLayout);
        updated = true;
      }
      break;
    }
    case AUTO_CAPITALISE:
    {
      if ( options.mImpl->mAutoCapital != mImpl->mAutoCapital )
      {
        mImpl->mAutoCapital = options.mImpl->mAutoCapital;
        index = static_cast<int>(mImpl->mAutoCapital);
        updated = true;
      }
      break;
    }
    case ACTION_BUTTON_TITLE:
    {
      if ( options.mImpl->mActionButton != mImpl->mActionButton )
      {
        mImpl->mActionButton = options.mImpl->mActionButton;
        index = static_cast<int>(mImpl->mActionButton);
        updated = true;
      }
      break;
    }
    case VARIATION:
    {
      if ( options.mImpl->mVariation != mImpl->mVariation )
      {
        mImpl->mVariation = options.mImpl->mVariation;
        index = static_cast<int>(mImpl->mVariation);
        updated = true;
      }
      break;
    }
  }
  return updated;
}

} // namespace Dali
