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

#include <dali-test-suite-utils.h>

#include "platform-abstractions/slp/resource-loader/loader-gif.h"

#include "image-loaders.h"

using namespace Dali;

static void Startup();
static void Cleanup();

extern "C" {
  void (*tet_startup)() = Startup;
  void (*tet_cleanup)() = Cleanup;
}

enum {
  POSITIVE_TC_IDX = 0x01,
  NEGATIVE_TC_IDX,
};

#define MAX_NUMBER_OF_TESTS 10000
extern "C" {
  struct tet_testlist tet_testlist[MAX_NUMBER_OF_TESTS];
}

TEST_FUNCTION( UtcDaliGifLoaderInterlaced, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliGifLoaderErrorBits, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliGifLoaderPattern, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliGifLoaderTransparency, POSITIVE_TC_IDX );

static const LoadFunctions GifLoaders( SlpPlatform::LoadGifHeader, SlpPlatform::LoadBitmapFromGif );

// Called only once before first test is run.
static void Startup()
{
}

// Called only once after last test is run
static void Cleanup()
{
}

static void UtcDaliGifLoaderInterlaced()
{
  ImageDetails interlaced( "images/interlaced.gif", 365u, 227u );
  TestImageLoading( interlaced, GifLoaders );
}

static void UtcDaliGifLoaderErrorBits()
{
  ImageDetails errorBits( "images/error-bits.gif", 534u, 749u, 1280u, 1024u );
  TestImageLoading( errorBits, GifLoaders );
}

static void UtcDaliGifLoaderPattern()
{
  ImageDetails pattern( "images/pattern.gif", 600u, 600u );
  TestImageLoading( pattern, GifLoaders );
}

static void UtcDaliGifLoaderTransparency()
{
  ImageDetails transparency( "images/transparency.gif", 320u, 280u );
  TestImageLoading( transparency, GifLoaders );
}

