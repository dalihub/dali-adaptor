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

#include "actor-wrapper.h"

// EXTERNAL INCLUDES
#include <dali/devel-api/scripting/scripting.h>

// INTERNAL INCLUDES
#include "emscripten-utils.h"
#include "signal-holder.h"

namespace
{
  enum ConditionType
  {
    False,                ///< Result Always False
    LessThan,             ///< Magnitude of type is less than float value (arg0).
    GreaterThan,          ///< Magnitude of type is greater than float value (arg0).
    Inside,               ///< Magnitude of type is within float values (arg0 & arg1).
    Outside,              ///< Magnitude of type is outside float values (arg0 & arg1).
    Step,                 ///< Value of type has crossed a step amount
    VariableStep          ///< Similar to step, except user can define a list of steps from reference value
  };

  Dali::Scripting::StringEnum ConditionTypeTable[] =
  { { "False",           ConditionType::False},
    { "LessThan",        ConditionType::LessThan},
    { "GreaterThan",     ConditionType::GreaterThan},
    { "Inside",          ConditionType::Inside},
    { "Outside",         ConditionType::Outside},
    { "Step",            ConditionType::Step},
    { "VariableStep",    ConditionType::VariableStep}
  };

const unsigned int ConditionTypeTableSize = sizeof( ConditionTypeTable ) / sizeof( ConditionTypeTable[0] );

};

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

emscripten::val JavascriptValue( const Dali::Property::Value& v );

/**
 * Struct to hold notification callback
 */
struct EmscriptenNotifiy
{
  EmscriptenNotifiy(const emscripten::val& callback )
    : mCallback(callback) {};
  emscripten::val mCallback;

  void operator()(void)
  {
    emscripten::val ret = mCallback();
  }
};

/**
 * Emscripten touched signal callback.
 *
 * Provides more contextual state information to the browser with the OnTouched signal.
 *
 */
struct EmscriptenTouchedSignal : public BaseSignalSlot
{
  EmscriptenTouchedSignal(const emscripten::val& callback, unsigned int id)
    : mCallback(callback), mId(id)  {};
  emscripten::val mCallback;
  unsigned int mId;

  bool OnTouched(Dali::Actor actor, const Dali::TouchData& touch)
  {
    Dali::Property::Map map;
    Dali::Property::Array points;

    const int pointCount = static_cast<int>( touch.GetPointCount() );

    map["pointCount"] = pointCount;
    map["time"] = static_cast<int>( touch.GetTime() );

    for( int i = 0; i < pointCount; ++i )
    {
      Dali::Property::Map pointMap;
      Dali::Actor hitActor = touch.GetHitActor( i );

      pointMap["deviceId"] = touch.GetDeviceId( i );
      pointMap["hitActorId"] = static_cast<int>( hitActor ? hitActor.GetId() : -1 );
      pointMap["local"] = touch.GetLocalPosition( i );
      pointMap["screen"]= touch.GetScreenPosition( i );

      switch( touch.GetState( i ) )
      {
        case PointState::DOWN:        { pointMap["state"] = "DOWN";         break; }
        case PointState::UP:          { pointMap["state"] = "UP";           break; }
        case PointState::MOTION:      { pointMap["state"] = "MOTION";       break; }
        case PointState::LEAVE:       { pointMap["state"] = "LEAVE";        break; }
        case PointState::STATIONARY:  { pointMap["state"] = "STATIONARY";   break; }
        case PointState::INTERRUPTED: { pointMap["state"] = "INTERRUPTED";  break; }
      };

      points.PushBack(pointMap);
    }

    map["points"] = points;

    Dali::Property::Value value(map);
    emscripten::val ret = mCallback( actor, JavascriptValue(value) );
    return true;
  }

  bool OnHovered(Dali::Actor actor, const Dali::HoverEvent& event)
  {
    Dali::Property::Map map;
    Dali::Property::Array points;

    map["pointCount"] = static_cast<int>(event.GetPointCount());
    map["time"] = static_cast<int>(event.time);

    for(TouchPointContainer::const_iterator iter = event.points.begin();
        iter != event.points.end(); ++iter) {
      const Dali::TouchPoint& pt = *iter;
      Dali::Property::Map pointMap;
      pointMap["deviceId"] = pt.deviceId;
      pointMap["hitActorId"] = static_cast<int>(pt.hitActor.GetId());
      pointMap["local"] = pt.local;
      pointMap["screen"]= pt.screen;

      switch(pt.state)
      {
        case TouchPoint::Down:        { pointMap["state"] = "Down"; break; }
        case TouchPoint::Up:          { pointMap["state"] = "Up"; break; }
        case TouchPoint::Motion:      { pointMap["state"] = "Motion"; break; }
        case TouchPoint::Leave:       { pointMap["state"] = "Leave"; break; }
        case TouchPoint::Stationary:  { pointMap["state"] = "Stationary"; break; }
        case TouchPoint::Interrupted: { pointMap["state"] = "Interrupted"; break; }
        case TouchPoint::Last:        { pointMap["state"] = "Last"; break; }
      };

      points.PushBack(pointMap);
    }

    map["points"] = points;

    Dali::Property::Value value(map);
    emscripten::val ret = mCallback( actor, JavascriptValue(value) );
    return true;
  }

};

/**
 * Struct to wrap a generic Emscripten callback.
 *
 */
struct EmscriptenSignal
{
  EmscriptenSignal(const emscripten::val& callback, unsigned int id)
    : mCallback(callback), mId(id) {};
  emscripten::val mCallback;
  unsigned int mId;
  bool operator()()
  {
    Dali::Actor a = Dali::Stage::GetCurrent().GetRootLayer().FindChildById(mId);
    if(a)
    {
      emscripten::val ret = mCallback( a );
    }
    else
    {
      emscripten::val ret = mCallback();
    }
    return true;
  }
};

bool ConnectSignal( Dali::Actor actor,
                    SignalHolder& signalHolder,
                    const std::string& signalName,
                    const emscripten::val& javascriptFunction )
{
  bool ret = false;
  if(0 == signalName.compare("touched"))
  {
    EmscriptenTouchedSignal* slot = new EmscriptenTouchedSignal(javascriptFunction, actor.GetId());
    actor.TouchSignal().Connect(slot, &EmscriptenTouchedSignal::OnTouched);
    signalHolder.add(slot);
    ret = true;
  }
  else if(0 == signalName.compare("hovered"))
  {
    EmscriptenTouchedSignal* slot = new EmscriptenTouchedSignal(javascriptFunction, actor.GetId());
    actor.HoveredSignal().Connect(slot, &EmscriptenTouchedSignal::OnHovered);
    signalHolder.add(slot);
    ret = true;
  }
  else
  {
    actor.ConnectSignal( &signalHolder, signalName, EmscriptenSignal(javascriptFunction, actor.GetId()));
    ret = true;
  }

  return ret;
}

unsigned int AddressOf(Dali::Actor self)
{
  return (unsigned int)&self.GetBaseObject();
}

std::vector<float> ScreenToLocal(Dali::Actor self, float screenX, float screenY)
{
  std::vector<float> ret;
  float localX = 0.f;
  float localY = 0.f;
  bool ok = self.ScreenToLocal(localX, localY, screenX, screenY);
  if( ok )
  {
    ret.push_back(localX);
    ret.push_back(localY);
    ret.push_back(1.0);
  }
  else
  {
    ret.push_back(0.0);
    ret.push_back(0.0);
    ret.push_back(0.0);
  }
  return ret;
}

void SetPropertyNotification( Dali::Actor self,
                              SignalHolder& signalHolder,
                              Dali::Property::Index index, const std::string& propertyConditionType, float arg0, float arg1,
                              const emscripten::val& javascriptFunction)
{
  unsigned int i = FindEnumIndex( propertyConditionType.c_str(), ConditionTypeTable, ConditionTypeTableSize );

  if( i < ConditionTypeTableSize )
  {
    ConditionType type = static_cast<ConditionType>(ConditionTypeTable[i].value);
    Dali::PropertyNotification notification;
    switch(type)
    {
      case ConditionType::False:
      {
        notification = self.AddPropertyNotification( index, Dali::LessThanCondition(arg0) );
      }
      case ConditionType::LessThan:
      {
        notification = self.AddPropertyNotification( index, Dali::LessThanCondition(arg0) );
      }
      case ConditionType::GreaterThan:
      {
        notification = self.AddPropertyNotification( index, Dali::GreaterThanCondition(arg0) );
      }
      case ConditionType::Inside:
      {
        notification = self.AddPropertyNotification( index, Dali::InsideCondition(arg0, arg1) );
      }
      case ConditionType::Outside:
      {
        notification = self.AddPropertyNotification( index, Dali::OutsideCondition(arg0, arg1) );
      }
      case ConditionType::Step:
      {
        notification = self.AddPropertyNotification( index, Dali::StepCondition(arg0, arg1) );
      }
      case ConditionType::VariableStep:
      {
        notification = self.AddPropertyNotification( index, Dali::StepCondition(arg0, arg1) );
      }
    };

    if(notification)
    {
      notification.NotifySignal().Connect( &signalHolder, FunctorDelegate::New(EmscriptenNotifiy(javascriptFunction)) );
    }
    else
    {
      EM_ASM( throw "Cannot set notification" );
    }
  }
}


}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
