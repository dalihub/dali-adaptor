#ifndef __DALI_TEST_IMG_UTILS_H__
#define __DALI_TEST_IMG_UTILS_H__

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

// EXTERNAL INCLUDES
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>
#include <dali/public-api/dali-core.h>


using namespace Dali;

namespace
{

/**
 * Test whether two buffers are equal with tolerance value.
 * @param[in] buffer1 The first buffer
 * @param[in] buffer2 The second pixelbuffer
 * @param[in] tolerance value, maximum difference to accept the similarity of buffers.
 * @param[in] location The TEST_LOCATION macro should be used here
 */

inline void DALI_TEST_EQUALS( const unsigned char* buffer1, const unsigned char* buffer2, unsigned int tolerance, long size, const char* location)
{
  if( !tolerance )
  {
    if ( memcmp( buffer1, buffer2, size) )
    {
      fprintf(stderr, "%s, checking buffer1 == buffer2\n", location );
      tet_result(TET_FAIL);
    }
    else
    {
      tet_result(TET_PASS);
    }
  }
  else
  {
    const unsigned char* buff1 = buffer1;
    const unsigned char* buff2 = buffer2;
    unsigned int i = 0;
    //Create a mask to fast compare, it is expected to be similar values.
    unsigned int maskBits = 0;
    while( maskBits < tolerance )
    {
      maskBits |= (1 << i);
      i++;
    }
    maskBits &= ~(1 << --i);
    maskBits = ~maskBits;

    bool equal = true;
    for( i = 0; i < size; ++i, ++buff1, ++buff2 )
    {
      //Check bit difference, if exist, do more exhaustive comparison with tolerance value
      if( (*buff1 ^ *buff2 ) & maskBits )
      {
        if( *buff1 < *buff2 )
        {
          unsigned int diff = *buff2 - *buff1;
          if( diff  > tolerance )
          {
            equal = false;
            break;
          }
        }
        else
        {
          unsigned int diff = *buff1 - *buff2;
          if( diff > tolerance )
          {
            equal = false;
            break;
          }
        }
      }
    }
    if ( !equal )
    {
      fprintf(stderr, "%s, byte %d, checking %u == %u\n", location, i, *buff1, *buff2 );
      tet_result(TET_FAIL);
    }
    else
    {
      tet_result(TET_PASS);
    }
  }
}

/**
 * Test whether two pixelbuffers are equal with tolerance value, with check of width and height.
 * @param[in] pixelBuffer1 The first buffer
 * @param[in] pixelBuffer2 The second pixelbuffer
 * @param[in] tolerance value, maximum difference to accept the similarity of pixel buffers.
 * @param[in] location The TEST_LOCATION macro should be used here
 */

inline void DALI_IMAGE_TEST_EQUALS( Dali::Devel::PixelBuffer pixelBuffer1, Dali::Devel::PixelBuffer pixelBuffer2, unsigned int tolerance, const char* location)
{
  if( ( pixelBuffer1.GetPixelFormat() != Pixel::RGB888 ) || ( pixelBuffer2.GetPixelFormat() != Pixel::RGB888 ) )
  {
    fprintf(stderr, "%s, PixelFormat != Pixel::RGB888, test only support Pixel::RGB888 formats\n", location );
    tet_result(TET_FAIL);
  }
  else if( ( pixelBuffer1.GetWidth() != pixelBuffer1.GetWidth() ) || ( pixelBuffer1.GetHeight() != pixelBuffer1.GetHeight() ) )
  {
    fprintf(stderr, "%s, Different Image sizes\n", location );
    tet_result(TET_FAIL);
  }
  else
  {
    DALI_TEST_EQUALS( pixelBuffer1.GetBuffer(), pixelBuffer2.GetBuffer(), tolerance, pixelBuffer1.GetHeight() * pixelBuffer1.GetWidth() * 3, location);
  }
}

}

#endif // __DALI_TEST_SUITE_UTILS_H__
