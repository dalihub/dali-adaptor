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

// EXTERNAL INCLUDES
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <dali-test-suite-utils.h>
#include "platform-abstractions/tizen/image-loaders/loader-ktx.h"

// INTERNAL INCLUDES
#include "image-loaders.h"

using namespace Dali;

/**
 * This class encapsulates knowledge of testing compressed files.
 * It requires a few input parameters per test to confirm if the file was read and understood.
 * The fixture guarantees that each test performed is setup and closed individually, therefore run order does not matter.
 */
class KtxTestFixture
{
  public:

    KtxTestFixture() {}
    ~KtxTestFixture() {}

    /**
     * This struct contains any per-test parameters.
     * This should be added to if more properties of a file/format should be tested.
     */
    struct TestEntry
    {
        std::string filename; ///< Name of the compressed texture KTX file to load.
        int expectedWidth;    ///< The width the texture should be.
        int expectedHeight;   ///< The height the KTX texture should be.

        TestEntry( std::string newFilename, int newExpectedWidth, int newExpectedHeight )
        : filename( newFilename ),
          expectedWidth( newExpectedWidth ),
          expectedHeight( newExpectedHeight )
        {
        }
    };

  private:

    typedef std::vector< TestEntry > TestContainer;

  public:

    /**
     * Adds a test to be performed.
     * @param[in] testEntry A TestEntry struct containing all the details to perform one test.
     */
    void AddTest( TestEntry testEntry )
    {
        mTests.push_back( testEntry );
    }

    /**
     * Runs all tests created with "AddTest".
     * This will create failures upon failing tests.
     */
    void RunTests()
    {
      for( TestContainer::iterator testIterator = mTests.begin(); testIterator != mTests.end(); ++testIterator )
      {
        const TestEntry& currentTest = *testIterator;

        RunTest( currentTest );
      }
    }

  private:

    /**
     * Sets up, Runs and Closes-down an individual test.
     * @param[in] testEntry A TestEntry struct containing all the details to perform one test.
     */
    void RunTest( const TestEntry& testEntry )
    {
      FILE* fileDescriptor = fopen( testEntry.filename.c_str(), "rb" );
      AutoCloseFile autoClose( fileDescriptor );
      DALI_TEST_CHECK( fileDescriptor != NULL );

      // Check the header file.
      unsigned int width( 0 ), height( 0 );
      const Dali::TizenPlatform::ImageLoader::Input input( fileDescriptor );

      DALI_TEST_CHECK( TizenPlatform::LoadKtxHeader( input, width, height ) );

      DALI_TEST_EQUALS( width,  testEntry.expectedWidth,  TEST_LOCATION );
      DALI_TEST_EQUALS( height, testEntry.expectedHeight, TEST_LOCATION );
    }

  private:

    TestContainer mTests;    ///< Holds all tests to be run.

};


int UtcDaliKtxLoaderETC(void)
{
  KtxTestFixture fixture;

  fixture.AddTest( KtxTestFixture::TestEntry( TEST_IMAGE_DIR "/fractal-compressed-ETC1_RGB8_OES-45x80.ktx", 45u, 80u ) );
  fixture.AddTest( KtxTestFixture::TestEntry( TEST_IMAGE_DIR "/fractal-compressed-RGB8_ETC2-45x80.ktx", 45u, 80u ) );

  fixture.RunTests();

  END_TEST;
}

int UtcDaliKtxLoaderPVRTC(void)
{
  KtxTestFixture fixture;

  fixture.AddTest( KtxTestFixture::TestEntry( TEST_IMAGE_DIR "/fractal-compressed-RGB_PVRTC_4BPPV1_IMG-32x64.ktx", 32u, 64u ) );

  fixture.RunTests();

  END_TEST;
}

int UtcDaliKtxLoaderEAC(void)
{
  KtxTestFixture fixture;

  fixture.AddTest( KtxTestFixture::TestEntry( TEST_IMAGE_DIR "/fractal-compressed-R11_EAC-45x80.ktx", 45u, 80u ) );

  fixture.RunTests();

  END_TEST;
}

int UtcDaliKtxLoaderASTC(void)
{
  KtxTestFixture fixture;

  fixture.AddTest( KtxTestFixture::TestEntry( TEST_IMAGE_DIR "/fractal-compressed-RGBA_ASTC_4x4_KHR-32x64.ktx", 32u, 64u ) );
  fixture.AddTest( KtxTestFixture::TestEntry( TEST_IMAGE_DIR "/fractal-compressed-SRBG8_ALPHA8_ASTC_4x4_KHR-32x64.ktx", 32u, 64u ) );

  fixture.RunTests();

  END_TEST;
}
