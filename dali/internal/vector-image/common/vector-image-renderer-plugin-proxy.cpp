/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/internal/vector-image/common/vector-image-renderer-plugin-proxy.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

// The default plugin name
const char* DEFAULT_OBJECT_NAME( "libdali2-vector-image-renderer-plugin.so" );

}

VectorImageRendererPluginProxy::VectorImageRendererPluginProxy(std::string sharedObjectName)
: mSharedObjectName(std::move(sharedObjectName)),
  mLibHandle(nullptr),
  mPlugin(nullptr),
  mCreateVectorImageRendererPtr(nullptr)
{
  if(mSharedObjectName.empty())
  {
    mSharedObjectName = DEFAULT_OBJECT_NAME;
  }

  InitializePlugin();
}

VectorImageRendererPluginProxy::~VectorImageRendererPluginProxy()
{
  if(mPlugin)
  {
    delete mPlugin;
    mPlugin = nullptr;

    if(mLibHandle && dlclose(mLibHandle))
    {
      DALI_LOG_ERROR("Error closing vector image renderer plugin library: %s\n", dlerror());
    }
  }
}

void VectorImageRendererPluginProxy::InitializePlugin()
{
  mLibHandle = dlopen(mSharedObjectName.c_str(), RTLD_LAZY);

  char* error = dlerror();
  if(mLibHandle == nullptr || error != nullptr)
  {
    DALI_LOG_WARNING("VectorImageRendererPluginProxy::Initialize: dlopen error [%s]\n", error);
    return;
  }

  // load plugin
  mCreateVectorImageRendererPtr = reinterpret_cast< CreateVectorImageRendererFunction >( dlsym( mLibHandle, "CreateVectorImageRendererPlugin" ) );

  error = dlerror();
  if( mCreateVectorImageRendererPtr == nullptr || error != nullptr )
  {
    DALI_LOG_ERROR( "VectorImageRendererPluginProxy::Initialize: Cannot load symbol: %s\n", error );
    return;
  }

  mPlugin = mCreateVectorImageRendererPtr();
  if( !mPlugin )
  {
    DALI_LOG_ERROR("VectorImageRendererPluginProxy::Initialize: Plugin creation failed\n");
    return;
  }
}

bool VectorImageRendererPluginProxy::IsValid() const
{
  return (mPlugin != nullptr);
}

bool VectorImageRendererPluginProxy::Load(const Vector<uint8_t>& data)
{
  if(mPlugin)
  {
    return mPlugin->Load(data);
  }
  return false;
}

bool VectorImageRendererPluginProxy::Rasterize(Dali::Devel::PixelBuffer& buffer, float scale)
{
  if( mPlugin )
  {
    return mPlugin->Rasterize(buffer, scale);
  }
  return false;
}

void VectorImageRendererPluginProxy::GetDefaultSize( uint32_t& width, uint32_t& height ) const
{
  if( mPlugin )
  {
    mPlugin->GetDefaultSize( width, height );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
