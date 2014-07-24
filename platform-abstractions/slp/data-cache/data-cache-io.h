#ifndef __DALI_SLP_PLATFORM_DATA_CACHE_IO_H__
#define __DALI_SLP_PLATFORM_DATA_CACHE_IO_H__

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

// INTERNAL INCLUDES
#include "../../interfaces/data-cache.h"

namespace Dali
{
namespace SlpPlatform
{

/**
 * Input / Output file routines for the data-cache
 * This file has no dependency on data-cache-impl.h, just the abstract interface data-cache.h
 * For future work it would be possible to replace FILE* with a resource handle so that
 * data-cache-impl would not have to know it is dealing with files.
 *
 */
namespace DataCacheIo
{

/**
 * Type of file
 */
enum FileType
{
  INDEX_FILE,     ///< index file
  DATA_FILE       ///< data file
};

/**
 * File lock mode
 */
enum LockMode
{
  LOCK_FILE,      ///< lock the file
  NO_LOCK         ///< no lock
};

/**
 * File open mode
 */
enum FileOpenMode
{
  READ_ONLY,      ///< read only
  READ_WRITE      ///< read write
};

/**
 * File creation mode
 */
enum FileCreationMode
{
  CREATE_IF_MISSING,        ///< create the file if missing
  DONT_CREATE_IF_MISSING    ///< don't create the file if missing
};


/**
 * Key meta information, holds key offset information for the index file.
 * Contains plain old data (POD).
 *
 * | KEY | OFFSET |
 */
struct KeyMeta
{
  /**
   * Constructor
   */
  KeyMeta():mKey(0),
      mOffset(0)
  {}
  /**
   * Constructor
   */
  KeyMeta( Dali::Platform::DataCache::DataKey key,
      unsigned int offset)
    :mKey(key), mOffset(offset)
  {}

  Dali::Platform::DataCache::DataKey mKey;  ///< Key
  unsigned int mOffset;                     ///< Offset

private:
  // Undefined copy constructor.
  KeyMeta( const KeyMeta& );

  // Undefined assignment operator.
  KeyMeta& operator=( const KeyMeta&);
};

/**
 *  Check the index and data files and repair if required
 *  @param[in] indexFileName index filename
 *  @param[in] dataFileName data filename
 *  @param[in] compressionMode compression mode
 *  @param[in] maxDataSize maximum data size
 *  @param[in] maxNummberEntries maximum number of entries allowed
 */
void CheckAndRepairFiles( const std::string& indexFileName,
                          const std::string& dataFileName,
                          Dali::Platform::DataCache::CompressionMode compressionMode,
                          unsigned int maxDataSize,
                          unsigned int maxNummberEntries );

/**
 * Opens a file.
 * @param[in] filename file name
 * @param[in] fileType file type
 * @param[in] lockMode whether to lock the file or not
 * @param[in] fileMode whether to open the file for reading or read/writing.
 * @param[in] creationMode whether to create the file if it's missing or not
 * @return pointer to a file.
 */
FILE* OpenFile(const std::string& fileName,
              FileType fileType,
              LockMode lockMode,
              FileOpenMode fileMode,
              FileCreationMode creationMode = DONT_CREATE_IF_MISSING);

/**
 * Erases the contents of both index and data files and writes a new header.
 * @param[in] indexFile index file
 * @param[in] dataFile data file
 * @param[in] compressionMode data compression mode
 */
void ReCreateFiles(FILE* indexFile, FILE* dataFile, Dali::Platform::DataCache::CompressionMode compressionMode);

/**
 * Prepares both index and data file to have data written to.
 * Internally seeks to the end of both files.
 * @param[in] indexFile index file
 * @param[in] dataFile data file
 */
void PrepareForWritingData( FILE* dataFile, FILE* indexFile );


/**
 * Writes data to the data file.
 * @param[in] dataFile data file
 * @param[in] key the key to write
 * @param[in] data the data to write
 * @return the file offset of where the data was written to.
 */
unsigned int WriteData( FILE* dataFile,
                        Dali::Platform::DataCache::DataKey key,
                        const  Dali::Platform::DataCache::Data& data );

/**
 * Reads data from the data file.
 * The size of the data read is held in data.length.
 * @param[in] dataFile data file
 * @param[in] offset file offset
 * @param[in] key the key value, used to ensure the correct data is read
 * @param[out] data the data.length value is set to the data length.
 * @param[in] dataBuffer the buffer to read the data into
 * @param[in] bufferSize the size of the buffer in bytes
 */
bool ReadData( FILE* dataFile,
              unsigned int offset,
              Dali::Platform::DataCache::DataKey key,
              Dali::Platform::DataCache::Data& data,
              unsigned char* dataBuffer,
              unsigned int bufferSize);

/**
 * Read the | KEY | OFFSET | entries from the index file.
 * @param[in] indexFile the index file
 * @param[out] meta pointer to an array of KeyMeta structures ( KEY | OFFSET ) structs
 * @param[in] startIndex the index to start reading from
 * @param[in] count the number of indexes to read
 * @return true on success, false on failure
 */
bool ReadEntries(FILE *indexFile,KeyMeta* meta, unsigned int startIndex, unsigned int count);

/**
 * Read the number of entries from the index file
 * @param[in] indexFile the index file
 * @param[out] numEntries number of entries read
 * @return true on success, false on failure
 */
bool ReadNumberEntries( FILE* indexFile, unsigned int &numEntries );

/**
 * Write the number of entries
 * @param[in] indexFile the index file
 * @param[in] numEntires number of entries
 */
void WriteNumberEntries( FILE* indexFile, unsigned int numEntries );

/**
 * Read a key from the index file
 * @param[in] indexFile the index file
 * @param[out] key the key
 * @param[out] the offset
 * @return true on success, false on failure
 */
bool ReadKey( FILE* indexFile, Dali::Platform::DataCache::DataKey& key, unsigned int& offset );

/**
 * Write a key to the index file
 * @param[in] indexFile the index file
 * @param[in] key the key
 * @param[in] the offset
 */
void WriteKey(FILE* indexFile, Dali::Platform::DataCache::DataKey key, unsigned int offset );

/**
 * Get the header size
 * @return the size of the file header
 */
std::size_t GetHeaderSize();

} // namespace DataCacheIO

} // namespace SlpPlatform

} // namespace Dali

#endif // __DALI_SLP_PLATFORM_DATA_CACHE_IO_H__
