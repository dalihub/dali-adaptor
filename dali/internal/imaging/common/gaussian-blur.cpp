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
 */

// EXTERNAL INCLUDES
#include <memory.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/gaussian-blur.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{



void ConvoluteAndTranspose( unsigned char* inBuffer,
                            unsigned char* outBuffer,
                            const unsigned int bufferWidth,
                            const unsigned int bufferHeight,
                            const float blurRadius )
{
  // Calculate the weights for gaussian blur
  int radius = static_cast<int>( std::ceil( blurRadius ) );
  int rows = radius * 2 + 1;

  float sigma = ( blurRadius < Math::MACHINE_EPSILON_1 ) ? 0.0f : blurRadius * 0.4f + 0.6f; // The same equation used by Android
  float sigma22 = 2.0f * sigma * sigma;
  float sqrtSigmaPi2 = std::sqrt( 2.0f * Math::PI ) * sigma;
  float radius2 = radius * radius;
  float normalizeFactor = 0.0f;

  float* weightMatrix = new float[rows];
  int index = 0;

  for ( int row = -radius; row <= radius; row++ )
  {
    float distance = row * row;
    if ( distance > radius2 )
    {
      weightMatrix[index] = 0.0f;
    }
    else
    {
      weightMatrix[index] = static_cast<float>( std::exp( -( distance ) / sigma22 ) / sqrtSigmaPi2 );
    }
    normalizeFactor += weightMatrix[index];
    index++;
  }

  for ( int i = 0; i < rows; i++ )
  {
    weightMatrix[i] /= normalizeFactor;
  }

  // Perform the convolution and transposition using the weights
  int columns = rows;
  int columns2 = columns / 2;
  for ( unsigned int y = 0; y < bufferHeight; y++ )
  {
    unsigned int targetPixelIndex = y;
    unsigned int ioffset = y * bufferWidth;
    for ( unsigned int x = 0; x < bufferWidth; x++ )
    {
      float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
      int weightColumnOffset = columns2;
      for ( int column = -columns2; column <= columns2; column++ )
      {
        float weight = weightMatrix[weightColumnOffset + column];
        if ( fabsf( weight ) > Math::MACHINE_EPSILON_1 )
        {
          int ix = x + column;
          ix = std::max( 0, std::min( ix, static_cast<int>( bufferWidth - 1 ) ) );
          unsigned int sourcePixelIndex = ioffset + ix;
          r += weight * inBuffer[sourcePixelIndex*4];
          g += weight * inBuffer[sourcePixelIndex*4+1];
          b += weight * inBuffer[sourcePixelIndex*4+2];
          a += weight * inBuffer[sourcePixelIndex*4+3];
        }
      }

      outBuffer[targetPixelIndex*4] = std::max( 0, std::min( static_cast<int>( r + 0.5f ), 255 ) );
      outBuffer[targetPixelIndex*4+1] = std::max( 0, std::min( static_cast<int>( g + 0.5f ), 255 ) );
      outBuffer[targetPixelIndex*4+2] = std::max( 0, std::min( static_cast<int>( b + 0.5f ), 255 ) );
      outBuffer[targetPixelIndex*4+3] = std::max( 0, std::min( static_cast<int>( a + 0.5f ), 255 ) );

      targetPixelIndex += bufferHeight;
    }
  }

  delete [] weightMatrix;
}

void PerformGaussianBlurRGBA( PixelBuffer& buffer, const float blurRadius )
{
  unsigned int bufferWidth = buffer.GetWidth();
  unsigned int bufferHeight = buffer.GetHeight();

  // Create a temporary buffer for the two-pass blur
  PixelBufferPtr softShadowImageBuffer = PixelBuffer::New( bufferWidth, bufferHeight, Pixel::RGBA8888 );
  memcpy( softShadowImageBuffer->GetBuffer(), buffer.GetBuffer(), 4u * bufferWidth * bufferHeight );

  // We perform the blur first but write its output image buffer transposed, so that we
  // can just do it in two passes. The first pass blurs horizontally and transposes, the
  // second pass does the same, but as the image is now transposed, it's really doing a
  // vertical blur. The second transposition makes the image the right way up again. This
  // is much faster than doing a 2D convolution.
  ConvoluteAndTranspose( buffer.GetBuffer(), softShadowImageBuffer->GetBuffer(), bufferWidth, bufferHeight, blurRadius );
  ConvoluteAndTranspose( softShadowImageBuffer->GetBuffer(), buffer.GetBuffer(), bufferHeight, bufferWidth, blurRadius );

  // On leaving scope, softShadowImageBuffer will get destroyed.
}

} //namespace Adaptor

}// namespace Internal

}// namespace Dali
