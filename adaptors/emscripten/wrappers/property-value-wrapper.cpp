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

#include "property-value-wrapper.h"

// EXTERNAL INCLUDES
#include <sstream>
#include <cassert>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

void RecursiveSetProperty(Dali::Property::Value& propertyValue, const emscripten::val& fromVal)
{
  static const std::string number("number"); // we could maybe just check the first three chars? (avoiding null ...if that's returned)
  const std::string fromType( fromVal.typeof().as<std::string>() );

  if( fromType == "object" )
  {
    // hasOwnProperty is the only way I can find to tell if the object is an array
    // (keys in HasKey returns the JS keys "0","1","2","3",...)
    if( fromVal.hasOwnProperty("length") )
    {
      int length = fromVal["length"].as<int>();
      // we can't tell if what the user of the property value wants with a JS Array
      // by default 'standard' length arrays are always interpreted as Vector2/3/4 etc
      // If the user specifically wants an array they must recast.
      bool isArray = false; // nested ie [ [1,2,3], [4,5,6] ]
      if( 4 == length )
      {
        if( number == fromVal["0"].typeof().as<std::string>() &&
            number == fromVal["1"].typeof().as<std::string>() &&
            number == fromVal["2"].typeof().as<std::string>() &&
            number == fromVal["3"].typeof().as<std::string>() )
        {
          propertyValue = Dali::Vector4( fromVal["0"].as<float>(),
                                         fromVal["1"].as<float>(),
                                         fromVal["2"].as<float>(),
                                         fromVal["3"].as<float>() );
        }
        else
        {
          isArray = true;
        }
      }
      else if( 3 == length )
      {
        if( number == fromVal["0"].typeof().as<std::string>() &&
            number == fromVal["1"].typeof().as<std::string>() &&
            number == fromVal["2"].typeof().as<std::string>() )
        {
          propertyValue = Dali::Vector3( fromVal["0"].as<float>(),
                                         fromVal["1"].as<float>(),
                                         fromVal["2"].as<float>() );
        }
        else
        {
          isArray = true;
        }
      }
      else if( 2 == length )
      {
        if( number == fromVal["0"].typeof().as<std::string>() &&
            number == fromVal["1"].typeof().as<std::string>() )
        {
          propertyValue = Dali::Vector2( fromVal["0"].as<float>(),
                                         fromVal["1"].as<float>() );
        }
        else
        {
          isArray = true;
        }
      }
      else
      {
        isArray = true;
      }

      if( isArray )
      {
        propertyValue = Dali::Property::Value(Dali::Property::ARRAY);
        Dali::Property::Array* array = propertyValue.GetArray();
        for( int j = 0; j < length; ++j )
        {
          Dali::Property::Value add;
          array->PushBack( add );

          std::stringstream ss;
          ss << j;

          emscripten::val val = fromVal[ ss.str() ];
          RecursiveSetProperty( array->GetElementAt(j), val );
        }
      }
    }
    else
    {
      propertyValue = Dali::Property::Value(Dali::Property::MAP);
      Dali::Property::Map* map = propertyValue.GetMap();
      emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", fromVal);
      int keyLength = keys["length"].as<int>();
      for( int j = 0; j < keyLength; ++j )
      {
        Dali::Property::Value add;
        std::string key = keys[j].as<std::string>();
        (*map)[key] = add;
        emscripten::val keyVal = fromVal[ key ];
        RecursiveSetProperty( *(map->Find( key )), keyVal );
      }
    }
  }
  else if( fromType == "number" )
  {
    propertyValue = Dali::Property::Value( fromVal.as<float>() );
  }
  else if( fromType == "string" )
  {
    propertyValue = Dali::Property::Value( fromVal.as<std::string>() );
  }
  else
  {
    assert(0);
  }

}

emscripten::val JavascriptValue( const Dali::Property::Value& v )
{
  switch( v.GetType() )
  {
    case Dali::Property::BOOLEAN:
    {
      return emscripten::val(v.Get<bool>());
      break;
    }
    case Dali::Property::FLOAT:
    {
      return emscripten::val(v.Get<float>());
      break;
    }
    case Dali::Property::INTEGER:
    {
      return emscripten::val(v.Get<int>());
      break;
    }
    case Dali::Property::VECTOR2:
    {
      Dali::Vector2 in = v.Get<Dali::Vector2>();
      emscripten::val out = emscripten::val::array();
      out.set("0", emscripten::val(in.x) );
      out.set("1", emscripten::val(in.y) );
      return out;
      break;
    }
    case Dali::Property::VECTOR3:
    {
      Dali::Vector3 in = v.Get<Dali::Vector3>();
      emscripten::val out = emscripten::val::array();
      out.set("0", emscripten::val(in.x) );
      out.set("1", emscripten::val(in.y) );
      out.set("2", emscripten::val(in.z) );
      return out;
      break;
    }
    case Dali::Property::VECTOR4:
    {
      Dali::Vector4 in = v.Get<Dali::Vector4>();
      emscripten::val out = emscripten::val::array();
      out.set("0", emscripten::val(in.x) );
      out.set("1", emscripten::val(in.y) );
      out.set("2", emscripten::val(in.z) );
      out.set("3", emscripten::val(in.w) );
      return out;
      break;
    }
    case Dali::Property::MATRIX3:
    {
      emscripten::val val = emscripten::val::array();
      Dali::Matrix3 mat3 = v.Get<Dali::Matrix3>();

      for( int i = 0; i < 9; ++i )
      {
        std::stringstream key;
        key << i;
        val.set( key.str(), emscripten::val(mat3.AsFloat()[i]) );
      }
      return val;
      break;
    }
    case Dali::Property::MATRIX:
    {
      emscripten::val val = emscripten::val::array();
      Dali::Matrix mat = v.Get<Dali::Matrix>();

      for( int i = 0; i < 16; ++i )
      {
        std::stringstream key;
        key << i;
        val.set( key.str(), emscripten::val(mat.AsFloat()[i]) );
      }
      return val;
      break;
    }
    case Dali::Property::RECTANGLE:
    {
      Dali::Rect<int> in = v.Get<Dali::Rect<int> >();
      emscripten::val out = emscripten::val::array();
      out.set("0", emscripten::val(in.x) );
      out.set("1", emscripten::val(in.y) );
      out.set("2", emscripten::val(in.width) );
      out.set("3", emscripten::val(in.height) );
      return out;
      break;
    }
    case Dali::Property::ROTATION:
    {
      Dali::Quaternion in = v.Get<Dali::Quaternion>();
      emscripten::val out = emscripten::val::array();
      Dali::Vector3 axis;
      Dali::Radian angle;
      in.ToAxisAngle(axis, angle);

      out.set("0", emscripten::val( axis.x ) );
      out.set("1", emscripten::val( axis.y ) );
      out.set("2", emscripten::val( axis.z ) );
      out.set("3", emscripten::val( Dali::Degree(angle).degree ) );

      return out;
      break;
    }
    case Dali::Property::STRING:
    {
      return emscripten::val( v.Get<std::string>() );
      break;
    }
    case Dali::Property::ARRAY:
    {
      emscripten::val val = emscripten::val::array();
      Dali::Property::Array *array = v.GetArray();
      DALI_ASSERT_ALWAYS(array);

      for( int i = 0; i < array->Count(); ++i )
      {
        Dali::Property::Value& property = array->GetElementAt( i );
        std::stringstream key;
        key << i;
        val.set( key.str(), JavascriptValue( property ) );
      }

      return val;
      break;
    }
    case Dali::Property::MAP:
    {
      emscripten::val val = emscripten::val::object();
      Dali::Property::Map *map = v.GetMap();
      DALI_ASSERT_ALWAYS(map);

      for( int i = 0; i < map->Count(); ++i )
      {
        std::string key;
        StringValuePair pair = map->GetPair(i);
        val.set( pair.first, JavascriptValue( pair.second ) );
      }

      return val;
      break;
    }
    case Dali::Property::NONE:
    {
      break;
    }
  } // switch type

  return emscripten::val::undefined();

}

Dali::Property::Value PropertyMapGet( Dali::Property::Map& self, const std::string& key)
{
  Dali::Property::Value ret;

  Dali::Property::Value* v = self.Find(key);

  if(v)
  {
    ret = *v;
  }

  return ret;
}

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
