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

#ifndef DALI_ADAPTOR_EXTERN_DEFINITIONS_H
#define DALI_ADAPTOR_EXTERN_DEFINITIONS_H

#include <xlocale>

int setenv( const char* __name, const char* __value, int __replace );
const char* app_get_data_path();

static int strncasecmp(const char *s1, const char *s2, register int n)
{
  while (--n >= 0 && toupper((unsigned char)*s1) == toupper((unsigned char)*s2++))
      if (*s1++ == 0)  return 0;
  return(n < 0 ? 0 : toupper((unsigned char)*s1) - toupper((unsigned char)*--s2));
}

int __cdecl setsockopt( int s, int level, int optname, unsigned int * optval, unsigned int optlen );

int __cdecl setsockopt( int s, int level, int optname, int * optval, unsigned int optlen );


#define fmemopen CustomFile::FMemopen

namespace CustomFile
{
  FILE* FMemopen( void* __s, size_t __len, const char* __modes );
}

#endif // DALI_ADAPTOR_EXTERN_DEFINITIONS_H
