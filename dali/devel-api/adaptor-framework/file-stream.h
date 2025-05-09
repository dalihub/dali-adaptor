#ifndef DALI_FILE_STREAM_H
#define DALI_FILE_STREAM_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <stdint.h>
#include <stdio.h>

#include <iostream>
#include <memory>
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
class DALI_ADAPTOR_API FileStream
{
public:
  /**
   * @brief File type formats
   * The default format is binary
   */
  enum FileMode ///< FileType format
  {
    BINARY = 1 << 0, ///< File stream will be opened as a binary
    TEXT   = 1 << 1, ///< File stream will be opened as text
    READ   = 1 << 2, ///< File stream will be opened for reading
    WRITE  = 1 << 3, ///< File stream will be opened for writing
    APPEND = 1 << 4, ///< File stream will be opened for appending
  };

  /**
   * Constructor
   * @param[in] filename Filename of the file to open the stream for
   * @param[in] mode How we want to open the stream. Binary or Text, Read or Write. Binary & Read default
   */
  FileStream(const std::string& filename, uint8_t mode = BINARY | READ);

  /**
   * Constructor
   * @param[in] buffer Buffer to open the stream for.
   *                   The buffer is not owned by FileStream and must be valid for entire lifetime of FileStream
   * @param[in] dataSize The maximum size of the data in the buffer.
   * @param[in] mode How we want to open the stream. Binary or Text, Read or Write. Binary & Read default
   */
  FileStream(uint8_t* buffer, size_t dataSize, uint8_t mode = BINARY | READ);

  /**
   * Constructor
   * @param[in] buffer Buffer to open the stream for.
   *                   The buffer is not owned by FileStream and must be valid for entire lifetime of FileStream
   * @param[in] dataSize The maximum size of the data in the buffer.
   * @param[in] mode How we want to open the stream. Binary or Text, Read or Write. Binary & Read default
   */
  FileStream(Dali::Vector<uint8_t>& buffer, size_t dataSize, uint8_t mode = BINARY | READ);

  /**
   * Default move constructor
   */
  FileStream(FileStream&&) noexcept;

  /**
   * Non copyable
   */
  FileStream(const FileStream&) = delete;

  /**
   * Non assignable
   */
  FileStream& operator=(const FileStream&) = delete;

  /**
   * Move assignable
   */
  FileStream& operator=(FileStream&&) noexcept;

  /**
   * Destructor
   */
  ~FileStream();

  /**
   * @brief Returns the stream
   * @return std::iostream.
   */
  std::iostream& GetStream();

  /**
   * @brief Returns the file stream
   * @return FILE.
   * @note This class is responsible for closing the file so the caller SHOULD NOT call fclose() on the returned pointer.
   */
  FILE* GetFile();

private:
  class Impl;
  std::unique_ptr<Impl> mImpl;
};

} // namespace Dali

#endif // DALI_FILE_STREAM_H
