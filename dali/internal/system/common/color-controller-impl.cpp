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
#include <dali/internal/system/common/color-controller-impl.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/singleton-service-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace // unnamed namespace
{
const char* COLOR_CONTROLLER_PLUGIN_SO( "libdali-color-controller-plugin.so" );
}

Dali::ColorController ColorController::Get()
{
  Dali::ColorController colorController;

  Dali::SingletonService service( SingletonService::Get() );
  if ( service )
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton( typeid( Dali::ColorController ) );
    if(handle)
    {
      // If so, downcast the handle
      colorController = Dali::ColorController( dynamic_cast< ColorController* >( handle.GetObjectPtr() ) );
    }
    else
    {
      colorController = Dali::ColorController( new ColorController( ) );
      service.Register( typeid( colorController ), colorController );
    }
  }

  return colorController;
}

ColorController::ColorController()
: mLibHandle( NULL ),
  mPlugin( NULL ),
  mCreateColorControllerPtr( NULL )
{
  Initialize();
}

ColorController::~ColorController()
{
  if( mPlugin )
  {
    delete mPlugin;
    mPlugin = NULL;

    if( mLibHandle && dlclose( mLibHandle ) )
    {
      DALI_LOG_ERROR( "Error closing color controller plugin library: %s\n", dlerror() );
    }
  }
}

void ColorController::Initialize()
{
  mLibHandle = dlopen( COLOR_CONTROLLER_PLUGIN_SO, RTLD_LAZY );

  char* error = dlerror();
  if( mLibHandle == NULL || error != NULL )
  {
    DALI_LOG_ERROR( "ColorController::Initialize: dlopen error [%s]\n", error );
    return;
  }

  // load plugin
  mCreateColorControllerPtr = reinterpret_cast< CreateColorControllerFunction >( dlsym( mLibHandle, "CreateColorControllerPlugin" ) );

  error = dlerror();
  if( mCreateColorControllerPtr == NULL || error != NULL )
  {
    DALI_LOG_ERROR( "ColorController::Initialize: Cannot load symbol CreateColorControllerPlugin(): %s\n", error );
    return;
  }

  mPlugin = mCreateColorControllerPtr();
  if( !mPlugin )
  {
    DALI_LOG_ERROR("ColorController::Initialize: Plugin creation failed\n");
    return;
  }
}

bool ColorController::RetrieveColor( const std::string& colorCode, Vector4& colorValue )
{
  if( mPlugin )
  {
    return mPlugin->RetrieveColor( colorCode, colorValue );
  }
  return false;
}

bool ColorController::RetrieveColor( const std::string& colorCode , Vector4& textColor, Vector4& textOutlineColor, Vector4& textShadowColor)
{
  if( mPlugin )
  {
    return mPlugin->RetrieveColor( colorCode, textColor, textOutlineColor, textShadowColor );
  }
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
