#ifndef __DALI_TIZEN_PLATFORM_DATA_CACHE_H__
#define __DALI_TIZEN_PLATFORM_DATA_CACHE_H__

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
#include <dali/public-api/common/map-wrapper.h>

namespace Dali
{

namespace TizenPlatform
{

/**
 * Class used for storing key, value data to the file system.
 * Concrete implementation of the Platform::DataCache interface
 * E.g. Unicode character code (Key), distance field binary data (Value).
 *
 *
 * Internals: Uses two files.
 * - Index file, which has Key values, and offsets to the data in the data file.
 *   __________________
 *  | FILE HEADER      \
 *  |-------------------|
 *  | Key | Data Offset |
 *  | Key | Data Offset |
 *  | Key | Data Offset |
 *
 * - Data file, contains the data
 *  __________________________
 * | FILE HEADER              \
 * | -------------------------|
 * | Key | Data Length | Data |
 * | Key | Data Length | Data |
 * | Key | Data Length | Data |
 *
 * The Key is stored in the data file as well, to ensure the information held in the index file
 * is correct.
 *
 * Multi-threading / multi-process notes
 *
 * - Any thread / process can read from the data-cache files without being blocked.
 * - Only a single thread / process can write data at any time. This is achieved by
 * using a file lock.
 * - Reading from the data-cache while it is being written to is fine. See DataCache::Add()
 * code comments for an explanation on how this works.
 *
 * Summary:
 * DataCache::Find = never blocks
 * DataCache::Add = will block, if another thread / process is also running DataCache::Add()
 *
 * File corruption
 *
 * This may occur if the power is cut or or if the process is killed during
 * a DataCache::Add() operation.
 * On class creation a check is made to ensure the files are valid.
 * If corruption is found they are deleted and recreated empty.
 *
 * Performance notes:
 * DataCache::Find() use std::map which has complexity of O( log n)
 * Plus the addition of a fixed time reading/writing the data to file system.
 * In simple terms, the time taken to Add or Find data in a data cache
 * with 10,000 entries is almost identical to a data cache with 100 items.
 *
 */
class DataCache  : public Platform::DataCache
{

public:

  /**
   * Constructor
   * @copydoc Dali::Platform::DataCache::DataCache()
   */
  DataCache( Dali::Platform::DataCache::ReadWriteMode mode,
               Dali::Platform::DataCache::CompressionMode compressionMode,
               const std::string& fileName,
               unsigned int maxDataSize,
               unsigned int maxNumberEntries);

  /**
   * Destructor
   */
  virtual ~DataCache();

  /**
   * @copydoc Dali::Platform::DataCache::Find()
   */
  virtual void Find( const KeyVector& keyVector, DataVector& valueArray );

  /**
   * @copydoc Dali::Platform::DataCache::Add()
   */
  virtual void Add(const KeyVector& keyVector, const DataVector& valueArray);

private:

  /**
   * Check and re-create the data/index files if they are corrupt.
   */
  void CorruptedFileCheck() const;

  /**
   * Initialise the lookup.
   * @param[out] indexFileLoaded set to true of the index file was loaded
   */
  void InitialiseLookup( bool &indexFileLoaded );

  /**
   * Load the index file if it exists
   */
  void LoadIndexFile();

  /**
   * Read data for each key.
   * @param[in] dataFile data file
   * @param[in] indexFileLoaded whether the index file has been loaded
   * @param[in] keyVector vector of keys
   * @param[out] dataVector vector of data objects
   */
  bool ReadFromDataFile( FILE* dataFile,
                         bool indexFileLoaded,
                         const KeyVector& keyVector,
                         DataVector& dataVector
                         );

  /**
   * Checks which keys / data exist in the data cache.
   * Done just before writing to the data cache to avoid duplicate
   * entries.
   * @param[in] keyVector vector of keys
   * @param[out] dataVector vector of data objects
   */
  void FindExistingData( const KeyVector& keyVector, DataVector& dataVector) const;

  /**
   * Open the index and data files for writing
   * Asserts on failure.
   * @param[out] dataFile data file
   * @param[out] indexFile index file
   */
  void OpenIndexAndDataForWriting(FILE** dataFile, FILE** indexFile);

  /**
   * Writes the data to the data file and updates the index file.
   * @param[in] keyVector vector of keys
   * @param[in] dataVector vector of data objects
   * @param[in] dataFile data file
   * @param[in] indexFile index file
   */
  void WriteDataAndUpdateIndex( const KeyVector& keyVector,
                                const DataVector& dataVector,
                                FILE* dataFile,
                                FILE* indexFile );
  /**
   * Read the data from the data file.
   * @param[in] dataFile data file
   * @param[in[ offset the file offset where the data exists
   * @param[in] key used to ensure the data is for the correct key
   * @param[out] data assigned the data from the file
   * @return true on success, false on failure (corruption)
   */
  bool ReadData( FILE* dataFile,
                  unsigned int offset,
                  DataKey key,
                  Data &data) const;

  /**
   * Write the data to the data file.
   * @param[in] dataFile data file
   * @param[in] key used to ensure the data is for the correct key
   * @param[in] data assigned the data from the file
   * @return the offset in the file where the data was written
   */
  unsigned int WriteData( FILE *dataFile, DataKey key, const Data &data) const;

  /**
   * Reload the index file to check for updates.
   * @param[in] indexFile index file
   * @return true on success, false on failure (corruption)
   */
  bool ReLoadIndexFile( FILE* indexFile );

  /**
   * Read new entries from the index file.
   * @param[in] indexFile index file
   * @param[in] newEntries number of new entries to read
   * @return true on success, false on failure (corruption)
   */
  bool ReadNewEntries( FILE* indexFile, unsigned int newEntries);

  /**
   * Close existing files, then calls DataCacheIo::RecreateFiles to truncate and reinitialize the files
   * @param[in] indexFile index file
   * @param[in] dataFile data file
   */
  void CloseAndReinitializeFiles(FILE** indexFile , FILE** dataFile) const;

  /**
   * Return the maximum buffer size of encoded data.
   * E.g. if maximum data size is 4 KB, and RLE compression is used
   * the maximum encoded size is 8 KB in the worst case scenario.
   * @return maximum size of encoded data.
   */
  unsigned int GetMaxEncodedDataSize() const;

  typedef std::map< DataKey, unsigned int /* file offset */ > KeyLookup;

  KeyLookup      mLookup;             ///< Lookup between key, and offset of the value in a table.
  std::string    mIndexFile;          ///< index file name
  std::string    mDataFile;           ///< cache file name
  unsigned int   mNumberEntries;      ///< how many entries in our lookup (mLookup.size() can be O(n)
  unsigned int   mMaxNumberEntries;   ///< Maximum number entries allowed
  unsigned int   mMaxDataSize;        ///< size of the data
  unsigned char* mEncodeBuffer;       ///< encode buffer for compressed data
  unsigned char* mDecodeBuffer;       ///< decode buffer for un-compressed data
  std::size_t    mEncodeBufferSize;   ///< Size of the encode buffer

  Platform::DataCache::ReadWriteMode    mMode;             ///< read / write mode.
  Platform::DataCache::CompressionMode  mCompressionMode;  ///< Compression mode

};

} // namespace TizenPlatform

} // namespace Dali

#endif // __DALI_TIZEN_PLATFORM_DATA_CACHE_H__
