/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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


// EXTERNAL INCLUDES
#include <sstream>
#include <dali/public-api/rendering/geometry.h>
#include <dali/public-api/rendering/sampler.h>
#include <dali/public-api/rendering/shader.h>
#include <dali/public-api/rendering/texture-set.h>
#include <dali/public-api/rendering/renderer.h>
#include <dali/devel-api/scripting/scripting.h>
#include <dali/devel-api/events/hit-test-algorithm.h>
#include <dali/devel-api/images/texture-set-image.h>
#include "emscripten/emscripten.h"
#include "emscripten/bind.h"
#include "emscripten/val.h"

// INTERNAL INCLUDES
#include "platform-abstractions/emscripten/emscripten-callbacks.h"
#include "platform-abstractions/emscripten/emscripten-platform-abstraction.h"
#include "sdl-application.h"
#include "actor-wrapper.h"
#include "animation-wrapper.h"
#include "emscripten-utils.h"
#include "geometry-wrapper.h"
#include "handle-wrapper.h"
#include "image-wrapper.h"
#include "property-buffer-wrapper.h"
#include "property-value-wrapper.h"
#include "render-task-wrapper.h"
#include "shader-effect-wrapper.h"
#include "signal-holder.h"
#include "type-info-wrapper.h"

////////////////////////////////////////////////////////////////////////////////
//
// The browser javascript wrapper consists of this cpp file and a counterpart
// javascript file.
//
// They work in tandem to help make the interface natural and convenient
// for a Javascript programmer.
//
// Unfortunately there is no finalize/destruction in javascript so any cpp
// objects created must be explicitly deleted. Sometimes this wrapper can
// hide the details and simple types can be immediately marshalled to and
// from javascript values. More often objects created must have
// object.delete() called at the correct time.
//
////////////////////////////////////////////////////////////////////////////////

extern void EmscriptenMouseEvent(double x, double y, int mouseIsDown);
extern void EmscriptenUpdateOnce();
extern void EmscriptenRenderOnce();

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

// Javascript callbacks. These are set in the Javascript wrapper source and called
// from the c++ wrapper source
extern emscripten::val JSGetGlyphImage;
extern emscripten::val JSGetImage;
extern emscripten::val JSGetImageMetaData;
extern emscripten::val JSRenderFinished;

using namespace emscripten;

/**
 * Dali Vector access for Emscripten wrapping of Dali::Vector
 */
template<typename DaliVectorType>
struct DaliVectorAccess
{
  static emscripten::val Get( const DaliVectorType& v, typename DaliVectorType::SizeType index )
  {
    if (index < v.Size())
    {
      return emscripten::val(v[index]);
    }
    else
    {
      return emscripten::val::undefined();
    }
  }

  static bool Set( DaliVectorType& v, typename DaliVectorType::SizeType index, const typename DaliVectorType::ItemType& value)
  {
    v[index] = value;
    return true;
  }

  static size_t Size( DaliVectorType& v)
  {
    return v.Size();
  }
};

/**
 * Registering a Dali Vector with Emscripten to wrap for JavaScript
 */
template<typename T>
class_<Dali::Vector<T>> register_dali_vector(const char* name)
{
  typedef Dali::Vector<T> DaliVecType;

  void (DaliVecType::*PushBack)(const T&) = &DaliVecType::PushBack;
  void (DaliVecType::*Resize)(const size_t, const T&) = &DaliVecType::Resize;
  return class_<DaliVecType>(name)
    .template constructor<>()
    .function("push_back", PushBack)
    .function("resize", Resize)
    .function("size", &DaliVectorAccess<DaliVecType>::Size)
    .function("get", &DaliVectorAccess<DaliVecType>::Get)
    .function("set", &DaliVectorAccess<DaliVecType>::Set)
;
};


namespace
{

std::string VersionString()
{
  std::stringstream versionString;
  versionString <<  "DALi Core:      " << Dali::CORE_MAJOR_VERSION << "." << Dali::CORE_MINOR_VERSION << "." << Dali::CORE_MICRO_VERSION << " (" << Dali::CORE_BUILD_DATE << ")" << std::endl;
  return versionString.str();
}

/**
 * Creates an Actor previously registered with the TypeRegistry by name
 * Actors are currently differentiated in the dali-wrapper.js and have accessor functions
 * to support 'property.name=' access in the Javascript side object
 */
Dali::Actor CreateActor(const std::string& name)
{
  Dali::Actor ret;

  Dali::TypeRegistry registry = Dali::TypeRegistry::Get();

  Dali::TypeInfo typeInfo = registry.GetTypeInfo( name );

  if(!typeInfo)
  {
    EM_ASM( throw "Invalid type name" );
  }
  else
  {
    Dali::BaseHandle handle = typeInfo.CreateInstance();

    if(!handle)
    {
      EM_ASM( throw "Invalid handle. Cannot downcast (not an actor)" );
    }

    if( Dali::Actor actor = Dali::Actor::DownCast(handle) )
    {
      ret = actor;
    }
  }

  return ret;
}

/**
 * Creates any Handle from the TypeRegistry by name
 */
Dali::Handle CreateHandle(const std::string& name)
{
  Dali::Handle ret;

  Dali::TypeRegistry registry = Dali::TypeRegistry::Get();

  Dali::TypeInfo typeInfo = registry.GetTypeInfo( name );

  if(!typeInfo)
  {
    EM_ASM( throw "Invalid type name" );
  }
  else
  {
    Dali::BaseHandle base = typeInfo.CreateInstance();
    if(!base)
    {
      EM_ASM( throw "Cannot create instance (not a handle)" );
    }
    Dali::Handle handle = Dali::Handle::DownCast(base);

    if(!handle)
    {
      EM_ASM( throw "Invalid handle. Cannot downcast" );
    }

    ret = handle;
  }

  return ret;
}

/**
 * The functor to be used in the hit-test algorithm to check whether the actor is hittable.
 */
bool IsActorHittableFunction(Dali::Actor actor, Dali::HitTestAlgorithm::TraverseType type)
{
  const std::string& name = actor.GetName();
  // by convention if not visible, or with a * starting the name or if the root layer
  return actor.IsVisible() && (name.size() ? name[0] != '*' : true);
};

/*
 * Hit Test wrapper
 *
 */
Dali::Actor HitTest(float x, float y)
{
  Dali::HitTestAlgorithm::Results results;
  Dali::HitTestAlgorithm::HitTest( Dali::Stage::GetCurrent(), Dali::Vector2(x,y), results, IsActorHittableFunction );
  return results.actor;
}

/**
 * Creates a solid colour actor
 */
Dali::Actor CreateSolidColorActor( const Dali::Vector4& color, bool border, const Dali::Vector4& borderColor, const unsigned int borderSize )
{
  static const unsigned int MAX_BORDER_SIZE( 9 );

  Dali::Actor image;
  if( borderSize > MAX_BORDER_SIZE )
  {
    return image;
  }

  const unsigned int bitmapWidth = borderSize * 2 + 2;

  // Using a (2 + border) x (2 + border) image gives a better blend with the GL implementation
  // than a (1 + border) x (1 + border) image
  const unsigned int bitmapSize = bitmapWidth * bitmapWidth;
  const unsigned int topLeft = bitmapWidth * borderSize + borderSize;
  const unsigned int topRight = topLeft + 1;
  const unsigned int bottomLeft = bitmapWidth * (borderSize + 1) + borderSize;
  const unsigned int bottomRight = bottomLeft + 1;

  Dali::BufferImage imageData;
  if(color.a != 1.0 || borderColor.a != 1.0)
  {
    imageData = Dali::BufferImage::New( bitmapWidth, bitmapWidth, Dali::Pixel::RGBA8888 );

    // Create the image
    Dali::PixelBuffer* pixbuf = imageData.GetBuffer();
    Dali::Vector4 outerColor = color;
    if ( border )
    {
      outerColor = borderColor;
    }

    for( size_t i = 0; i < bitmapSize; ++i )
    {
      if( i == topLeft ||
          i == topRight ||
          i == bottomLeft ||
          i == bottomRight )
      {
        pixbuf[i*4+0] = 0xFF * color.r;
        pixbuf[i*4+1] = 0xFF * color.g;
        pixbuf[i*4+2] = 0xFF * color.b;
        pixbuf[i*4+3] = 0xFF * color.a;
      }
      else
      {
        pixbuf[i*4+0] = 0xFF * outerColor.r;
        pixbuf[i*4+1] = 0xFF * outerColor.g;
        pixbuf[i*4+2] = 0xFF * outerColor.b;
        pixbuf[i*4+3] = 0xFF * outerColor.a;
      }
    }
  }
  else
  {
    imageData = Dali::BufferImage::New( bitmapWidth, bitmapWidth, Dali::Pixel::RGB888 );

    // Create the image
    Dali::PixelBuffer* pixbuf = imageData.GetBuffer();
    Dali::Vector4 outerColor = color;
    if ( border )
    {
      outerColor = borderColor;
    }

    for( size_t i = 0; i < bitmapSize; ++i )
    {
      if( i == topLeft ||
          i == topRight ||
          i == bottomLeft ||
          i == bottomRight )
      {
        pixbuf[i*3+0] = 0xFF * color.r;
        pixbuf[i*3+1] = 0xFF * color.g;
        pixbuf[i*3+2] = 0xFF * color.b;
      }
      else
      {
        pixbuf[i*3+0] = 0xFF * outerColor.r;
        pixbuf[i*3+1] = 0xFF * outerColor.g;
        pixbuf[i*3+2] = 0xFF * outerColor.b;
      }
    }
  }

  imageData.Update();
  image = Dali::Actor::New();
  image.SetAnchorPoint( Dali::AnchorPoint::CENTER );
  image.SetParentOrigin( Dali::ParentOrigin::CENTER );

  std::string vertexShader(
    "attribute mediump vec2 aPosition;\n"
    "varying mediump vec2 vTexCoord;\n"
    "uniform mediump mat4 uMvpMatrix;\n"
    "uniform mediump vec3 uSize;\n"
    "uniform mediump vec4 sTextureRect;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = uMvpMatrix * vec4(aPosition * uSize.xy, 0.0, 1.0);\n"
    "  vTexCoord = aPosition + vec2(0.5);\n"
    "}\n");

  std::string fragmentShader(
    "varying mediump vec2 vTexCoord;\n"
    "uniform sampler2D sTexture;\n"
    "uniform lowp vec4 uColor;\n"
    "void main()\n"
    "{\n"
    "  gl_FragColor = texture2D( sTexture, vTexCoord )*uColor;\n"
    "}\n");

  Dali::Shader shader = Dali::Shader::New( vertexShader, fragmentShader );

  // Create Quad geometry
  Dali::Property::Map quadVertexFormat;
  quadVertexFormat["aPosition"] = Dali::Property::VECTOR2;
  Dali::PropertyBuffer vertexData = Dali::PropertyBuffer::New( quadVertexFormat );
  const float halfQuadSize = .5f;
  struct QuadVertex { Dali::Vector2 position; };
  QuadVertex quadVertexData[4] = {
      { Dali::Vector2(-halfQuadSize, -halfQuadSize) },
      { Dali::Vector2(-halfQuadSize, halfQuadSize) },
      { Dali::Vector2( halfQuadSize, -halfQuadSize) },
      { Dali::Vector2( halfQuadSize, halfQuadSize) } };
  vertexData.SetData(quadVertexData, 4);

  Dali::Geometry quad = Dali::Geometry::New();
  quad.AddVertexBuffer( vertexData );
  quad.SetType( Dali::Geometry::TRIANGLE_STRIP );

  Dali::Renderer renderer = Dali::Renderer::New( quad, shader );
  Dali::TextureSet textureSet = Dali::TextureSet::New();
  TextureSetImage( textureSet, 0u, imageData );
  renderer.SetTextures( textureSet );

  image.AddRenderer( renderer );

  return image;
}

} // anon namespace

/**
 * Sets the callback to getting a glyph (from dali-wrapper.js/browser)
 */
void SetCallbackGetGlyphImage(const emscripten::val& callback)
{
  JSGetGlyphImage = callback;
}

/**
 * Sets the callback to get an image (from dali-wrapper.js/browser)
 */
void SetCallbackGetImage(const emscripten::val& callback)
{
  JSGetImage = callback;
}

/**
 * Sets the callback to get image meta data (from dali-wrapper.js/browser)
 */
void SetCallbackGetImageMetadata(const emscripten::val& callback)
{
  JSGetImageMetaData = callback;
}

/**
 * Sets the callback to signal render finished to dali-wrapper.js/browser
 */
void SetCallbackRenderFinished(const emscripten::val& callback)
{
  JSRenderFinished = callback;
}

/**
 * Generates control points for a Path
 */
void GenerateControlPoints(Dali::Handle& handle, float curvature)
{
  if( handle )
  {
    Dali::Path path = Dali::Path::DownCast(handle);
    if(path)
    {
      path.GenerateControlPoints(curvature);
    }
    else
    {
      EM_ASM( throw "Handle is not a path object" );
    }
  }
  else
  {
    EM_ASM( throw "Handle is empty" );
  }

}

/**
 * Creating property Value Objects
 *   javascript can't select on type so we have renamed constructors
 *
 * Emscripten can convert some basic types and ones we've declared as value_array(s)
 * (These are given member access offsets/functions and are for simple structs etc)
 *
 * The composite types need converting.
 */
Dali::Property::Value PropertyValueBoolean(bool v)
{
  return Dali::Property::Value(v);
}

Dali::Property::Value PropertyValueFloat(float v)
{
  return Dali::Property::Value(v);
}

Dali::Property::Value PropertyValueInteger(int v)
{
  return Dali::Property::Value(v);
}

Dali::Property::Value PropertyValueVector2( const Dali::Vector2& v )
{
  return Dali::Property::Value(v);
}

Dali::Property::Value PropertyValueVector3( const Dali::Vector3& v )
{
  return Dali::Property::Value(v);
}

Dali::Property::Value PropertyValueVector4( const Dali::Vector4& v )
{
  return Dali::Property::Value(v);
}

Dali::Property::Value PropertyValueIntRect( int a, int b, int c, int d )
{
  return Dali::Property::Value(Dali::Rect<int>( a, b, c, d));
}

Dali::Property::Value PropertyValueMatrix( const Dali::Matrix& v )
{
  return Dali::Property::Value(v);
}

Dali::Property::Value PropertyValueMatrix3( const Dali::Matrix3& v )
{
  return Dali::Property::Value(v);
}

Dali::Property::Value PropertyValueEuler( const Dali::Vector3& v )
{
  return Dali::Property::Value( Dali::Quaternion(
                                  Dali::Radian(Dali::Degree(v.x)),
                                  Dali::Radian(Dali::Degree(v.y)),
                                  Dali::Radian(Dali::Degree(v.z)) ) );
}

Dali::Property::Value PropertyValueAxisAngle( const Dali::Vector4& v )
{
  return Dali::Quaternion( Dali::Radian(Dali::Degree(v[3])), Dali::Vector3(v) );
}

Dali::Property::Value PropertyValueString( const std::string& v )
{
  return Dali::Property::Value(v);
}

Dali::Property::Value PropertyValueContainer( const emscripten::val& v )
{
  Dali::Property::Value ret;
  RecursiveSetProperty( ret, v );
  return ret;
}

/**
 * Property value accessors from Dali Property Value
 */
bool PropertyGetBoolean(const Dali::Property::Value& v)
{
  return v.Get<bool>();
}

float PropertyGetFloat(const Dali::Property::Value& v)
{
  return v.Get<float>();
}

int PropertyGetInteger(const Dali::Property::Value& v)
{
  return v.Get<int>();
}

Dali::Vector2 PropertyGetVector2(const Dali::Property::Value& v)
{
  return v.Get<Dali::Vector2>();
}

Dali::Vector3 PropertyGetVector3(const Dali::Property::Value& v)
{
  return v.Get<Dali::Vector3>();
}

Dali::Vector4 PropertyGetVector4(const Dali::Property::Value& v)
{
  return v.Get<Dali::Vector4>();
}

emscripten::val PropertyGetIntRect(const Dali::Property::Value& v)
{
  return JavascriptValue(v);
}

std::string PropertyGetString(const Dali::Property::Value& v)
{
  return v.Get<std::string>();
}

emscripten::val PropertyGetMap(const Dali::Property::Value& v)
{
  return JavascriptValue(v);
}

emscripten::val PropertyGetArray(const Dali::Property::Value& v)
{
  return JavascriptValue(v);
}

emscripten::val PropertyGetMatrix(const Dali::Property::Value& v)
{
  return JavascriptValue(v);
}

emscripten::val PropertyGetMatrix3(const Dali::Property::Value& v)
{
  return JavascriptValue(v);
}

emscripten::val PropertyGetEuler(const Dali::Property::Value& v)
{
  return JavascriptValue(v);
}

emscripten::val PropertyGetRotation(const Dali::Property::Value& v)
{
  return JavascriptValue(v);
}

int PropertyGetType(Dali::Property::Value& v)
{
  return (int)v.GetType();
}

std::string PropertyGetTypeName(Dali::Property::Value& v)
{
  return Dali::PropertyTypes::GetName(v.GetType());
}

template <typename T>
float MatrixGetter(T &v, int n)
{
  return *(v.AsFloat() + n);
}

template <typename T>
void MatrixSetter(T &v, float f, int n)
{
  *(v.AsFloat() + n) = f;
}

float MatrixGetter0(const Dali::Matrix &v)        { return MatrixGetter(v, 0); }
float MatrixGetter1(const Dali::Matrix &v)        { return MatrixGetter(v, 1); }
float MatrixGetter2(const Dali::Matrix &v)        { return MatrixGetter(v, 2); }
float MatrixGetter3(const Dali::Matrix &v)        { return MatrixGetter(v, 3); }
float MatrixGetter4(const Dali::Matrix &v)        { return MatrixGetter(v, 4); }
float MatrixGetter5(const Dali::Matrix &v)        { return MatrixGetter(v, 5); }
float MatrixGetter6(const Dali::Matrix &v)        { return MatrixGetter(v, 6); }
float MatrixGetter7(const Dali::Matrix &v)        { return MatrixGetter(v, 7); }
float MatrixGetter8(const Dali::Matrix &v)        { return MatrixGetter(v, 8); }
float MatrixGetter9(const Dali::Matrix &v)        { return MatrixGetter(v, 9); }
float MatrixGetter10(const Dali::Matrix &v)        { return MatrixGetter(v,10); }
float MatrixGetter11(const Dali::Matrix &v)        { return MatrixGetter(v,11); }
float MatrixGetter12(const Dali::Matrix &v)        { return MatrixGetter(v,12); }
float MatrixGetter13(const Dali::Matrix &v)        { return MatrixGetter(v,13); }
float MatrixGetter14(const Dali::Matrix &v)        { return MatrixGetter(v,14); }
float MatrixGetter15(const Dali::Matrix &v)        { return MatrixGetter(v,15); }
float MatrixGetter16(const Dali::Matrix &v)        { return MatrixGetter(v,16); }

void MatrixSetter0(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 0); }
void MatrixSetter1(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 1); }
void MatrixSetter2(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 2); }
void MatrixSetter3(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 3); }
void MatrixSetter4(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 4); }
void MatrixSetter5(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 5); }
void MatrixSetter6(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 6); }
void MatrixSetter7(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 7); }
void MatrixSetter8(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 8); }
void MatrixSetter9(Dali::Matrix &v, float f)      { MatrixSetter(v, f, 9); }
void MatrixSetter10(Dali::Matrix &v, float f)      { MatrixSetter(v, f,10); }
void MatrixSetter11(Dali::Matrix &v, float f)      { MatrixSetter(v, f,11); }
void MatrixSetter12(Dali::Matrix &v, float f)      { MatrixSetter(v, f,12); }
void MatrixSetter13(Dali::Matrix &v, float f)      { MatrixSetter(v, f,13); }
void MatrixSetter14(Dali::Matrix &v, float f)      { MatrixSetter(v, f,14); }
void MatrixSetter15(Dali::Matrix &v, float f)      { MatrixSetter(v, f,15); }
void MatrixSetter16(Dali::Matrix &v, float f)      { MatrixSetter(v, f,16); }

float Matrix3Getter0(const Dali::Matrix3 &v)        { return MatrixGetter(v, 0); }
float Matrix3Getter1(const Dali::Matrix3 &v)        { return MatrixGetter(v, 1); }
float Matrix3Getter2(const Dali::Matrix3 &v)        { return MatrixGetter(v, 2); }
float Matrix3Getter3(const Dali::Matrix3 &v)        { return MatrixGetter(v, 3); }
float Matrix3Getter4(const Dali::Matrix3 &v)        { return MatrixGetter(v, 4); }
float Matrix3Getter5(const Dali::Matrix3 &v)        { return MatrixGetter(v, 5); }
float Matrix3Getter6(const Dali::Matrix3 &v)        { return MatrixGetter(v, 6); }
float Matrix3Getter7(const Dali::Matrix3 &v)        { return MatrixGetter(v, 7); }
float Matrix3Getter8(const Dali::Matrix3 &v)        { return MatrixGetter(v, 8); }

void Matrix3Setter0(Dali::Matrix3 &v, float f)      { MatrixSetter(v, f, 0); }
void Matrix3Setter1(Dali::Matrix3 &v, float f)      { MatrixSetter(v, f, 1); }
void Matrix3Setter2(Dali::Matrix3 &v, float f)      { MatrixSetter(v, f, 2); }
void Matrix3Setter3(Dali::Matrix3 &v, float f)      { MatrixSetter(v, f, 3); }
void Matrix3Setter4(Dali::Matrix3 &v, float f)      { MatrixSetter(v, f, 4); }
void Matrix3Setter5(Dali::Matrix3 &v, float f)      { MatrixSetter(v, f, 5); }
void Matrix3Setter6(Dali::Matrix3 &v, float f)      { MatrixSetter(v, f, 6); }
void Matrix3Setter7(Dali::Matrix3 &v, float f)      { MatrixSetter(v, f, 7); }
void Matrix3Setter8(Dali::Matrix3 &v, float f)      { MatrixSetter(v, f, 8); }

val JavascriptUpdateCallback( val::undefined() );
bool JavascriptUpdateCallbackSet = false;

void JavascriptUpdate(int dt)
{
  if( JavascriptUpdateCallbackSet )
  {
    JavascriptUpdateCallback( val(dt) );
  }
}

void SetUpdateFunction( const emscripten::val& function )
{
  JavascriptUpdateCallback = function;
  JavascriptUpdateCallbackSet = true;
}

const emscripten::val& GetUpdateFunction()
{
  return JavascriptUpdateCallback;
}

/**
 * Emscripten Bindings
 *
 * This uses Emscripten 'bind' (similar in design to Boost's python wrapper library).
 * It provides a simplified way to wrap C++ classes and wraps Emscriptens type
 * registration API.
 *
 * A convention below is that where there is a function or method name prepended
 * with '__' there is a corresponding Javascript helper function to help with
 * marshalling parameters and return values.
 *
 */
EMSCRIPTEN_BINDINGS(dali_wrapper)
{
  // With a little help embind knows about vectors so we tell it which ones to instantiate
  register_dali_vector<int>("DaliVectorInt");
  register_vector<std::string>("VectorString");
  register_vector<int>("VectorInt");
  register_vector<float>("VectorFloat");
  register_vector<Dali::Actor>("VectorActor");

  //
  // Creation functions.
  //
  emscripten::function("VersionString", &VersionString);
  emscripten::function("__createActor", &CreateActor);
  emscripten::function("__createHandle", &CreateHandle);
  emscripten::function("__createSolidColorActor", &CreateSolidColorActor);

  //
  // Helper functions
  //
  emscripten::function("javascriptValue", &JavascriptValue);
  emscripten::function("__hitTest", &HitTest);
  emscripten::function("sendMouseEvent", &EmscriptenMouseEvent);
  emscripten::function("__updateOnce", &EmscriptenUpdateOnce);
  emscripten::function("__renderOnce", &EmscriptenRenderOnce);
  emscripten::function("generateControlPoints", &GenerateControlPoints);

  //
  // Global callback functions
  //
  emscripten::function("setCallbackGetGlyphImage", &SetCallbackGetGlyphImage);
  emscripten::function("setCallbackGetImage", &SetCallbackGetImage);
  emscripten::function("setCallbackGetImageMetadata", &SetCallbackGetImageMetadata);
  emscripten::function("setCallbackRenderFinished", &SetCallbackRenderFinished);
  emscripten::function("setUpdateFunction", &SetUpdateFunction);
  emscripten::function("getUpdateFunction", &GetUpdateFunction);

  //
  // Property value creation (Javascript can't select on type)
  //
  emscripten::function("PropertyValueBoolean", &PropertyValueBoolean);
  emscripten::function("PropertyValueFloat", &PropertyValueFloat);
  emscripten::function("PropertyValueInteger", &PropertyValueInteger);
  emscripten::function("PropertyValueString", &PropertyValueString);
  emscripten::function("PropertyValueVector2", &PropertyValueVector2);
  emscripten::function("PropertyValueVector3", &PropertyValueVector3);
  emscripten::function("PropertyValueVector4", &PropertyValueVector4);
  emscripten::function("PropertyValueMatrix", &PropertyValueMatrix);
  emscripten::function("PropertyValueMatrix3", &PropertyValueMatrix3);
  emscripten::function("PropertyValueMap", &PropertyValueContainer);
  emscripten::function("PropertyValueArray", &PropertyValueContainer);
  emscripten::function("PropertyValueEuler", &PropertyValueEuler);
  emscripten::function("PropertyValueAxisAngle", &PropertyValueAxisAngle);
  emscripten::function("PropertyValueString", &PropertyValueString);
  emscripten::function("PropertyValueIntRect", &PropertyValueIntRect);

  //
  // One unfortunate aspect of wrapping for the browser is that we get no notification
  // of object deletion so all JS wrapper objects must be wrapper.delete() correctly.
  //
  // Embind has a mechanism around this for simple c style structs decared as value_arrays.
  // These are immediately transformed to Javascript arrays and don't need explicit
  // destruction, however the API needs a member offset.
  //
  value_array<Dali::Internal::Emscripten::Statistics>("Statistics")
    .element(&Dali::Internal::Emscripten::Statistics::on)
    .element(&Dali::Internal::Emscripten::Statistics::frameCount)
    .element(&Dali::Internal::Emscripten::Statistics::lastFrameDeltaSeconds)
    .element(&Dali::Internal::Emscripten::Statistics::lastSyncTimeMilliseconds)
    .element(&Dali::Internal::Emscripten::Statistics::nextSyncTimeMilliseconds)
    .element(&Dali::Internal::Emscripten::Statistics::keepUpdating)
    .element(&Dali::Internal::Emscripten::Statistics::needsNotification)
    .element(&Dali::Internal::Emscripten::Statistics::secondsFromLastFrame)
;

  value_array<Dali::Vector2>("Vector2")
    .element(&Dali::Vector2::x)
    .element(&Dali::Vector2::y)
;

  value_array<Dali::Vector3>("Vector3")
    .element(&Dali::Vector3::x)
    .element(&Dali::Vector3::y)
    .element(&Dali::Vector3::z)
;

  value_array<Dali::Vector4>("Vector4")
    .element(&Dali::Vector4::x)
    .element(&Dali::Vector4::y)
    .element(&Dali::Vector4::z)
    .element(&Dali::Vector4::w)
;

  value_array<Dali::Matrix>("Matrix")
    .element(&MatrixGetter0, &MatrixSetter0)
    .element(&MatrixGetter1, &MatrixSetter1)
    .element(&MatrixGetter2, &MatrixSetter2)
    .element(&MatrixGetter3, &MatrixSetter3)
    .element(&MatrixGetter4, &MatrixSetter4)
    .element(&MatrixGetter5, &MatrixSetter5)
    .element(&MatrixGetter6, &MatrixSetter6)
    .element(&MatrixGetter7, &MatrixSetter7)
    .element(&MatrixGetter8, &MatrixSetter8)
    .element(&MatrixGetter9, &MatrixSetter9)
    .element(&MatrixGetter10, &MatrixSetter10)
    .element(&MatrixGetter11, &MatrixSetter11)
    .element(&MatrixGetter12, &MatrixSetter12)
    .element(&MatrixGetter13, &MatrixSetter13)
    .element(&MatrixGetter14, &MatrixSetter14)
    .element(&MatrixGetter15, &MatrixSetter15)
;

  value_array<Dali::Matrix3>("Matrix3")
    .element(&Matrix3Getter0, &Matrix3Setter0)
    .element(&Matrix3Getter1, &Matrix3Setter1)
    .element(&Matrix3Getter2, &Matrix3Setter2)
    .element(&Matrix3Getter3, &Matrix3Setter3)
    .element(&Matrix3Getter4, &Matrix3Setter4)
    .element(&Matrix3Getter5, &Matrix3Setter5)
    .element(&Matrix3Getter6, &Matrix3Setter6)
    .element(&Matrix3Getter7, &Matrix3Setter7)
    .element(&Matrix3Getter8, &Matrix3Setter8)
;

  //
  // enums
  //
  enum_<Dali::Property::Type>("PropertyType")
    .value("NONE", Dali::Property::NONE)
    .value("BOOLEAN", Dali::Property::BOOLEAN)
    .value("FLOAT", Dali::Property::FLOAT)
    .value("INTEGER", Dali::Property::INTEGER)
    .value("VECTOR2", Dali::Property::VECTOR2)
    .value("VECTOR3", Dali::Property::VECTOR3)
    .value("VECTOR4", Dali::Property::VECTOR4)
    .value("MATRIX3", Dali::Property::MATRIX3)
    .value("MATRIX", Dali::Property::MATRIX)
    .value("RECTANGLE", Dali::Property::RECTANGLE)
    .value("ROTATION", Dali::Property::ROTATION)
    .value("STRING", Dali::Property::STRING)
    .value("ARRAY", Dali::Property::ARRAY)
    .value("MAP", Dali::Property::MAP)
;

  enum_<Dali::ShaderEffect::GeometryHints>("GeometryHints")
    .value("HINT_NONE", Dali::ShaderEffect::HINT_NONE)
    .value("HINT_GRID_X", Dali::ShaderEffect::HINT_GRID_X)
    .value("HINT_GRID_Y", Dali::ShaderEffect::HINT_GRID_Y)
    .value("HINT_GRID", Dali::ShaderEffect::HINT_GRID)
    .value("HINT_DEPTH_BUFFER", Dali::ShaderEffect::HINT_DEPTH_BUFFER)
    .value("HINT_BLENDING", Dali::ShaderEffect::HINT_BLENDING)
    .value("HINT_DOESNT_MODIFY_GEOMETRY", Dali::ShaderEffect::HINT_DOESNT_MODIFY_GEOMETRY)
;

  enum_<Dali::Shader::Hint::Value>("ShaderHints")
    .value("NONE",                      Dali::Shader::Hint::NONE)
    .value("OUTPUT_IS_TRANSPARENT",     Dali::Shader::Hint::OUTPUT_IS_TRANSPARENT)
    .value("MODIFIES_GEOMETRY",         Dali::Shader::Hint::MODIFIES_GEOMETRY)
;

  enum_<Dali::Animation::EndAction>("EndAction")
    .value("Bake",        Dali::Animation::Bake)
    .value("Discard",     Dali::Animation::Discard)
    .value("BakeFinal",   Dali::Animation::BakeFinal)
;

  enum_<Dali::Animation::Interpolation>("Interpolation")
    .value("Linear", Dali::Animation::Interpolation::Linear)
    .value("Cubic", Dali::Animation::Interpolation::Cubic)
;

  enum_<Dali::Geometry::Type>("Type")
    .value("POINTS",          Dali::Geometry::POINTS)
    .value("LINES",           Dali::Geometry::LINES)
    .value("LINE_LOOP",       Dali::Geometry::LINE_LOOP)
    .value("LINE_STRIP",      Dali::Geometry::LINE_STRIP)
    .value("TRIANGLES",       Dali::Geometry::TRIANGLES)
    .value("TRIANGLE_FAN",    Dali::Geometry::TRIANGLE_FAN)
    .value("TRIANGLE_STRIP",  Dali::Geometry::TRIANGLE_STRIP)
;

  enum_<Dali::Pixel::Format>("PixelFormat")
    .value("A8", Dali::Pixel::Format::A8)
    .value("L8", Dali::Pixel::Format::L8)
    .value("LA88", Dali::Pixel::Format::LA88)
    .value("RGB565", Dali::Pixel::Format::RGB565)
    .value("BGR565", Dali::Pixel::Format::BGR565)
    .value("RGBA4444", Dali::Pixel::Format::RGBA4444)
    .value("BGRA4444", Dali::Pixel::Format::BGRA4444)
    .value("RGBA5551", Dali::Pixel::Format::RGBA5551)
    .value("BGRA5551", Dali::Pixel::Format::BGRA5551)
    .value("RGB888", Dali::Pixel::Format::RGB888)
    .value("RGB8888", Dali::Pixel::Format::RGB8888)
    .value("BGR8888", Dali::Pixel::Format::BGR8888)
    .value("RGBA8888", Dali::Pixel::Format::RGBA8888)
    .value("BGRA8888", Dali::Pixel::Format::BGRA8888)
    // GLES 3 Standard compressed formats:
    .value("COMPRESSED_R11_EAC", Dali::Pixel::Format::COMPRESSED_R11_EAC)
    .value("COMPRESSED_SIGNED_R11_EAC", Dali::Pixel::Format::COMPRESSED_SIGNED_R11_EAC)
    .value("COMPRESSED_RG11_EAC", Dali::Pixel::Format::COMPRESSED_RG11_EAC)
    .value("COMPRESSED_SIGNED_RG11_EAC", Dali::Pixel::Format::COMPRESSED_SIGNED_RG11_EAC)
    .value("COMPRESSED_RGB8_ETC2", Dali::Pixel::Format::COMPRESSED_RGB8_ETC2)
    .value("COMPRESSED_SRGB8_ETC2", Dali::Pixel::Format::COMPRESSED_SRGB8_ETC2)
    .value("COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2", Dali::Pixel::Format::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2)
    .value("COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2", Dali::Pixel::Format::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2)
    .value("COMPRESSED_RGBA8_ETC2_EAC", Dali::Pixel::Format::COMPRESSED_RGBA8_ETC2_EAC)
    .value("COMPRESSED_SRGB8_ALPHA8_ETC2_EAC", Dali::Pixel::Format::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC)
    // GLES 2 extension compressed formats:
    .value("COMPRESSED_RGB8_ETC1", Dali::Pixel::Format::COMPRESSED_RGB8_ETC1)
    .value("COMPRESSED_RGB_PVRTC_4BPPV1", Dali::Pixel::Format::COMPRESSED_RGB_PVRTC_4BPPV1)
;

  enum_<Dali::FaceCullingMode::Type>("FaceCullingMode")
    .value("NONE", Dali::FaceCullingMode::NONE)
    .value("FRONT", Dali::FaceCullingMode::FRONT)
    .value("BACK", Dali::FaceCullingMode::BACK)
    .value("FRONT_AND_BACK", Dali::FaceCullingMode::FRONT_AND_BACK)
;

  enum_<Dali::DepthWriteMode::Type>("DepthWriteMode")
    .value("OFF", Dali::DepthWriteMode::OFF)
    .value("AUTO", Dali::DepthWriteMode::AUTO)
    .value("ON", Dali::DepthWriteMode::ON)
;

  enum_<Dali::BlendMode::Type>("BlendMode")
    .value("OFF", Dali::BlendMode::OFF)
    .value("AUTO", Dali::BlendMode::AUTO)
    .value("ON", Dali::BlendMode::ON)
;

  enum_<Dali::AlphaFunction::BuiltinFunction>("AlphaFunction")
    .value("DEFAULT", Dali::AlphaFunction::BuiltinFunction::DEFAULT)
    .value("LINEAR", Dali::AlphaFunction::BuiltinFunction::LINEAR)
    .value("REVERSE", Dali::AlphaFunction::BuiltinFunction::REVERSE)
    .value("EASE_IN_SQUARE", Dali::AlphaFunction::BuiltinFunction::EASE_IN_SQUARE)
    .value("EASE_OUT_SQUARE", Dali::AlphaFunction::BuiltinFunction::EASE_OUT_SQUARE)
    .value("EASE_IN", Dali::AlphaFunction::BuiltinFunction::EASE_IN)
    .value("EASE_OUT", Dali::AlphaFunction::BuiltinFunction::EASE_OUT)
    .value("EASE_IN_OUT", Dali::AlphaFunction::BuiltinFunction::EASE_IN_OUT)
    .value("EASE_IN_SINE", Dali::AlphaFunction::BuiltinFunction::EASE_IN_SINE)
    .value("EASE_OUT_SINE", Dali::AlphaFunction::BuiltinFunction::EASE_OUT_SINE)
    .value("EASE_IN_OUT_SINE", Dali::AlphaFunction::BuiltinFunction::EASE_IN_OUT_SINE)
    .value("BOUNCE", Dali::AlphaFunction::BuiltinFunction::BOUNCE)
    .value("SIN", Dali::AlphaFunction::BuiltinFunction::SIN)
    .value("EASE_OUT_BACK", Dali::AlphaFunction::BuiltinFunction::EASE_OUT_BACK)
;

  //
  // classes
  //

  // we need property map as an object rather than straight conversion to javascript 'object'
  // because its ordered. And PropertyBuffer needs an order.
  class_<Dali::Property::Map>("PropertyMap")
    .constructor<>()
    .function("count", &Dali::Property::Map::Count)
    .function("empty", &Dali::Property::Map::Empty)
    .function("__insert", select_overload< void(const std::string&, const Dali::Property::Value&) > (&Dali::Property::Map::Insert))
    .function("__get", &PropertyMapGet)
    .function("__getValue", &Dali::Property::Map::GetValue)
    .function("getKey", &Dali::Property::Map::GetKey)
    .function("clear", &Dali::Property::Map::Clear)
    .function("merge", &Dali::Property::Map::Merge)
;

  class_<Dali::Property::Value>("PropertyValue")
    .constructor<>()
    .function("getType", &PropertyGetType)
    .function("getTypeName", &PropertyGetTypeName)
    .function("getBoolean", &PropertyGetBoolean)
    .function("getFloat", &PropertyGetFloat)
    .function("getInteger", &PropertyGetInteger)
    .function("getVector2", &PropertyGetVector2)
    .function("getVector3", &PropertyGetVector3)
    .function("getVector4", &PropertyGetVector4)
    .function("getString", &PropertyGetString)
    .function("getMap", &PropertyGetMap)
    .function("getArray", &PropertyGetArray)
    .function("getMatrix", &PropertyGetMatrix)
    .function("getMatrix3", &PropertyGetMatrix3)
    .function("getEuler", &PropertyGetEuler)
    .function("getRotation", &PropertyGetRotation)
    .function("getIntRect", &PropertyGetIntRect)
;

  class_<Dali::BaseHandle>("BaseHandle")
    .function("ok", &BaseHandleOk)
    .function("getTypeName", &BaseHandle::GetTypeName)
;

  class_<Dali::TypeInfo, base<Dali::BaseHandle>>("TypeInfo")
    .function("getName", &Dali::TypeInfo::GetName)
    .function("getBaseName", &Dali::TypeInfo::GetBaseName)
    .function("getProperties", &GetAllProperties)
    .function("getActions", &GetActions)
    .function("getSignals", &GetSignals)
    .function("getPropertyIndices", &Dali::TypeInfo::GetPropertyIndices)
;

  class_<Dali::TypeRegistry>("TypeRegistry")
    .constructor<>(&Dali::TypeRegistry::Get)
    .function("getTypeNameCount", &Dali::TypeRegistry::GetTypeNameCount)
    .function("getTypeName", &Dali::TypeRegistry::GetTypeName)
    .function("getTypeInfo", select_overload< Dali::TypeInfo(const std::string&) > (&Dali::TypeRegistry::GetTypeInfo))
;

  class_<SignalHolder>("SignalHolder")
    .constructor<>()
;

  class_<Dali::Handle, base<Dali::BaseHandle>>("Handle")
    .function("__registerProperty", &RegisterProperty)
    .function("__registerAnimatedProperty", &RegisterAnimatedProperty)
    .function("setSelf", &SetSelf)
    .function("setProperty", &SetProperty)
    .function("getProperty", &GetProperty)
    .function("getPropertyIndex", &GetPropertyIndex)
    .function("getProperties", &GetProperties)
    .function("getPropertyIndices", &Handle::GetPropertyIndices)
    .function("getPropertyTypeFromName", &GetPropertyTypeFromName)
    .function("getPropertyTypeName", &GetPropertyTypeName)
    .function("registerProperty", &RegisterProperty)
    .function("registerAnimatedProperty", &RegisterAnimatedProperty)
    .function("getTypeInfo", &GetTypeInfo)
    .function("isPropertyWritable", &Handle::IsPropertyWritable)
    .function("isPropertyAnimatable", &Handle::IsPropertyAnimatable)
    .function("isPropertyAConstraintInput", &Handle::IsPropertyAConstraintInput)
;

  class_<Dali::Path, base<Dali::Handle>>("Path")
    .constructor<>(&Dali::Path::New)
    .function("addPoint", &Dali::Path::AddPoint)
    .function("addControlPoint", &Dali::Path::AddControlPoint)
    .function("generateControlPoints", &Dali::Path::GenerateControlPoints)
    .function("sample", &Dali::Path::Sample)
    .function("getPoint", &Dali::Path::GetPoint)
    .function("getControlPoint", &Dali::Path::GetControlPoint)
    .function("getPointCount", &Dali::Path::GetPointCount)
;

  class_<Dali::KeyFrames>("KeyFrames")
    .constructor<>(&Dali::KeyFrames::New)
    .function("add", select_overload<void (float progress, Property::Value value)>(&Dali::KeyFrames::Add))
    .function("addWithAlpha", &KeyFramesAddWithAlpha)
;

  class_<Dali::Animation>("Animation")
    .constructor<float>(&Dali::Animation::New)
    .function("__animateTo", &AnimateTo)
    .function("__animateBy", &AnimateBy)
    .function("__animateBetween", &AnimateBetween)
    .function("__animatePath", &AnimatePath)
    .function("setDuration", &Dali::Animation::SetDuration)
    .function("getDuration", &Dali::Animation::GetDuration)
    .function("setLooping", &Dali::Animation::SetLooping)
    .function("isLooping", &Dali::Animation::IsLooping)
    .function("setEndAction", &Dali::Animation::SetEndAction)
    .function("getEndAction", &Dali::Animation::GetEndAction)
    .function("setDisconnectAction", &Dali::Animation::SetDisconnectAction)
    .function("getDisconnectAction", &Dali::Animation::GetDisconnectAction)
    .function("setCurrentProgress", &Dali::Animation::SetCurrentProgress)
    .function("getCurrentProgress", &Dali::Animation::GetCurrentProgress)
    .function("setSpeedFactor", &Dali::Animation::SetSpeedFactor)
    .function("getSpeedFactor", &Dali::Animation::GetSpeedFactor)
    .function("setPlayRange", &Dali::Animation::SetPlayRange)
    .function("getPlayRange", &Dali::Animation::GetPlayRange)
    .function("play", &Dali::Animation::Play)
    .function("playFrom", &Dali::Animation::PlayFrom)
    .function("pause", &Dali::Animation::Pause)
    .function("stop", &Dali::Animation::Stop)
    .function("clear", &Dali::Animation::Clear)
;

  class_<Dali::PropertyBuffer>("PropertyBuffer")
    .constructor<Dali::Property::Map&>(Dali::PropertyBuffer::New)
    .function("setData", &SetPropertyBufferDataRaw)
;

  class_<Dali::Geometry>("Geometry")
    .constructor<>(&Dali::Geometry::New)
    .function("addVertexBuffer", &Dali::Geometry::AddVertexBuffer)
    .function("getNumberOfVertexBuffers", &Dali::Geometry::GetNumberOfVertexBuffers)
    .function("setIndexBuffer", &SetIndexBufferDataRaw)
    .function("setType", &Dali::Geometry::SetType)
    .function("getType", &Dali::Geometry::GetType)
;

  class_<Dali::Image>("Image")
;

  class_<Dali::BufferImage, base<Dali::Image> >("BufferImage")
    .constructor<const std::string&, unsigned int, unsigned int, Dali::Pixel::Format>(&BufferImageNew)
;

  class_<Dali::EncodedBufferImage, base<Dali::Image> >("EncodedBufferImage")
    .constructor<const std::string&>(&EncodedBufferImageNew)
;

  class_<Dali::Sampler>("Sampler")
    .constructor<>(&Dali::Sampler::New)
;

  class_<Dali::Shader, base<Dali::Handle>>("Shader")
    .constructor<>(&Dali::Shader::New)
;

  class_<Dali::TextureSet>("TextureSet")
    .constructor<>(&Dali::TextureSet::New)
    .function("setTexture", &Dali::TextureSet::SetTexture)
    .function("setSampler", &Dali::TextureSet::SetSampler)
    .function("getTexture", &Dali::TextureSet::GetTexture)
    .function("getSampler", &Dali::TextureSet::GetSampler)
    .function("getTextureCount", &Dali::TextureSet::GetTextureCount)
;

  class_<Dali::Renderer, base<Dali::Handle>>("Renderer")
    .constructor<>(&Dali::Renderer::New)
    .function("setGeometry", &Dali::Renderer::SetGeometry)
    .function("getGeometry", &Dali::Renderer::GetGeometry)
    .function("SetTextures", &Dali::Renderer::SetTextures)
    .function("SetTextures", &Dali::Renderer::SetTextures)
;

  class_<Dali::ShaderEffect, base<Dali::Handle>>("ShaderEffect")
    .constructor<const std::string&, const std::string&,
                 const std::string&, const std::string&,
                 int >(&CreateShaderEffect)
    .function("setEffectImage", &Dali::ShaderEffect::SetEffectImage)
    .function("__setUniform", &SetUniform)
;

  class_<Dali::Actor, base<Dali::Handle>>("Actor")
    .constructor<>(&Dali::Actor::New)
    .function("add", &Dali::Actor::Add)
    .function("remove", &Dali::Actor::Remove)
    .function("getId", &Dali::Actor::GetId)
    .function("__getParent", &Dali::Actor::GetParent)
    .function("__findChildById", &Dali::Actor::FindChildById)
    .function("__findChildByName", &Dali::Actor::FindChildByName)
    .function("__getChildAt", &Dali::Actor::GetChildAt)
    .function("getChildCount", &Dali::Actor::GetChildCount)
    .function("__screenToLocal",
              select_overload<std::vector<float> (Dali::Actor, float, float)>(&ScreenToLocal))
    .function("addressOf", &AddressOf)
    .function("__connect", &ConnectSignal)
    .function("__setPropertyNotification", &SetPropertyNotification)
    .function("addRenderer", &Dali::Actor::AddRenderer)
    .function("getRendererCount", &Dali::Actor::GetRendererCount)
    .function("removeRenderer",
              select_overload<void(unsigned int)>(&Dali::Actor::RemoveRenderer))
    .function("__getRendererAt", &Dali::Actor::GetRendererAt)
;

  class_<Dali::CameraActor, base<Dali::Actor>>("CameraActor")
    .constructor<>( select_overload<Dali::CameraActor()>(&Dali::CameraActor::New))
;

  class_<Dali::Layer, base<Dali::Actor>>("Layer")
    .constructor<>(&Dali::Layer::New)
    .function("raise", &Dali::Layer::Raise)
    .function("lower", &Dali::Layer::Lower)
;

  class_<Dali::Stage>("Stage")
    .constructor<>(&Dali::Stage::GetCurrent)
    .function("add", &Dali::Stage::Add)
    .function("remove", &Dali::Stage::Remove)
    .function("__getRootLayer", &Dali::Stage::GetRootLayer)
    .function("getLayer", &Dali::Stage::GetLayer)
    .function("getRenderTaskList", &Dali::Stage::GetRenderTaskList)
    .function("setBackgroundColor", &Dali::Stage::SetBackgroundColor)
;

  class_<Dali::RenderTaskList>("RenderTaskList")
    .function("createTask", &Dali::RenderTaskList::CreateTask)
    .function("removeTask", &Dali::RenderTaskList::RemoveTask)
    .function("getTaskCount", &Dali::RenderTaskList::GetTaskCount)
    .function("getTask", &Dali::RenderTaskList::GetTask)
;

  class_<Dali::RenderTask>("RenderTask")
    .function("__getCameraActor", &Dali::RenderTask::GetCameraActor)
    .function("setCameraActor", &Dali::RenderTask::SetCameraActor)
    .function("setSourceActor", &Dali::RenderTask::SetSourceActor)
    .function("setExclusive", &Dali::RenderTask::SetExclusive)
    .function("setInputEnabled", &Dali::RenderTask::SetInputEnabled)
    .function("setViewportPosition", &Dali::RenderTask::SetViewportPosition)
    .function("setViewportSize", &Dali::RenderTask::SetViewportSize)
    .function("getCurrentViewportPosition", &Dali::RenderTask::GetCurrentViewportPosition)
    .function("getCurrentViewportSize", &Dali::RenderTask::GetCurrentViewportSize)
    .function("setClearColor", &Dali::RenderTask::SetClearColor)
    .function("getClearColor", &Dali::RenderTask::GetClearColor)
    .function("setClearEnabled", &Dali::RenderTask::SetClearEnabled)
    .function("getClearEnabled", &Dali::RenderTask::GetClearEnabled)
    .function("screenToLocal",
              select_overload<Dali::Vector2(Dali::RenderTask, Dali::Actor, float, float)>(&ScreenToLocal))
    .function("worldToScreen", &WorldToScreen)
;

}

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali
