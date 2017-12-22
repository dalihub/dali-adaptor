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
#include "widget-controller.h"

// EXTERNAL INCLUDES
#include <bundle.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

Widget::Impl::Impl( widget_base_instance_h instanceHandle )
: mInstanceHandle( instanceHandle )
{
}

Widget::Impl::~Impl()
{
}

void Widget::Impl::SetContentInfo( const std::string& contentInfo )
{
  bundle *contentBundle;
  bundle_raw *contentBundleRaw = reinterpret_cast< bundle_raw* >( const_cast<char*>(contentInfo.c_str()) );
  int len = contentInfo.length();
  contentBundle = bundle_decode(contentBundleRaw, len);

  widget_base_context_set_content_info( mInstanceHandle, contentBundle );

  bundle_free( contentBundle );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
