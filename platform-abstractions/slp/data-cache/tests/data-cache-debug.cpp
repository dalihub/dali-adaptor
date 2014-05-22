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

// CLASS HEADER
#include "data-cache-debug.h"

#ifdef DATA_CACHE_DEBUG

// INTERNAL INCLUDES
#include "../data-cache-io.h"
#include "../data-compression.h"
#include <dali/integration-api/debug.h>

// EXTERNAL INCLUDES
#include <boost/thread.hpp>
#include <set>

namespace Dali
{

namespace SlpPlatform
{

namespace DataCacheIo
{

namespace
{
const char * const DALI_USER_FONT_CACHE_PATH( DALI_USER_FONT_CACHE_DIR );
const std::string TEST_FILE("test-file");

const unsigned int NUMBER_TEST_ITERATIONS(25000);               ///< number of test iterations to perform per thread
const bool RANDOM_DATA_SIZE(true);                              ///< whether to use random data sizes (up to DATA_SIZE)
const unsigned int MAX_KEY_VALUE(200000);                       ////< number key index value
const unsigned int DATA_SIZE(64);                               ///< maximum data size
const unsigned int MAX_NUMBER_ENTRIES( 200000 );                ///< Maximum number of entries size
const unsigned int ENTRIES_TO_READ_WRITE(1);                    ///< maximum number of entries to read/write in one API call
const Platform::DataCache::CompressionMode COMPRESSION_MODE(  Platform::DataCache::RUN_LENGTH_ENCODING); //or COMPRESSION_OFF );

/**
 * Fills the key vector with random keys between 0 and MAX_KEY_VALUE
 */
void FillVectorWithRandomKeys( Platform::DataCache::KeyVector& keyVector)
{
  // make sure if we are setup to write 20 key/data pairs in a single API call
  // the we have at least 20  unique keys
  DALI_ASSERT_ALWAYS( MAX_KEY_VALUE > ENTRIES_TO_READ_WRITE);

  // create a set of unique keys, then insert them in to the key vector
  typedef std::set< Platform::DataCache::DataKey > KeySet;
  KeySet uniqueKeys;
  while( uniqueKeys.size() != keyVector.size() )
  {
    uniqueKeys.insert(  rand() % MAX_KEY_VALUE );
  }
  int i(0);
  for( KeySet::const_iterator iter =  uniqueKeys.begin(), endIter = uniqueKeys.end(); iter != endIter; ++iter )
  {
    keyVector[i] =  (*iter);
    i++;
  }
}

/**
 * Fill the data vector with random data
 */
void FillVectorWithRandomData( Platform::DataCache::DataVector& dataVector)
{
  for( unsigned int i = 0; i < dataVector.size(); i++ )
  {
    unsigned int length( DATA_SIZE );
    if( RANDOM_DATA_SIZE )
    {
     length =  1 + rand() % DATA_SIZE;
    }
    unsigned char* data =  new unsigned char[length];
    dataVector[i].SetData( data, length );
  }
}

/**
 * delete the data vector, and check the exists flag is correct
 */
void DeleteData(Platform::DataCache::DataVector& dataVector )
{
  for( unsigned int i = 0; i < dataVector.size(); i++ )
  {
    Platform::DataCache::Data& data( dataVector[i] );
    // check the exists flag
    if (data.data && !data.exists)
    {
      DALI_ASSERT_ALWAYS(0 && "data exist flag wrong #1 ");
    }
    if (!data.data && data.exists)
    {
      DALI_ASSERT_ALWAYS(0 && "data exist flag wrong #2 ");
    }
    if (data.data)
    {
      delete []data.data;
    }
  }
}

/**
 * read the number of entries, and get the entry list
 */
unsigned int CheckNumberEntries(  FILE* indexFile, KeyMeta** keyMeta  )
{
  unsigned int numberEntries;

  bool ok = ReadNumberEntries( indexFile, numberEntries );
  if( !ok )
  {
    // ReadEntries will log an error
    DALI_ASSERT_ALWAYS(0);
  }
  // Check there is no data after the header, if there are no entries in the index file
  if( numberEntries == 0 )
  {
    if( fgetc( indexFile ) != EOF)
    {
      // data index is corrupt, it contains more data than are recorded in numberEntries field
      DALI_LOG_ERROR("index file has zero entries, but contains data after header\n");
      DALI_ASSERT_ALWAYS(0);
    }
    return 0;
  }

  // allocate an array of  ( key | offset ) structures
  *keyMeta = new  DataCacheIo::KeyMeta[ numberEntries ];

  // read the index table in to memory
  ok = ReadEntries(indexFile, *keyMeta, 0, numberEntries);
  if( !ok )
  {
    // ReadEntries will log an error
    DALI_ASSERT_ALWAYS(0);
  }
  // check there's no data at the end of the file
  if( fgetc( indexFile ) != EOF)
  {
    // data index is corrupt, it contains more entries than are recorded in numberEntries field
    DALI_LOG_ERROR("index has  more entries than recorded\n");
    DALI_ASSERT_ALWAYS(0);
  }
  return numberEntries;
}

unsigned int GetMaxDataSizeOnFile( unsigned int dataSize )
{
  if( COMPRESSION_MODE == Platform::DataCache::RUN_LENGTH_ENCODING )
  {
    return DataCompression::GetMaximumRleCompressedSize(  dataSize );
  }
  return dataSize;
}

/**
 * Checks every single entry in the data file.
 * This is slow, it should not be performed unless debugging.
 */
void FullFileCheck(  FILE* indexFile,
                     FILE* dataFile,
                     unsigned int maxDataSize)
{
  //  Read entries from index file and make sure the file size is correct
  DataCacheIo::KeyMeta* keyMeta(NULL);
  unsigned int numberEntries = CheckNumberEntries( indexFile, &keyMeta );
  unsigned int maxDataSizeOnFile = GetMaxDataSizeOnFile( maxDataSize );

  // Allocate the file buffer for reading the data, and a decode buffer if the data is compressed
  unsigned char* fileDataBuffer = new unsigned char[ maxDataSizeOnFile ];
  unsigned char* decodeBuffer = new unsigned char[ maxDataSize ];

  // For each entry, check the data is valid in the
  unsigned int previousOffset = GetHeaderSize();

  std::set< Dali::Platform::DataCache::DataKey  > KeyLookup;

  for(unsigned int i = 0; i < numberEntries; ++i )
  {
    Dali::Platform::DataCache::DataKey key( keyMeta[i].mKey );
    Dali::Platform::DataCache::Data data;

    // check for duplicate keys
    if( KeyLookup.find(key) != KeyLookup.end() )
    {
      printf(" Duplicate Key Found %d \n",key);
      DALI_ASSERT_ALWAYS(0 );
    }
    KeyLookup.insert(key);

    unsigned int offset( keyMeta[i].mOffset);

    // ensure the offset of each entry is correct
    DALI_ASSERT_ALWAYS( previousOffset == offset);

    bool ok = ReadData( dataFile, offset, key, data, fileDataBuffer, maxDataSizeOnFile );
    if( !ok )
    {
      DALI_ASSERT_ALWAYS(0 && "DataCacheIO::ReadData failed");
    }

    unsigned int dataLengthOnFile = data.length;

    if( COMPRESSION_MODE == Platform::DataCache::RUN_LENGTH_ENCODING )
    {
      // try and de-compress it
      std::size_t decodedSize;
      bool ok = DataCompression::DecodeRle( fileDataBuffer, data.length, decodeBuffer, maxDataSize, decodedSize);
      DALI_ASSERT_ALWAYS( ok && " DataCompression::DecodeRle failed");
      data.length = decodedSize;
    }

   // un-comment for list of key / data entries
   // printf(" key = %d, length = %d \n",key,data.length);
    previousOffset = offset + dataLengthOnFile + 8; /* 8 = key + length field */
  }
  delete []keyMeta;
  delete []decodeBuffer;
  delete []fileDataBuffer;
}

void ClearTestFiles( const std::string& indexFileName, const std::string& dataFileName )
{
  FILE* dataFile = DataCacheIo::OpenFile( dataFileName, DataCacheIo::DATA_FILE, DataCacheIo::NO_LOCK, DataCacheIo::READ_WRITE, DataCacheIo::CREATE_IF_MISSING );
  FILE* indexFile = DataCacheIo::OpenFile( indexFileName, DataCacheIo::INDEX_FILE, DataCacheIo::NO_LOCK, DataCacheIo::READ_WRITE, DataCacheIo::CREATE_IF_MISSING);

  ReCreateFiles( indexFile, dataFile, COMPRESSION_MODE);

  fclose( dataFile );
  fclose( indexFile );
}

} // unnamed name space

void DataCacheStressTest(void)
{
  printf("thread started \n");

  static unsigned int apiCalls(0);
  std::string path(DALI_USER_FONT_CACHE_PATH);
  std::string file = path + TEST_FILE ;

  Platform::DataCache* cache =  Platform::DataCache::New(   Platform::DataCache::READ_WRITE,
                                          COMPRESSION_MODE,
                                          file,
                                          DATA_SIZE,
                                          MAX_NUMBER_ENTRIES);


  for(unsigned int i = 0; i < NUMBER_TEST_ITERATIONS; ++i )
  {
    apiCalls+=2;
    if( i%50 == 0)
    {
      printf("DataCache Add() & Find() calls: %d \r",apiCalls);
    }

    // Read a random array of key/data pairs
    Platform::DataCache::KeyVector keyVector;
    Platform::DataCache::DataVector dataVector;

    unsigned int arraySize  =  ENTRIES_TO_READ_WRITE;
    keyVector.resize( arraySize );

    // read a random entry
    FillVectorWithRandomKeys( keyVector );
    cache->Find( keyVector, dataVector);
    DeleteData( dataVector);

    // Write a random entry
    FillVectorWithRandomKeys( keyVector );
    FillVectorWithRandomData( dataVector );
    cache->Add( keyVector, dataVector );
    DeleteData(dataVector);
  }
  delete cache;
}

void ThreadedStressTest()
{
  //  only allow test to run once
  static int done(false);
  if( done )
  {
    return;
  }
  done = true;

  std::string path(DALI_USER_FONT_CACHE_PATH);
  std::string dataFileName = path + TEST_FILE + ".data";
  std::string indexFileName = path + TEST_FILE + ".index";

  // Make sure the data files are empty to start with
  ClearTestFiles( indexFileName  , dataFileName);


  printf("____ DataCache Multi Thread Test Starting ____ \n");

  boost::thread t1(DataCacheStressTest);
  boost::thread t2(DataCacheStressTest);
  boost::thread t3(DataCacheStressTest);
  boost::thread t4(DataCacheStressTest);
  boost::thread t5(DataCacheStressTest);
  boost::thread t6(DataCacheStressTest);
  boost::thread t7(DataCacheStressTest);
  boost::thread t8(DataCacheStressTest);

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  t6.join();
  t7.join();
  t8.join();

  // Check the data that was written is not corrupt

  FILE* dataFile = OpenFile( dataFileName, DataCacheIo::DATA_FILE, DataCacheIo::NO_LOCK, DataCacheIo::READ_ONLY );
  FILE* indexFile = OpenFile( indexFileName, DataCacheIo::INDEX_FILE, DataCacheIo::NO_LOCK, DataCacheIo::READ_ONLY );

  FullFileCheck( indexFile, dataFile, DATA_SIZE );

  fclose( dataFile );
  fclose( indexFile );

  printf("____ DataCache Multi Thread Test PASSED ____ \n");
}

} // namespace DataCacheIO

} // namespace SlpPlatform

} // namespace Dali
#endif // #ifdef DATA_CACHE_DEBUG
