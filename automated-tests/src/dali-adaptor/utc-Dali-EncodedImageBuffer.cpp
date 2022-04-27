/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/public-api/adaptor-framework/encoded-image-buffer.h>
#include <stdlib.h>
#include <unistd.h>

using namespace Dali;

namespace
{
EncodedImageBuffer::RawBufferType tinybuffer()
{
  EncodedImageBuffer::RawBufferType buffer;
  buffer.PushBack(0x11);
  buffer.PushBack(0x22);
  buffer.PushBack(0x33);
  return buffer;
}

} // anonymous namespace

void dali_encoded_image_buffer_startup(void)
{
  test_return_value = TET_UNDEF;
}

void dali_encoded_image_buffer_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliEncodedImageBufferNew(void)
{
  // invoke default handle constructor
  EncodedImageBuffer buffer;

  DALI_TEST_CHECK(!buffer);

  // initialise handle
  buffer = EncodedImageBuffer::New(tinybuffer());

  DALI_TEST_CHECK(buffer);
  END_TEST;
}

int UtcDaliEncodedImageBufferCopyConstructor(void)
{
  EncodedImageBuffer buffer = EncodedImageBuffer::New(tinybuffer());
  EncodedImageBuffer bufferCopy(buffer);

  DALI_TEST_EQUALS((bool)bufferCopy, true, TEST_LOCATION);
  END_TEST;
}

int UtcDaliEncodedImageBufferAssignmentOperator(void)
{
  EncodedImageBuffer buffer = EncodedImageBuffer::New(tinybuffer());

  EncodedImageBuffer buffer2;
  DALI_TEST_EQUALS((bool)buffer2, false, TEST_LOCATION);

  buffer2 = buffer;
  DALI_TEST_EQUALS((bool)buffer2, true, TEST_LOCATION);

  END_TEST;
}

int UtcDaliEncodedImageBufferGetRawBuffer(void)
{
  EncodedImageBuffer::RawBufferType originBuffer = tinybuffer();

  EncodedImageBuffer buffer = EncodedImageBuffer::New(originBuffer);

  EncodedImageBuffer::RawBufferType getBuffer = buffer.GetRawBuffer();

  // compare value between originBuffer and getBuffer
  DALI_TEST_EQUALS(originBuffer.Count(), getBuffer.Count(), TEST_LOCATION);

  EncodedImageBuffer::RawBufferType::Iterator iter = originBuffer.Begin();
  EncodedImageBuffer::RawBufferType::Iterator jter = getBuffer.Begin();
  for(; iter != originBuffer.End(); ++iter, ++jter)
  {
    DALI_TEST_EQUALS(*iter, *jter, TEST_LOCATION);
  }

  END_TEST;
}

int UtcDaliEncodedImageBufferGetHash(void)
{
  EncodedImageBuffer buffer1 = EncodedImageBuffer::New(tinybuffer());
  EncodedImageBuffer buffer2 = EncodedImageBuffer::New(tinybuffer());
  EncodedImageBuffer buffer3 = EncodedImageBuffer::New(EncodedImageBuffer::RawBufferType()); //< EmptyBuffer

  tet_infoline("Test different encoded buffer with same data has same hash value.");
  DALI_TEST_CHECK(buffer1 != buffer2);
  DALI_TEST_CHECK(buffer1.GetHash() == buffer2.GetHash());

  tet_infoline("Test hash with empty buffer.");
  DALI_TEST_CHECK(buffer1.GetHash() != buffer3.GetHash());
  DALI_TEST_CHECK(buffer2.GetHash() != buffer3.GetHash());

  END_TEST;
}