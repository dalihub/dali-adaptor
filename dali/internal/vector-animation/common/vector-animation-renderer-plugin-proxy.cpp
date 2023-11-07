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

// CLASS HEADER
#include <dali/internal/vector-animation/common/vector-animation-renderer-plugin-proxy.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dlfcn.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
// The default plugin name
const char* DEFAULT_OBJECT_NAME("libdali2-vector-animation-renderer-plugin.so");

} // namespace

VectorAnimationRendererPluginProxy::VectorAnimationRendererPluginProxy(const std::string& sharedObjectName)
: mSharedObjectName(),
  mLibHandle(NULL),
  mPlugin(NULL),
  mCreateVectorAnimationRendererPtr(NULL),
  mDefaultSignal()
{
  if(!sharedObjectName.empty())
  {
    mSharedObjectName = sharedObjectName;
  }
  else
  {
    mSharedObjectName = DEFAULT_OBJECT_NAME;
  }

  Initialize();
}

VectorAnimationRendererPluginProxy::~VectorAnimationRendererPluginProxy()
{
  if(mPlugin)
  {
    delete mPlugin;
    mPlugin = NULL;

    if(mLibHandle && dlclose(mLibHandle))
    {
      DALI_LOG_ERROR("Error closing vector animation renderer plugin library: %s\n", dlerror());
    }
  }
}

void VectorAnimationRendererPluginProxy::Initialize()
{
  mLibHandle = dlopen(mSharedObjectName.c_str(), RTLD_LAZY);

  char* error = dlerror();
  if(mLibHandle == NULL || error != NULL)
  {
    DALI_LOG_ERROR("VectorAnimationRendererPluginProxy::Initialize: dlopen error [%s]\n", error);
    return;
  }

  // load plugin
  mCreateVectorAnimationRendererPtr = reinterpret_cast<CreateVectorAnimationRendererFunction>(dlsym(mLibHandle, "CreateVectorAnimationRendererPlugin"));

  error = dlerror();
  if(mCreateVectorAnimationRendererPtr == NULL || error != NULL)
  {
    DALI_LOG_ERROR("VectorAnimationRendererPluginProxy::Initialize: Cannot load symbol: %s\n", error);
    return;
  }

  mPlugin = mCreateVectorAnimationRendererPtr();
  if(!mPlugin)
  {
    DALI_LOG_ERROR("VectorAnimationRendererPluginProxy::Initialize: Plugin creation failed\n");
    return;
  }
}

void VectorAnimationRendererPluginProxy::Finalize()
{
  if(mPlugin)
  {
    mPlugin->Finalize();
  }
}

bool VectorAnimationRendererPluginProxy::Load(const std::string& url)
{
  if(mPlugin)
  {
    return mPlugin->Load(url);
  }
  return false;
}

void VectorAnimationRendererPluginProxy::SetRenderer(Dali::Renderer renderer)
{
  if(mPlugin)
  {
    mPlugin->SetRenderer(renderer);
  }
}

void VectorAnimationRendererPluginProxy::SetSize(uint32_t width, uint32_t height)
{
  if(mPlugin)
  {
    mPlugin->SetSize(width, height);
  }
}

bool VectorAnimationRendererPluginProxy::Render(uint32_t frameNumber)
{
  if(mPlugin)
  {
    return mPlugin->Render(frameNumber);
  }
  return false;
}

void VectorAnimationRendererPluginProxy::RenderStopped()
{
  if(mPlugin)
  {
    mPlugin->RenderStopped();
  }
}

uint32_t VectorAnimationRendererPluginProxy::GetTotalFrameNumber() const
{
  if(mPlugin)
  {
    return mPlugin->GetTotalFrameNumber();
  }
  return 0;
}

float VectorAnimationRendererPluginProxy::GetFrameRate() const
{
  if(mPlugin)
  {
    return mPlugin->GetFrameRate();
  }
  return 0.0f;
}

void VectorAnimationRendererPluginProxy::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  if(mPlugin)
  {
    mPlugin->GetDefaultSize(width, height);
  }
  else
  {
    width  = 0;
    height = 0;
  }
}

void VectorAnimationRendererPluginProxy::GetLayerInfo(Property::Map& map) const
{
  if(mPlugin)
  {
    mPlugin->GetLayerInfo(map);
  }
}

bool VectorAnimationRendererPluginProxy::GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const
{
  if(mPlugin)
  {
    return mPlugin->GetMarkerInfo(marker, startFrame, endFrame);
  }
  return false;
}

void VectorAnimationRendererPluginProxy::GetMarkerInfo(Property::Map& map) const
{
  if(mPlugin)
  {
    mPlugin->GetMarkerInfo(map);
  }
}

void VectorAnimationRendererPluginProxy::InvalidateBuffer()
{
  if(mPlugin)
  {
    mPlugin->InvalidateBuffer();
  }
}

void VectorAnimationRendererPluginProxy::AddPropertyValueCallback(const std::string& keyPath, Dali::VectorAnimationRenderer::VectorProperty property, CallbackBase* callback, int32_t id)
{
  if(mPlugin)
  {
    mPlugin->AddPropertyValueCallback(keyPath, property, callback, id);
  }
}

void VectorAnimationRendererPluginProxy::KeepRasterizedBuffer()
{
  if(mPlugin)
  {
    mPlugin->KeepRasterizedBuffer();
  }
}

VectorAnimationRendererPlugin::UploadCompletedSignalType& VectorAnimationRendererPluginProxy::UploadCompletedSignal()
{
  if(mPlugin)
  {
    return mPlugin->UploadCompletedSignal();
  }
  return mDefaultSignal;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
