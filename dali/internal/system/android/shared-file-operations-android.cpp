/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/shared-file.h>

// EXTERNAL INCLUDES
#include <fcntl.h>
#include <bits/ioctl.h>
#include <include/linux/ashmem.h>
#include <sys/mman.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

int SharedFile::Open( const char* filename, int size, int oflag, mode_t mode )
{
  int fileDescriptor = open( ASHMEM_NAME_DEF, oflag );
  if( mFileDescriptor >= 0 )
  {
    ioctl( mFileDescriptor, ASHMEM_SET_NAME, filename );
    ioctl( mFileDescriptor, ASHMEM_SET_SIZE, size );
  }

  return mFileDescriptor;
}

} // Adaptor

} // Internal

} // Dali
