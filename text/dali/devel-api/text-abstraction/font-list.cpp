/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/text-abstraction/font-list.h>

namespace Dali
{

namespace TextAbstraction
{

DALI_IMPORT_API std::ostream& operator<<( std::ostream& o, const FontList& fontList )
{
  for( unsigned int i=0; i<fontList.size(); ++i )
  {
    o << "Font " << i << ") path: " << fontList[i].path << " family: " << fontList[i].family << " style: " << fontList[i].style << std::endl;
  }

  return o;
}

} // namespace TextAbstraction

} // namespace Dali
