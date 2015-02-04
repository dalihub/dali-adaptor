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
#include <dali/public-api/text-abstraction/font-metrics.h>

namespace Dali
{

namespace TextAbstraction
{

FontMetrics::FontMetrics()
: ascender( 0 ),
  descender( 0 ),
  height( 0 )
{
}

FontMetrics::FontMetrics( signed long ascender26Dot6,
                          signed long descender26Dot6,
                          signed long height26Dot6 )
: ascender( ascender26Dot6 ),
  descender( descender26Dot6 ),
  height( height26Dot6 )
{
}

} // namespace TextAbstraction

} // namespace Dali
