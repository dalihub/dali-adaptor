//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <iostream>

#include <stdlib.h>
#include <tet_api.h>
#include <getopt.h>

#include <dali/dali.h>

#include <internal/command-line-options.h>

#include <dali-test-suite-utils.h>

using namespace Dali;
using namespace Dali::Internal::Adaptor;

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

TEST_FUNCTION( UtcDaliCommandLineOptionsNoArgs, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliCommandLineOptionsDaliShortArgs, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliCommandLineOptionsDaliLongArgsEqualsSign, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliCommandLineOptionsDaliLongArgsSpaces, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliCommandLineOptionsNonDaliArgs, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliCommandLineOptionsMixture, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliCommandLineOptionsMixtureDaliOpsAtStart, POSITIVE_TC_IDX );
TEST_FUNCTION( UtcDaliCommandLineOptionsMixtureDaliOpsAtEnd, POSITIVE_TC_IDX );

int gOriginalOptIndValue(0);

// Called only once before first test is run.
static void Startup()
{
  gOriginalOptIndValue = optind;
}

// Called only once after last test is run
static void Cleanup()
{
  optind = gOriginalOptIndValue; // Reset overall value
}

void UtcDaliCommandLineOptionsNoArgs()
{
  optind = 1; // Reset opt for test

  int argc( 1 );
  char* argList[1] = { "program" };
  char** argv = argList;

  CommandLineOptions options( &argc, &argv );

  DALI_TEST_EQUALS( argc, 1, TEST_LOCATION );

  // Check values
  DALI_TEST_EQUALS( options.noVSyncOnRender, 0, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageWidth, 0, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageHeight, 0, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageDPI, "", TEST_LOCATION );

  optind = 1; // Reset opt for test
}

void UtcDaliCommandLineOptionsDaliShortArgs()
{
  optind = 1; // Reset opt for test

  char* argList[] =
  {
      "program",
      "-w", "800",
      "-h", "1000",
      "-d", "4x5",
  };
  int argc( sizeof( argList ) / sizeof( argList[0] ) );
  char** argv = argList;

  CommandLineOptions options( &argc, &argv );

  // Should strip out the height and width
  DALI_TEST_EQUALS( argc, 1, TEST_LOCATION );

  // Check values
  DALI_TEST_EQUALS( options.noVSyncOnRender, 0, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageWidth, 800, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageHeight, 1000, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageDPI, "4x5", TEST_LOCATION );

  optind = 1; // Reset opt for test
}

void UtcDaliCommandLineOptionsDaliLongArgsEqualsSign()
{
  optind = 1; // Reset opt for test

  char* argList[] =
  {
      "program",
      "--width=800",
      "--height=1000",
      "--dpi=3x4",
      "--no-vsync",
      "--help"
  };
  int argc( sizeof( argList ) / sizeof( argList[0] ) );
  char** argv = argList;

  CommandLineOptions options( &argc, &argv );

  // Should strip out the height and width
  DALI_TEST_EQUALS( argc, 1, TEST_LOCATION );

  // Check values
  DALI_TEST_EQUALS( options.noVSyncOnRender, 1, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageWidth, 800, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageHeight, 1000, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageDPI, "3x4", TEST_LOCATION );

  optind = 1; // Reset opt for test
}

void UtcDaliCommandLineOptionsDaliLongArgsSpaces()
{
  optind = 1; // Reset optind

  char* argList[] =
  {
      "program",
      "--width", "800",
      "--height", "1000",
      "--dpi", "3x4",
      "--no-vsync",
      "--help"
  };
  int argc( sizeof( argList ) / sizeof( argList[0] ) );
  char** argv = argList;

  CommandLineOptions options( &argc, &argv );

  // Should strip out the height and width
  DALI_TEST_EQUALS( argc, 1, TEST_LOCATION );

  // Check values
  DALI_TEST_EQUALS( options.noVSyncOnRender, 1, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageWidth, 800, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageHeight, 1000, TEST_LOCATION );
  DALI_TEST_EQUALS( options.stageDPI, "3x4", TEST_LOCATION );

  optind = 1; // Reset opt for test
}

void UtcDaliCommandLineOptionsNonDaliArgs()
{
  optind = 1; // Reset opt for test

  char* argList[] =
  {
      "program",
      "hello-world",
      "-y", "600",
  };
  int argc( sizeof( argList ) / sizeof( argList[0] ) );
  char** argv = argList;

  CommandLineOptions options( &argc, &argv );

  // Should still be the same
  DALI_TEST_EQUALS( argc, 4, TEST_LOCATION );

  // Ensure order has not changed
  DALI_TEST_EQUALS( argList[0], "program", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[1], "hello-world", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[2], "-y", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[3], "600", TEST_LOCATION );

  optind = 1; // Reset opt for test
}

void UtcDaliCommandLineOptionsMixture()
{
  optind = 1; // Reset opt for test

  char* argList[] =
  {
      "program",
      "--width=800",
      "hello-world",
      "-y", "600",
      "--height", "1000",
      "-r",
  };
  int argc( sizeof( argList ) / sizeof( argList[0] ) );
  char** argv = argList;

  CommandLineOptions options( &argc, &argv );

  // Should still be the same
  DALI_TEST_EQUALS( argc, 5, TEST_LOCATION );

  // Ensure order of program name and unhandled options has not changed
  DALI_TEST_EQUALS( argList[0], "program", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[1], "hello-world", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[2], "-y", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[3], "600", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[4], "-r", TEST_LOCATION );

  optind = 1; // Reset opt for test
}

void UtcDaliCommandLineOptionsMixtureDaliOpsAtStart()
{
  optind = 1; // Reset opt for test

  char* argList[] =
  {
      "program",
      "--width=800",
      "--height", "1000",
      "-r",
      "hello-world",
      "-y", "600",
  };
  int argc( sizeof( argList ) / sizeof( argList[0] ) );
  char** argv = argList;

  CommandLineOptions options( &argc, &argv );

  // Should still be the same
  DALI_TEST_EQUALS( argc, 5, TEST_LOCATION );

  // Ensure order of program name and unhandled options has not changed
  DALI_TEST_EQUALS( argList[0], "program", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[1], "-r", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[2], "hello-world", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[3], "-y", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[4], "600", TEST_LOCATION );

  optind = 1; // Reset opt for test
}

void UtcDaliCommandLineOptionsMixtureDaliOpsAtEnd()
{
  optind = 1; // Reset opt for test

  char* argList[] =
  {
      "program",
      "hello-world",
      "-y", "600",
      "-r",
      "--width=800",
      "--height", "1000",
  };
  int argc( sizeof( argList ) / sizeof( argList[0] ) );
  char** argv = argList;

  CommandLineOptions options( &argc, &argv );

  // Should still be the same
  DALI_TEST_EQUALS( argc, 5, TEST_LOCATION );

  // Ensure order of program name and unhandled options has not changed
  DALI_TEST_EQUALS( argList[0], "program", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[1], "hello-world", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[2], "-y", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[3], "600", TEST_LOCATION );
  DALI_TEST_EQUALS( argList[4], "-r", TEST_LOCATION );

  optind = 1; // Reset opt for test
}
