/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

// #include "animation-wrapper.h"

// EXTERNAL INCLUDES
#include "emscripten/emscripten.h"
#include <dali/devel-api/scripting/scripting.h>

// INTERNAL INCLUDES
#include "property-value-wrapper.h"

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

void KeyFramesAddWithAlpha(Dali::KeyFrames& keyFrames, float progress, Property::Value& value,
                           const std::string& alphaFunction)
{
  Dali::AlphaFunction func(AlphaFunction::LINEAR);

  if( alphaFunction == "LINEAR" )
  {
    func = Dali::AlphaFunction(AlphaFunction::LINEAR);
  }
  else if( alphaFunction == "REVERSE" )
  {
    func = Dali::AlphaFunction(AlphaFunction::REVERSE);
  }
  else if( alphaFunction == "EASE_IN_SQUARE" )
  {
    func = Dali::AlphaFunction(AlphaFunction::EASE_IN_SQUARE);
  }
  else if( alphaFunction == "EASE_OUT_SQUARE" )
  {
    func = Dali::AlphaFunction(AlphaFunction::EASE_OUT_SQUARE);
  }
  else if( alphaFunction == "EASE_IN" )
  {
    func = Dali::AlphaFunction(AlphaFunction::EASE_IN);
  }
  else if( alphaFunction == "EASE_OUT" )
  {
    func = Dali::AlphaFunction(AlphaFunction::EASE_OUT);
  }
  else if( alphaFunction == "EASE_IN_OUT" )
  {
    func = Dali::AlphaFunction(AlphaFunction::EASE_IN_OUT);
  }
  else if( alphaFunction == "EASE_IN_SINE" )
  {
    func = Dali::AlphaFunction(AlphaFunction::EASE_IN_SINE);
  }
  else if( alphaFunction == "EASE_OUT_SINE" )
  {
    func = Dali::AlphaFunction(AlphaFunction::EASE_OUT_SINE);
  }
  else if( alphaFunction == "EASE_IN_OUT_SINE" )
  {
    func = Dali::AlphaFunction(AlphaFunction::EASE_IN_OUT_SINE);
  }
  else if( alphaFunction == "BOUNCE" )
  {
    func = Dali::AlphaFunction(AlphaFunction::BOUNCE);
  }
  else if( alphaFunction == "SIN" )
  {
    func = Dali::AlphaFunction(AlphaFunction::SIN);
  }
  else if( alphaFunction == "EASE_OUT_BACK" )
  {
    func = Dali::AlphaFunction(AlphaFunction::EASE_OUT_BACK);
  }
  else
  {
    EM_ASM( console.log( "KeyFramesAddWithAlpha: Unknown alpha function" ) );
  }

  keyFrames.Add(progress, value, func);
}

void AnimateTo(Dali::Animation& self,
               Dali::Handle& handle,
               const std::string& property,
               const Dali::Property::Value& destinationValue,
               const Dali::AlphaFunction::BuiltinFunction& alphaFunction,
               const float delay,
               const float duration)
{
  DALI_ASSERT_ALWAYS(self);

  Dali::Property::Index propertyIndex = handle.GetPropertyIndex(property);

  Dali::Property::Type propertyType = handle.GetPropertyType(propertyIndex);

  if( propertyType != destinationValue.GetType() )
  {
    EM_ASM( throw "animateTo property types are not the same" );
  }
  else if( propertyIndex != Dali::Property::INVALID_INDEX )
  {
    Dali::Property target( handle, propertyIndex );
    self.AnimateTo( target, destinationValue, alphaFunction, Dali::TimePeriod(delay, duration) );
  }
  else
  {
    EM_ASM( throw "unknown property" );
  }
}

void AnimateBy(Dali::Animation& self,
               Dali::Handle& handle,
               const std::string& property,
               const Dali::Property::Value& destinationValue,
               const Dali::AlphaFunction::BuiltinFunction& alphaFunction,
               const float delay,
               const float duration)
{
  DALI_ASSERT_ALWAYS(self);

  Dali::Property::Index propertyIndex = handle.GetPropertyIndex(property);

  Dali::Property::Type propertyType = handle.GetPropertyType(propertyIndex);

  if( propertyType != destinationValue.GetType() )
  {
    EM_ASM( throw "animateTo property types are not the same" );
  }
  else if( propertyIndex != Dali::Property::INVALID_INDEX )
  {
    Dali::Property target( handle, propertyIndex );
    self.AnimateBy( target, destinationValue, alphaFunction, Dali::TimePeriod(delay,duration));
  }
  else
  {
    EM_ASM( throw "unknown property" );
  }
}

void AnimateBetween(Dali::Animation& self,
                    Dali::Handle& handle,
                    const std::string& property,
                    Dali::KeyFrames& keyFrames,
                    const Dali::AlphaFunction::BuiltinFunction& alphaFunction,
                    const float delay,
                    const float duration,
                    const Dali::Animation::Interpolation& interpolation)
{
  DALI_ASSERT_ALWAYS(self);

  Dali::Property::Index propertyIndex = handle.GetPropertyIndex(property);

  if( propertyIndex != Dali::Property::INVALID_INDEX )
  {
    Dali::Property target( handle, propertyIndex );
    self.AnimateBetween(target, keyFrames, alphaFunction, Dali::TimePeriod(delay, duration), interpolation);
  }
  else
  {
    EM_ASM( throw "unknown property" );
  }
}

void AnimatePath(Dali::Animation& self,
                 Dali::Handle& target,
                 const Dali::Path& path,
                 const Dali::Vector3& forward,
                 const Dali::AlphaFunction::BuiltinFunction& alphaFunction,
                 const float delay,
                 const float duration)
{
  DALI_ASSERT_ALWAYS(self);

  if(!path)
  {
    printf("Unable to add animation, bad path object\n");
  }
  else
  {
    Dali::Actor actor = Dali::Actor::DownCast(target);
    if( !actor )
    {
      printf("Unable to add path animation, bad actor\n");
    }
    else
    {
      self.Animate( actor, path, forward, alphaFunction, Dali::TimePeriod( delay, duration ) );
    }
  }
}

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
