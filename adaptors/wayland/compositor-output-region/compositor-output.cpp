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

// CLASS HEADER
#include "compositor-output.h"

// EXTERNAL INCLUDES
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
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_MONITOR_INFO");
#endif

const float MILLIMETRE_TO_INCH = 1.f / 25.4f;

unsigned int gDpiHorizontal = 75;
unsigned int gDpiVertical = 75;


void OutputGeometryCallback( void *data,
     struct wl_output *wl_output,
     int32_t x,
     int32_t y,
     int32_t physical_width,
     int32_t physical_height,
     int32_t subpixel,
     const char *make,
     const char *model,
     int32_t transform)
{
  CompositorOutput* output = static_cast< CompositorOutput* >( data );
  output->SetMonitorDimensions( physical_width, physical_height );

  DALI_LOG_INFO( gLogFilter, Debug::General, "Monitor width: %d mm, height: %d mm\n",physical_width, physical_height  );
}

void OutputModeCallback( void *data,
       struct wl_output *wl_output,
       uint32_t flags,
       int32_t width,
       int32_t height,
       int32_t refresh )
{

  if (flags & WL_OUTPUT_MODE_CURRENT)
  {
    CompositorOutput* output = static_cast< CompositorOutput* >( data );
    output->SetMonitorResolution( width, height );

    DALI_LOG_INFO( gLogFilter, Debug::General, "Monitor refresh rate: %f Hz, resolution: %d x %d\n", static_cast<float>(refresh)/ 1000.f, width, height );
  }
}

void  OutputCallbackDone(void *data,
       struct wl_output *wl_output)
{
  CompositorOutput* output = static_cast< CompositorOutput* >( data );
  output->CallbacksDone();
}

void OutputCallbackScale(void *data,
        struct wl_output *wl_output,
        int32_t factor)
{
  // Need to understand if we need to support output scaling and what impact it has on input handling etc.
}



//output typically corresponds to a monitor
const struct wl_output_listener OutputListener =
{
   OutputGeometryCallback,
   OutputModeCallback,
   OutputCallbackDone,
   OutputCallbackScale
};
}
CompositorOutput::CompositorOutput()
:mOutput( NULL ),
 mXresolution( 0 ),
 mYresolution( 0 ),
 mMonitorWidth( 0.f ),
 mMonitorHeight( 0.f ),
 mDataReady( false )
{

}

CompositorOutput::~CompositorOutput()
{
  if( mOutput )
  {
    wl_output_destroy( mOutput );
  }
}

void CompositorOutput::AddListener( WlOutput* outputInterface )
{
  mOutput = outputInterface;

  wl_output_add_listener( mOutput , &OutputListener, this);
}

void CompositorOutput::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  dpiHorizontal = gDpiHorizontal;
  dpiVertical = gDpiVertical;
}

bool CompositorOutput::DataReady() const
{
  return mDataReady;
}

void CompositorOutput::SetMonitorDimensions( unsigned int width, unsigned int height )
{
  mMonitorHeight = static_cast<float> ( height ) * MILLIMETRE_TO_INCH;
  mMonitorWidth = static_cast<float>( width ) * MILLIMETRE_TO_INCH;
}

void CompositorOutput::SetMonitorResolution( unsigned int width, unsigned int height )
{
  mXresolution = width;
  mYresolution = height;
}

void CompositorOutput::CallbacksDone()
{
  mDataReady = true;

  // formula for DPI is screen resolution / physical size in inches
  gDpiHorizontal =  (static_cast<float>( mXresolution) / mMonitorWidth) + 0.5f;
  gDpiVertical =  (static_cast<float>( mYresolution )/ mMonitorHeight) +0.5f;

  DALI_LOG_INFO( gLogFilter, Debug::General, "Monitor DPI %d x %d\n", gDpiHorizontal, gDpiVertical );
}




} // Internal
} // Adaptor
} // Dali
