/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <iostream>

#include <stdlib.h>
#include <dali/public-api/dali-core.h>

#include <dali-test-suite-utils.h>

// Internal headers are allowed here

#include <platform-abstractions/portable/pixel-manipulation.h>

using namespace Dali;
using namespace Dali::Internal::Adaptor;
void utc_dali_internal_pixel_data_startup()
{
  test_return_value = TET_UNDEF;
}

void utc_dali_internal_pixel_data_cleanup()
{
  test_return_value = TET_PASS;
}

const char* ChannelToString( Dali::Internal::Adaptor::Channel channel )
{
  switch(channel)
  {
    case LUMINANCE: return "Luminance";
    case RED: return "Red";
    case GREEN: return "Green";
    case BLUE: return "Blue";
    case ALPHA: return "Alpha";
    default:
      return "Unknown";
  }
}

const char* FormatToString( Dali::Pixel::Format format )
{
  switch(format)
  {
    case Dali::Pixel::A8: return "A8";
    case Dali::Pixel::L8: return "L8";
    case Dali::Pixel::LA88: return "LA88";
    case Dali::Pixel::RGB565: return "RGB565";
    case Dali::Pixel::BGR565: return "BGR565";
    case Dali::Pixel::RGBA4444: return "RGBA4444";
    case Dali::Pixel::BGRA4444: return "BGRA4444";
    case Dali::Pixel::RGBA5551: return "RGBA5551";
    case Dali::Pixel::BGRA5551: return "BGRA5551";

    case Dali::Pixel::RGB888: return "RGB888";
    case Dali::Pixel::RGBA8888: return "RGBA8888";
    case Dali::Pixel::BGRA8888: return "BGRA8888";

    default:
      return "Unknown";
  }
}


int UtcDaliPixelManipulation01(void)
{
  tet_infoline("Testing Dali::Internal::AdaptorManipulation HasChannel");

  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::A8, Dali::Internal::Adaptor::ALPHA ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::A8, Dali::Internal::Adaptor::LUMINANCE ), false, TEST_LOCATION );

  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::L8, Dali::Internal::Adaptor::LUMINANCE ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::L8, Dali::Internal::Adaptor::ALPHA ), false, TEST_LOCATION );

  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::LA88, Dali::Internal::Adaptor::LUMINANCE ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::LA88, Dali::Internal::Adaptor::ALPHA ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::LA88, Dali::Internal::Adaptor::RED ), false, TEST_LOCATION );

  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::RGB565, Dali::Internal::Adaptor::RED ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::RGB565, Dali::Internal::Adaptor::GREEN ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::RGB565, Dali::Internal::Adaptor::BLUE ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::RGB565, Dali::Internal::Adaptor::LUMINANCE ), false, TEST_LOCATION );

  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::RGBA8888, Dali::Internal::Adaptor::RED ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::RGBA8888, Dali::Internal::Adaptor::GREEN ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::RGBA8888, Dali::Internal::Adaptor::BLUE ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::RGBA8888, Dali::Internal::Adaptor::ALPHA ), true, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::RGBA8888, Dali::Internal::Adaptor::LUMINANCE ), false, TEST_LOCATION );

  DALI_TEST_EQUALS( Dali::Internal::Adaptor::HasChannel( Dali::Pixel::COMPRESSED_RGBA_ASTC_10x6_KHR, Dali::Internal::Adaptor::BLUE ), false, TEST_LOCATION );

  END_TEST;
}



int UtcDaliPixelManipulation02(void)
{
  tet_infoline("Testing Dali::Internal::AdaptorManipulation Read/WriteChannel");

  unsigned char pixel[4] = {0,0,0,0};

  for( int formatIdx=1; formatIdx<Dali::Pixel::COMPRESSED_R11_EAC; ++formatIdx)
  {
    pixel[0] = 0xFF;
    pixel[1] = 0xFF;
    pixel[2] = 0xFF;
    pixel[3] = 0xFF;

    for( int channelIdx=0; channelIdx < Dali::Internal::Adaptor::MAX_NUMBER_OF_CHANNELS; ++channelIdx )
    {
      Dali::Pixel::Format format = static_cast<Dali::Pixel::Format>(formatIdx);
      Dali::Internal::Adaptor::Channel channel = static_cast<Dali::Internal::Adaptor::Channel>(channelIdx);
      if( Dali::Internal::Adaptor::HasChannel( format, channel ) )
      {
        Dali::Internal::Adaptor::WriteChannel( &pixel[0], format, channel, 0x15);
        unsigned int value = Dali::Internal::Adaptor::ReadChannel( &pixel[0], format, channel );

        tet_printf( "Testing writing and reading to %s channel in %s format:\n",
                      ChannelToString(channel), FormatToString(format) );

        if( channel == Dali::Internal::Adaptor::ALPHA && (format == Dali::Pixel::RGBA5551 || format == Dali::Pixel::BGRA5551 ) )
        {
          DALI_TEST_EQUALS( value, 0x1, TEST_LOCATION );
        }
        else if( format == Dali::Pixel::RGBA4444 || format == Dali::Pixel::BGRA4444 )
        {
          DALI_TEST_EQUALS( value, 0x05, TEST_LOCATION );
        }
        else
        {
          DALI_TEST_EQUALS( value, 0x15, TEST_LOCATION );
        }
      }
    }
  }

  END_TEST;
}


int UtcDaliPixelManipulation03N(void)
{
  tet_infoline("Testing Dali::Internal::AdaptorManipulation Read/WriteChannel");

  unsigned char pixel[4] = {0,0,0,0};

  for( int formatIdx=1; formatIdx<Dali::Pixel::COMPRESSED_R11_EAC; ++formatIdx)
  {
    pixel[0] = 0xFF;
    pixel[1] = 0xFF;
    pixel[2] = 0xFF;
    pixel[3] = 0xFF;

    for( int channelIdx=0; channelIdx < Dali::Internal::Adaptor::MAX_NUMBER_OF_CHANNELS; ++channelIdx )
    {
      Dali::Pixel::Format format = static_cast<Dali::Pixel::Format>(formatIdx);
      Dali::Internal::Adaptor::Channel channel = static_cast<Dali::Internal::Adaptor::Channel>(channelIdx);
      if( ! Dali::Internal::Adaptor::HasChannel( format, channel ) )
      {
        unsigned int value = Dali::Internal::Adaptor::ReadChannel( &pixel[0], format, channel );

        tet_printf( "Testing reading from %s channel in %s format:\n",
                      ChannelToString(channel), FormatToString(format) );

        DALI_TEST_EQUALS( value, 0x00, TEST_LOCATION );
      }
    }
  }

  END_TEST;
}
