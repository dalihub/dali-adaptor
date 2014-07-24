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

#include <libgen.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include "binary-model-builder.h"

namespace
{

/**
 * Calculate a checksum for a block of data
 * @param[in] buffer A pointer to a data block
 * @param[in] length The number of items in the data block
 * @return A checksum for the data block
 */
unsigned short CalculateChecksum(unsigned short* buffer, unsigned int length)
{
  unsigned short checkSum = 0;
  for( unsigned int i = 0; i < length; ++i)
  {
    checkSum += *buffer++;
  }

  return (~checkSum) + 1;
}

} // namespace

namespace Dali
{

namespace SlpPlatform
{

BinaryModelBuilder::BinaryModelBuilder(const std::string& name)
: mFilename(name)
{
}

BinaryModelBuilder::~BinaryModelBuilder()
{
}

bool BinaryModelBuilder::Build(ModelData model)
{
  std::filebuf buf;
  buf.open(mFilename.c_str(), std::ios::in | std::ios::binary);

  {
    std::istream stream(&buf);

    // determine data length
    stream.seekg(0, std::ios_base::end);
    unsigned int length = static_cast<unsigned int>(stream.tellg());
    stream.seekg(0, std::ios_base::beg);

    // allocate a buffer
    unsigned short* buffer = new unsigned short[length / 2];
    // read data into buffer
    stream.read(reinterpret_cast<char*>(buffer), length);
    // calculate a checksum
    unsigned short checkSum = CalculateChecksum(buffer, length / 2);
    // finished with buffer, delete it
    delete [] buffer;

    if( checkSum != 0 )
    {
      return false;
    }

    // return read pointer to beginning of the file
    stream.seekg(0, std::ios_base::beg);
  }

  return model.Read(buf);
}

const std::string& BinaryModelBuilder::GetModelName()
{
  char* modelName = strdup(mFilename.c_str());

  mModelName.assign(basename(modelName));

  free(modelName);

  return mModelName;
}

bool BinaryModelBuilder::Write(ModelData model)
{
  bool status = false;

  // open file
  std::filebuf buf;
  buf.open(mFilename.c_str(), std::ios::out | std::ios::binary);

  // write model to file
  status = model.Write(buf);
  buf.close();

  // if model write was successfull, calculate
  // a checksum and append to the file
  if( status )
  {
    // reopen file for read/write
    buf.open(mFilename.c_str(), std::ios::in | std::ios::out | std::ios::binary);
    std::iostream stream(&buf);

    // determine data length
    stream.seekg(0, std::ios_base::end);
    unsigned int length = static_cast<unsigned int>(stream.tellg());
    stream.seekg(0, std::ios_base::beg);

    // allocate a buffer
    unsigned short* buffer = new unsigned short[length / 2];
    // read data into buffer
    stream.read(reinterpret_cast<char*>(buffer), length);
    // calculate a checksum
    unsigned short checkSum = CalculateChecksum(buffer, length / 2);
    // finished with buffer, delete it
    delete [] buffer;

    // Append checksum to file
    stream.seekp(0, std::ios_base::end);
    stream.write(reinterpret_cast<char*>(&checkSum), sizeof(unsigned short));
  }

  return status;
}


} // namespace SlpPlatform

} // namespace Dali

