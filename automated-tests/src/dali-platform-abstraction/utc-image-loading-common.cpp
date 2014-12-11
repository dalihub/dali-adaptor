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

#include "utc-image-loading-common.h"

/** Live platform abstraction recreated for each test case. */
Integration::PlatformAbstraction * gAbstraction = 0;

/** A variety of ImageAttributes to reach different code paths that have embedded code paths. */
std::vector<ImageAttributes> gCancelAttributes;

void utc_dali_loading_startup(void)
{
  test_return_value = TET_UNDEF;
  gAbstraction = CreatePlatformAbstraction();

  // Setup some ImageAttributes to engage post-processing stages:

  ImageAttributes scaleToFillAttributes;
  scaleToFillAttributes.SetScalingMode( ImageAttributes::ScaleToFill );
  scaleToFillAttributes.SetSize( 160, 120 );
  gCancelAttributes.push_back( scaleToFillAttributes );

  // Hit the derived dimensions code:
  ImageAttributes scaleToFillAttributesDeriveWidth = scaleToFillAttributes;
  scaleToFillAttributesDeriveWidth.SetSize( 0, 120 );
  gCancelAttributes.push_back( scaleToFillAttributesDeriveWidth );

  ImageAttributes scaleToFillAttributesDeriveHeight = scaleToFillAttributes;
  scaleToFillAttributesDeriveHeight.SetSize( 160, 0 );
  gCancelAttributes.push_back( scaleToFillAttributesDeriveHeight );

  // Try to push a tall crop:
  ImageAttributes scaleToFillAttributesTall = scaleToFillAttributes;
  scaleToFillAttributesTall.SetSize( 160, 480 );
  ImageAttributes scaleToFillAttributesTall2 = scaleToFillAttributes;
  scaleToFillAttributesTall2.SetSize( 160, 509 );
  ImageAttributes scaleToFillAttributesTall3 = scaleToFillAttributes;
  scaleToFillAttributesTall3.SetSize( 37, 251 );
  gCancelAttributes.push_back( scaleToFillAttributesTall );
  gCancelAttributes.push_back( scaleToFillAttributesTall2 );
  gCancelAttributes.push_back( scaleToFillAttributesTall3 );

  // Try to push a wide crop:
  ImageAttributes scaleToFillAttributesWide = scaleToFillAttributes;
  scaleToFillAttributesWide.SetSize( 320, 60 );
  ImageAttributes scaleToFillAttributesWide2 = scaleToFillAttributes;
  scaleToFillAttributesWide2.SetSize( 317, 60 );
  ImageAttributes scaleToFillAttributesWide3 = scaleToFillAttributes;
  scaleToFillAttributesWide3.SetSize( 317, 53 );
  gCancelAttributes.push_back( scaleToFillAttributesWide );
  gCancelAttributes.push_back( scaleToFillAttributesWide2 );
  gCancelAttributes.push_back( scaleToFillAttributesWide3 );

  ImageAttributes shrinkToFitAttributes = scaleToFillAttributes;
  shrinkToFitAttributes.SetScalingMode( ImageAttributes::ShrinkToFit );
  gCancelAttributes.push_back( shrinkToFitAttributes );

  ImageAttributes fitWidthAttributes = scaleToFillAttributes;
  fitWidthAttributes.SetScalingMode( ImageAttributes::FitWidth );
  gCancelAttributes.push_back( fitWidthAttributes );

  ImageAttributes fitHeightAttributes = scaleToFillAttributes;
  fitHeightAttributes.SetScalingMode( ImageAttributes::FitHeight );
  gCancelAttributes.push_back( fitHeightAttributes );

  ///@ToDo: Add attribute variants for all scale modes.

  // Pad the array to a prime number to mitigate any accidental periodic
  // patterns in which image file has which attributes applied to its load:
  srand48( 104729 );
  const float lastUniques = gCancelAttributes.size() - 0.001f;
  while( gCancelAttributes.size() < 61u )
  {
    gCancelAttributes.push_back( gCancelAttributes[unsigned(drand48() * lastUniques)] );
  }
}

void utc_dali_loading_cleanup(void)
{
  delete gAbstraction;
  gAbstraction = 0;

  test_return_value = TET_PASS;
}
