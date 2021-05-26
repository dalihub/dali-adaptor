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

#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/internal/text/text-abstraction/font-client-helper.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

using namespace Dali;

int UtcDaliFontClient(void)
{
  const int ORDERED_VALUES[] = {-1, 50, 63, 75, 87, 100, 113, 125, 150, 200};

  const unsigned int NUM_OF_ORDERED_VALUES = sizeof(ORDERED_VALUES) / sizeof(int);

  TestApplication application;
  int             result = 0;

  tet_infoline("UtcDaliFontClient No table");
  result = TextAbstraction::Internal::ValueToIndex(100, NULL, 0u);
  DALI_TEST_EQUALS(-1, result, TEST_LOCATION);

  tet_infoline("UtcDaliFontClient Non defined values");
  result = TextAbstraction::Internal::ValueToIndex(-1, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(0, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(-3, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(0, result, TEST_LOCATION);

  tet_infoline("UtcDaliFontClient Between non defined and first of range.");
  result = TextAbstraction::Internal::ValueToIndex(0, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(1, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(30, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(1, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(49, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(1, result, TEST_LOCATION);

  tet_infoline("UtcDaliFontClient Defined in range");
  for(unsigned int index = 1u; index < NUM_OF_ORDERED_VALUES; ++index)
  {
    result = TextAbstraction::Internal::ValueToIndex(ORDERED_VALUES[index], ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
    DALI_TEST_EQUALS(index, result, TEST_LOCATION);
  }

  tet_infoline("UtcDaliFontClient Non defined in range");
  result = TextAbstraction::Internal::ValueToIndex(51, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(1, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(55, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(1, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(62, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(2, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(64, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(2, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(151, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(8, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(175, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(9, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(176, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(9, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(199, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(9, result, TEST_LOCATION);

  tet_infoline("UtcDaliFontClient above of range");
  result = TextAbstraction::Internal::ValueToIndex(220, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(9, result, TEST_LOCATION);
  result = TextAbstraction::Internal::ValueToIndex(500, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u);
  DALI_TEST_EQUALS(9, result, TEST_LOCATION);

  END_TEST;
}

int UtcDaliFontClientAtlasLimitation(void)
{
  TestApplication             application;
  bool                        result     = 0;
  TextAbstraction::FontClient fontClient = TextAbstraction::FontClient::Get();

  tet_infoline("UtcDaliFontClientAtlasLimitation Default");
  result = fontClient.IsAtlasLimitationEnabled();
  DALI_TEST_EQUALS(TextAbstraction::FontClient::DEFAULT_ATLAS_LIMITATION_ENABLED, result, TEST_LOCATION);

  tet_infoline("UtcDaliFontClientAtlasLimitation Enabled");
  fontClient.EnableAtlasLimitation(true);
  result = fontClient.IsAtlasLimitationEnabled();
  DALI_TEST_EQUALS(true, result, TEST_LOCATION);

  tet_infoline("UtcDaliFontClientAtlasLimitation Disabled");
  fontClient.EnableAtlasLimitation(false);
  result = fontClient.IsAtlasLimitationEnabled();
  DALI_TEST_EQUALS(false, result, TEST_LOCATION);
  END_TEST;
}

const std::string DEFAULT_FONT_DIR("/resources/fonts");
const uint32_t    MAX_WIDTH_FIT_IN_ATLAS  = TextAbstraction::FontClient::MAX_TEXT_ATLAS_WIDTH - TextAbstraction::FontClient::PADDING_TEXT_ATLAS_BLOCK;
const uint32_t    MAX_HEIGHT_FIT_IN_ATLAS = TextAbstraction::FontClient::MAX_TEXT_ATLAS_HEIGHT - TextAbstraction::FontClient::PADDING_TEXT_ATLAS_BLOCK;

int UtcDaliFontClientAtlasLimitationEnabled(void)
{
  TestApplication application;

  char*             pathNamePtr = get_current_dir_name();
  const std::string pathName(pathNamePtr);
  free(pathNamePtr);

  TextAbstraction::FontClient fontClient = TextAbstraction::FontClient::Get();
  fontClient.EnableAtlasLimitation(true);

  // The font file "DejaVuSans.ttf" is copied from Toolkit to Adaptor to make test-case of specific font-family.
  // Note the block size depends on font-family.
  // This is to create FontDescription and pass it to GetFontId with point-size (various cases).
  TextAbstraction::FontDescription fontDescription;
  fontDescription.path   = pathName + DEFAULT_FONT_DIR + "/dejavu/DejaVuSans.ttf";
  fontDescription.family = "DejaVuSans";
  fontDescription.width  = TextAbstraction::FontWidth::NONE;
  fontDescription.weight = TextAbstraction::FontWeight::NORMAL;
  fontDescription.slant  = TextAbstraction::FontSlant::NONE;

  // Block's width or height are less than 512
  tet_infoline("UtcDaliFontClientAtlasLimitationEnabled PointSize=200");
  uint32_t                                     pointSize200 = 200 * TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE;
  TextAbstraction::FontId                      fontId200    = fontClient.GetFontId(fontDescription, pointSize200);
  TextAbstraction::FontClient::GlyphBufferData glyphBufferData200;
  glyphBufferData200.width  = 0;
  glyphBufferData200.height = 0;
  fontClient.CreateBitmap(fontId200, 68, false, false, glyphBufferData200, 0);

  DALI_TEST_GREATER(MAX_WIDTH_FIT_IN_ATLAS, glyphBufferData200.width, TEST_LOCATION);
  DALI_TEST_GREATER(MAX_HEIGHT_FIT_IN_ATLAS, glyphBufferData200.height, TEST_LOCATION);

  // Block's width or height are  greater than 512 and less than 1024
  uint32_t pointSize1000 = 1000 * TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE;
  tet_infoline("UtcDaliFontClientAtlasLimitationEnabled PointSize=1000");
  TextAbstraction::FontId                      fontId1000 = fontClient.GetFontId(fontDescription, pointSize1000);
  TextAbstraction::FontClient::GlyphBufferData glyphBufferData1000;
  glyphBufferData1000.width  = 0;
  glyphBufferData1000.height = 0;
  fontClient.CreateBitmap(fontId1000, 68, false, false, glyphBufferData1000, 0);

  DALI_TEST_GREATER(MAX_WIDTH_FIT_IN_ATLAS, glyphBufferData1000.width, TEST_LOCATION);
  DALI_TEST_GREATER(MAX_HEIGHT_FIT_IN_ATLAS, glyphBufferData1000.height, TEST_LOCATION);

  // Block's width or height are greater than 1024 and less than 2048
  uint32_t pointSize2000 = 2000 * TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE;
  tet_infoline("UtcDaliFontClientAtlasLimitationEnabled PointSize=2000");
  TextAbstraction::FontId                      fontId2000 = fontClient.GetFontId(fontDescription, pointSize2000);
  TextAbstraction::FontClient::GlyphBufferData glyphBufferData2000;
  glyphBufferData2000.width  = 0;
  glyphBufferData2000.height = 0;
  fontClient.CreateBitmap(fontId2000, 68, false, false, glyphBufferData2000, 0);

  DALI_TEST_GREATER(MAX_WIDTH_FIT_IN_ATLAS, glyphBufferData2000.width, TEST_LOCATION);
  DALI_TEST_GREATER(MAX_HEIGHT_FIT_IN_ATLAS, glyphBufferData2000.height, TEST_LOCATION);

  END_TEST;
}

int UtcDaliFontClientAtlasLimitationDisabled(void)
{
  TestApplication application;

  char*             pathNamePtr = get_current_dir_name();
  const std::string pathName(pathNamePtr);
  free(pathNamePtr);

  TextAbstraction::FontClient fontClient = TextAbstraction::FontClient::Get();
  fontClient.EnableAtlasLimitation(false);

  TextAbstraction::FontDescription fontDescription;
  fontDescription.path   = pathName + DEFAULT_FONT_DIR + "/dejavu/DejaVuSans.ttf";
  fontDescription.family = "DejaVuSans";
  fontDescription.width  = TextAbstraction::FontWidth::NONE;
  fontDescription.weight = TextAbstraction::FontWeight::NORMAL;
  fontDescription.slant  = TextAbstraction::FontSlant::NONE;

  // Block's width or height are less than 512
  tet_infoline("UtcDaliFontClientAtlasLimitationDisabled PointSize=200");
  uint32_t                                     pointSize200 = 200 * TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE;
  TextAbstraction::FontId                      fontId200    = fontClient.GetFontId(fontDescription, pointSize200);
  TextAbstraction::FontClient::GlyphBufferData glyphBufferData200;
  glyphBufferData200.width  = 0;
  glyphBufferData200.height = 0;
  fontClient.CreateBitmap(fontId200, 68, false, false, glyphBufferData200, 0);

  DALI_TEST_GREATER(512u, glyphBufferData200.width, TEST_LOCATION);  //93u
  DALI_TEST_GREATER(512u, glyphBufferData200.height, TEST_LOCATION); //115u

  // Block's width or height are  greater than 512 and less than 1024
  tet_infoline("UtcDaliFontClientAtlasLimitationDisabled PointSize=1000");
  uint32_t                                     pointSize1000 = 1000 * TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE;
  TextAbstraction::FontId                      fontId1000    = fontClient.GetFontId(fontDescription, pointSize1000);
  TextAbstraction::FontClient::GlyphBufferData glyphBufferData1000;
  glyphBufferData1000.width  = 0;
  glyphBufferData1000.height = 0;
  fontClient.CreateBitmap(fontId1000, 68, false, false, glyphBufferData1000, 0);

  DALI_TEST_GREATER(512u, glyphBufferData1000.width, TEST_LOCATION);  //462u
  DALI_TEST_GREATER(glyphBufferData1000.height, 512u, TEST_LOCATION); //574u

  // Block's width or height are greater than 1024 and less than 2048
  tet_infoline("UtcDaliFontClientAtlasLimitationDisabled PointSize=2000");
  uint32_t                                     pointSize2000 = 2000 * TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE;
  TextAbstraction::FontId                      fontId2000    = fontClient.GetFontId(fontDescription, pointSize2000);
  TextAbstraction::FontClient::GlyphBufferData glyphBufferData2000;
  glyphBufferData2000.width  = 0;
  glyphBufferData2000.height = 0;
  fontClient.CreateBitmap(fontId2000, 68, false, false, glyphBufferData2000, 0);

  DALI_TEST_GREATER(1024u, glyphBufferData2000.width, TEST_LOCATION);  //924u
  DALI_TEST_GREATER(glyphBufferData2000.height, 1024u, TEST_LOCATION); //1148u

  END_TEST;
}

int UtcDaliFontClientCurrentMaximumBlockSizeFitInAtlas(void)
{
  TestApplication application;
  tet_infoline(" UtcDaliFontClientCurrentMaximumBlockSizeFitInAtlas ");

  bool                        isChanged;
  TextAbstraction::FontClient fontClient;
  fontClient = TextAbstraction::FontClient::Get();

  Size defaultTextAtlasSize              = fontClient.GetDefaultTextAtlasSize();
  Size maximumTextAtlasSize              = fontClient.GetMaximumTextAtlasSize();
  Size currentMaximumBlockSizeFitInAtlas = fontClient.GetCurrentMaximumBlockSizeFitInAtlas();

  tet_infoline("CurrentMaximumBlockSizeFitInAtlas start with default ");
  DALI_TEST_EQUALS(currentMaximumBlockSizeFitInAtlas, defaultTextAtlasSize, TEST_LOCATION);

  tet_infoline("SetCurrentMaximumBlockSizeFitInAtlas is changed with current ");
  isChanged = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(currentMaximumBlockSizeFitInAtlas);
  DALI_TEST_CHECK(isChanged);

  Size decreaseOnlyWidth(currentMaximumBlockSizeFitInAtlas.width - 1u, currentMaximumBlockSizeFitInAtlas.height);
  tet_infoline("SetCurrentMaximumBlockSizeFitInAtlas is changed with width < current ");
  isChanged = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(decreaseOnlyWidth);
  DALI_TEST_CHECK(isChanged);

  Size decreaseOnlyHeigth(currentMaximumBlockSizeFitInAtlas.width, currentMaximumBlockSizeFitInAtlas.height - 1u);
  tet_infoline("SetCurrentMaximumBlockSizeFitInAtlas is changed with height < current ");
  isChanged = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(decreaseOnlyHeigth);
  DALI_TEST_CHECK(isChanged);

  Size decreaseBoth(currentMaximumBlockSizeFitInAtlas.width - 1u, currentMaximumBlockSizeFitInAtlas.height - 1u);
  tet_infoline("SetCurrentMaximumBlockSizeFitInAtlas is changed with size < current ");
  isChanged = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(decreaseBoth);
  DALI_TEST_CHECK(isChanged);

  Size increaseOnlyWidth(maximumTextAtlasSize.width + 1u, currentMaximumBlockSizeFitInAtlas.height);
  tet_infoline("SetCurrentMaximumBlockSizeFitInAtlas is not changed with width > max ");
  isChanged = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(increaseOnlyWidth);
  DALI_TEST_CHECK(!isChanged);

  Size increaseOnlyHeigth(currentMaximumBlockSizeFitInAtlas.width, maximumTextAtlasSize.height + 1u);
  tet_infoline("SetCurrentMaximumBlockSizeFitInAtlas is not changed with height > max ");
  isChanged = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(increaseOnlyHeigth);
  DALI_TEST_CHECK(!isChanged);

  Size increaseBoth(maximumTextAtlasSize.width + 1u, maximumTextAtlasSize.height + 1u);
  tet_infoline("SetCurrentMaximumBlockSizeFitInAtlas is not changed with size > max ");
  isChanged = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(increaseBoth);
  DALI_TEST_CHECK(!isChanged);

  currentMaximumBlockSizeFitInAtlas = fontClient.GetCurrentMaximumBlockSizeFitInAtlas();
  if(currentMaximumBlockSizeFitInAtlas.width < maximumTextAtlasSize.width && currentMaximumBlockSizeFitInAtlas.height < maximumTextAtlasSize.height)
  {
    tet_infoline("SetCurrentMaximumBlockSizeFitInAtlas is changed with current < size < max ");
    Size increaseWidth = Size(currentMaximumBlockSizeFitInAtlas.width + 1u, currentMaximumBlockSizeFitInAtlas.height);
    isChanged          = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(increaseWidth);
    DALI_TEST_CHECK(isChanged);
    currentMaximumBlockSizeFitInAtlas = fontClient.GetCurrentMaximumBlockSizeFitInAtlas();

    Size increaseHeigth = Size(currentMaximumBlockSizeFitInAtlas.width, currentMaximumBlockSizeFitInAtlas.height + 1u);
    isChanged           = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(increaseHeigth);
    DALI_TEST_CHECK(isChanged);
    currentMaximumBlockSizeFitInAtlas = fontClient.GetCurrentMaximumBlockSizeFitInAtlas();

    Size sizeLessThanMax = Size(currentMaximumBlockSizeFitInAtlas.width + 1u, currentMaximumBlockSizeFitInAtlas.height + 1u);
    isChanged            = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(sizeLessThanMax);

    if(currentMaximumBlockSizeFitInAtlas.width < maximumTextAtlasSize.width && currentMaximumBlockSizeFitInAtlas.height < maximumTextAtlasSize.height)
    {
      DALI_TEST_CHECK(isChanged);
    }
    else
    {
      DALI_TEST_CHECK(!isChanged);
    }

    currentMaximumBlockSizeFitInAtlas = fontClient.GetCurrentMaximumBlockSizeFitInAtlas();

    DALI_TEST_NOT_EQUALS(currentMaximumBlockSizeFitInAtlas, defaultTextAtlasSize, 0.0f, TEST_LOCATION);
    DALI_TEST_EQUALS(currentMaximumBlockSizeFitInAtlas, sizeLessThanMax, TEST_LOCATION);

    tet_infoline("SetCurrentMaximumBlockSizeFitInAtlas is not changed with size == max ");
    isChanged = fontClient.SetCurrentMaximumBlockSizeFitInAtlas(maximumTextAtlasSize);
    DALI_TEST_CHECK(!isChanged);
  }

  currentMaximumBlockSizeFitInAtlas = fontClient.GetCurrentMaximumBlockSizeFitInAtlas();
  DALI_TEST_NOT_EQUALS(currentMaximumBlockSizeFitInAtlas, maximumTextAtlasSize, 0.0f, TEST_LOCATION);

  END_TEST;
}

int UtcDaliFontClientTextAtlasConstants(void)
{
  TestApplication application;
  tet_infoline(" UtcDaliFontClientTextAtlasConstants ");

  TextAbstraction::FontClient fontClient;
  fontClient = TextAbstraction::FontClient::Get();

  Size defaultTextAtlasSize = fontClient.GetDefaultTextAtlasSize();
  Size defaultTextAtlasWidthHeight(TextAbstraction::FontClient::DEFAULT_TEXT_ATLAS_WIDTH, TextAbstraction::FontClient::DEFAULT_TEXT_ATLAS_HEIGHT);
  DALI_TEST_EQUALS(defaultTextAtlasSize, TextAbstraction::FontClient::DEFAULT_TEXT_ATLAS_SIZE, TEST_LOCATION);
  DALI_TEST_EQUALS(defaultTextAtlasSize, defaultTextAtlasWidthHeight, TEST_LOCATION);

  Size maximumTextAtlasSize = fontClient.GetMaximumTextAtlasSize();
  Size maxTextAtlasWidthHeight(TextAbstraction::FontClient::MAX_TEXT_ATLAS_WIDTH, TextAbstraction::FontClient::MAX_TEXT_ATLAS_HEIGHT);
  DALI_TEST_EQUALS(maximumTextAtlasSize, TextAbstraction::FontClient::MAX_TEXT_ATLAS_SIZE, TEST_LOCATION);
  DALI_TEST_EQUALS(maximumTextAtlasSize, maxTextAtlasWidthHeight, TEST_LOCATION);

  uint32_t numberOfPointsPerOneUnitOfPointSize = fontClient.GetNumberOfPointsPerOneUnitOfPointSize();
  DALI_TEST_EQUALS(numberOfPointsPerOneUnitOfPointSize, TextAbstraction::FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE, TEST_LOCATION);

  END_TEST;
}