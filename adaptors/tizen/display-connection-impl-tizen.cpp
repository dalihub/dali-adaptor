/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <display-connection-impl.h>

// EXTERNAL_HEADERS
#include <tbm_bufmgr.h>
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <native-render-surface.h>
#include <gl/egl-implementation.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

EGLNativeDisplayType DisplayConnection::GetNativeDisplay()
{
  return reinterpret_cast< EGLNativeDisplayType >( tbm_bufmgr_init( -1 ) );
}

void DisplayConnection::ReleaseNativeDisplay()
{
  if( mDisplay )
  {
    tbm_bufmgr_deinit( reinterpret_cast< tbm_bufmgr >( mDisplay ) );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
