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
#include <stdint.h>
#include <dali/dali.h>
#include <dali-test-suite-utils.h>
#include <dali/internal/text-abstraction/font-client-helper.h>

using namespace Dali;

int UtcDaliFontClient(void)
{
  const int ORDERED_VALUES[] = { -1, 50, 63, 75, 87, 100, 113, 125, 150, 200 };

  const unsigned int NUM_OF_ORDERED_VALUES = sizeof( ORDERED_VALUES ) / sizeof( int );

  TestApplication application;
  int result=0;

  tet_infoline("UtcDaliFontClient No table");
  result = TextAbstraction::Internal::ValueToIndex( 100, NULL, 0u );
  DALI_TEST_EQUALS( -1, result, TEST_LOCATION );

  tet_infoline("UtcDaliFontClient Non defined values");
  result = TextAbstraction::Internal::ValueToIndex( -1, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 0, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( -3, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 0, result, TEST_LOCATION );

  tet_infoline("UtcDaliFontClient Between non defined and first of range.");
  result = TextAbstraction::Internal::ValueToIndex( 0, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 1, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 30, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 1, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 49, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 1, result, TEST_LOCATION );

  tet_infoline("UtcDaliFontClient Defined in range");
  for( unsigned int index = 1u; index < NUM_OF_ORDERED_VALUES; ++index )
  {
    result = TextAbstraction::Internal::ValueToIndex( ORDERED_VALUES[index], ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
    DALI_TEST_EQUALS( index, result, TEST_LOCATION );
  }

  tet_infoline("UtcDaliFontClient Non defined in range");
  result = TextAbstraction::Internal::ValueToIndex( 51, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 1, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 55, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 1, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 62, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 2, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 64, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 2, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 151, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 8, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 175, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 9, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 176, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 9, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 199, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 9, result, TEST_LOCATION );

  tet_infoline("UtcDaliFontClient above of range");
  result = TextAbstraction::Internal::ValueToIndex( 220, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 9, result, TEST_LOCATION );
  result = TextAbstraction::Internal::ValueToIndex( 500, ORDERED_VALUES, NUM_OF_ORDERED_VALUES - 1u );
  DALI_TEST_EQUALS( 9, result, TEST_LOCATION );

  END_TEST;
}


