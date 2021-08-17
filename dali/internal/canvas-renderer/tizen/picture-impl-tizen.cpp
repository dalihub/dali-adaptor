/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/internal/canvas-renderer/tizen/picture-impl-tizen.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace // unnamed namespace
{
// Type Registration
Dali::BaseHandle Create()
{
  return Dali::BaseHandle();
}

Dali::TypeRegistration type(typeid(Dali::CanvasRenderer::Picture), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

PictureTizen* PictureTizen::New()
{
  return new PictureTizen();
}

PictureTizen::PictureTizen()
#ifdef THORVG_SUPPORT
: mTvgPicture(nullptr)
#endif
{
  Initialize();
}

PictureTizen::~PictureTizen()
{
}

void PictureTizen::Initialize()
{
#ifdef THORVG_SUPPORT
  mTvgPicture = tvg::Picture::gen().release();
  if(!mTvgPicture)
  {
    DALI_LOG_ERROR("Picture is null [%p]\n", this);
  }

  Drawable::Create();
  Drawable::SetObject(static_cast<void*>(mTvgPicture));
  Drawable::SetType(Drawable::Types::PICTURE);
#endif
}

bool PictureTizen::Load(const std::string& url)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgPicture)
  {
    DALI_LOG_ERROR("Picture is null [%p]\n", this);
    return false;
  }
  if(url.empty())
  {
    DALI_LOG_ERROR("Url is empty [%p]\n", this);
    return false;
  }

  if(mTvgPicture->load(url.c_str()) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("Load() fail. (%s)\n", url.c_str());
    return false;
  }

  Drawable::SetChanged(true);
  return true;
#else
  return false;
#endif
}

bool PictureTizen::SetSize(Vector2 size)
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgPicture)
  {
    DALI_LOG_ERROR("Picture is null [%p]\n", this);
    return false;
  }
  if(mTvgPicture->size(size.width, size.height) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("SetSize() fail.\n");
    return false;
  }
  Drawable::SetChanged(true);

  return true;
#else
  return false;
#endif
}

Vector2 PictureTizen::GetSize() const
{
#ifdef THORVG_SUPPORT
  if(!Drawable::GetObject() || !mTvgPicture)
  {
    DALI_LOG_ERROR("Picture is null [%p]\n", this);
    return Vector2::ZERO;
  }

  auto width  = 0.0f;
  auto height = 0.0f;

  if(mTvgPicture->size(&width, &height) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("GetSize() fail.\n");
    return Vector2::ZERO;
  }
  return Vector2(width, height);
#else
  return Vector2::ZERO;
#endif
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
