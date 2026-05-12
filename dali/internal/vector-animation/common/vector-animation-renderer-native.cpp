/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/vector-animation/common/vector-animation-renderer-event-manager.h>
#include <dali/internal/vector-animation/common/vector-animation-renderer-native.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/property-array.h>
#include <dali/public-api/rendering/texture-set.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>

namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gVectorAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VECTOR_ANIMATION");
#endif

/**
 * @brief Applies aspect-fit scaling and translation to the given ThorVG picture.
 *
 * Calculates the scale that fits the animation within the target area while
 * preserving aspect ratio, then sets the picture size and translate accordingly.
 *
 * @param[in] picture       The ThorVG picture to scale and position
 * @param[in] defaultWidth  Default width from the source file
 * @param[in] defaultHeight Default height from the source file
 * @param[in] targetWidth   Target rendering width
 * @param[in] targetHeight  Target rendering height
 */
void ApplyAspectFitSize(tvg::Picture* picture, uint32_t defaultWidth, uint32_t defaultHeight, uint32_t targetWidth, uint32_t targetHeight)
{
  if(!picture || defaultWidth == 0 || defaultHeight == 0)
  {
    return;
  }

  if(targetWidth > 0 && targetHeight > 0)
  {
    const float targetW   = static_cast<float>(targetWidth);
    const float targetH   = static_cast<float>(targetHeight);
    const float defaultW  = static_cast<float>(defaultWidth);
    const float defaultH  = static_cast<float>(defaultHeight);

    const float scale = std::min(targetW / defaultW, targetH / defaultH);

    const float newW = defaultW * scale;
    const float newH = defaultH * scale;

    const float tx = (targetW - newW) * 0.5f;
    const float ty = (targetH - newH) * 0.5f;

    picture->size(newW, newH);
    picture->translate(tx, ty);
  }
}

} // unnamed namespace

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

VectorAnimationRendererNative::VectorAnimationRendererNative()
: mCanvas(),
  mAnimation(),
  mUploadCompletedSignal(),
  mDecodedBuffers(),
  mPreviousRenderingData(),
  mPreparedRenderingData(),
  mCurrentRenderingData(),
  mUrl(),
  mJsonData(),
  mCachedLayerInfo(),
  mCachedMarkerInfo(),
  mMutex(),
  mRenderingDataMutex(),
  mRenderer(),
  mTotalFrame(0),
  mDefaultWidth(0),
  mDefaultHeight(0),
  mTargetWidth(0),
  mTargetHeight(0),
  mFrameRate(0.0f),
  mDuration(0.0f),
  mPropertyCallbacks(),
  mResourceReady(false),
  mResourceReadyTriggered(false),
  mPendingSizeUpdate(false),
  mFinalized(false),
  mLoadFailed(false),
  mEnableFixedCache(false),
  mEnableAspectFit(true),
  mMetadataParsed(false)
{
}

VectorAnimationRendererNative::~VectorAnimationRendererNative()
{
  // Ensure the rendering data removed after Render finisehd at VectorAnimationTaskThread.
  mCurrentRenderingData.reset();
}

void VectorAnimationRendererNative::Initialize()
{
  // Register to the VectorAnimationRendererEventManager for event handling.
  // This also ensures VectorAnimationRendererEventManager singleton is initialized.
  VectorAnimationRendererEventManager::Get().AddEventHandler(*this);
}

void VectorAnimationRendererNative::Finalize()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mFinalized)
  {
    return;
  }

  mRenderer.Reset();

  {
    Dali::Mutex::ScopedLock dataLock(mRenderingDataMutex);
    mPreparedRenderingData.reset();
    mPreviousRenderingData.clear();
    mFinalized = true;
  }

  OnFinalize();

  // Unregister from manager
  VectorAnimationRendererEventManager::Get().RemoveEventHandler(*this);

  // Clear property callbacks before animation to prevent dangling raw pointer
  // passed to thorvg overrideProperty()
  mPropertyCallbacks.clear();

  if(mCanvas)
  {
    mCanvas->sync();
    mCanvas->remove();
  }
  mCanvas.reset();
  mAnimation.reset();

  mDecodedBuffers.clear();

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "[%p]\n", this);
}

bool VectorAnimationRendererNative::Load(const std::string& url)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(DALI_UNLIKELY(mFinalized))
  {
    return false;
  }

  mUrl            = url;
  mMetadataParsed = false;

  if(!mAnimation)
  {
    mAnimation = std::unique_ptr<tvg::Animation>(tvg::LottieAnimation::gen());
  }

  if(!mAnimation)
  {
    DALI_LOG_ERROR("VectorAnimationRendererNative::Load: Failed to create Animation [%p]\n", this);
    return false;
  }

  auto picture = mAnimation->picture();
  if(!picture)
  {
    DALI_LOG_ERROR("VectorAnimationRendererNative::Load: picture() returned null [%p]\n", this);
    return false;
  }

  auto loadResult = picture->load(url.c_str());
  if(loadResult != tvg::Result::Success)
  {
    DALI_LOG_ERROR("VectorAnimationRendererNative::Load: Failed to load %s (result=%d) [%p]\n",
                   url.c_str(), static_cast<int>(loadResult), this);
    mLoadFailed = true;
    return false;
  }

  float w = 0.0f, h = 0.0f;
  picture->size(&w, &h);
  mDefaultWidth  = static_cast<uint32_t>(w);
  mDefaultHeight = static_cast<uint32_t>(h);

  mTotalFrame = static_cast<uint32_t>(mAnimation->totalFrame());
  mDuration   = mAnimation->duration();
  mFrameRate  = (mDuration > 0.0f) ? (static_cast<float>(mTotalFrame) / mDuration) : 60.0f;

  ApplyAspectFitSize(picture, mDefaultWidth, mDefaultHeight, mTargetWidth, mTargetHeight);

  if(!mCanvas)
  {
    mCanvas = std::unique_ptr<tvg::SwCanvas>(tvg::SwCanvas::gen());
  }

  if(mCanvas)
  {
    // Clear previous picture before adding new one to avoid stacking
    mCanvas->remove();

    auto pic = mAnimation->picture();
    pic->ref();
    mCanvas->add(pic);
  }

  mResourceReady          = true;
  mResourceReadyTriggered = false;

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "file [%s] [%p]\n", url.c_str(), this);

  return true;
}

bool VectorAnimationRendererNative::Load(const Dali::Vector<uint8_t>& data)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(DALI_UNLIKELY(mFinalized))
  {
    return false;
  }

  mJsonData.assign(reinterpret_cast<const char*>(data.Begin()), data.Count());
  mMetadataParsed = false;

  if(!mAnimation)
  {
    mAnimation = std::unique_ptr<tvg::Animation>(tvg::LottieAnimation::gen());
  }

  if(!mAnimation)
  {
    DALI_LOG_ERROR("VectorAnimationRendererNative::Load(data): Failed to create Animation [%p]\n", this);
    return false;
  }

  auto picture = mAnimation->picture();
  if(!picture)
  {
    DALI_LOG_ERROR("VectorAnimationRendererNative::Load(data): picture() returned null [%p]\n", this);
    return false;
  }

  auto loadResult = picture->load(reinterpret_cast<const char*>(data.Begin()), data.Count(), "lottie", nullptr, true);
  if(loadResult != tvg::Result::Success)
  {
    DALI_LOG_ERROR("VectorAnimationRendererNative::Load(data): Failed to load (result=%d) [%p]\n",
                   static_cast<int>(loadResult), this);
    mLoadFailed = true;
    return false;
  }

  float w = 0.0f, h = 0.0f;
  picture->size(&w, &h);
  mDefaultWidth  = static_cast<uint32_t>(w);
  mDefaultHeight = static_cast<uint32_t>(h);

  mTotalFrame = static_cast<uint32_t>(mAnimation->totalFrame());
  mDuration   = mAnimation->duration();
  mFrameRate  = (mDuration > 0.0f) ? (static_cast<float>(mTotalFrame) / mDuration) : 60.0f;

  ApplyAspectFitSize(picture, mDefaultWidth, mDefaultHeight, mTargetWidth, mTargetHeight);

  if(!mCanvas)
  {
    mCanvas = std::unique_ptr<tvg::SwCanvas>(tvg::SwCanvas::gen());
  }

  if(mCanvas)
  {
    mCanvas->remove();

    auto pic = mAnimation->picture();
    pic->ref();
    mCanvas->add(pic);
  }

  mResourceReady          = true;
  mResourceReadyTriggered = false;

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "data [data size : %zu byte] [%p]\n", data.Size(), this);

  return true;
}

void VectorAnimationRendererNative::SetRenderer(Dali::Renderer renderer)
{
  bool emitSignal = false;
  {
    Dali::Mutex::ScopedLock lock(mMutex);
    mRenderer = renderer;

    if(IsRenderReady())
    {
      Dali::TextureSet textureSet = renderer.GetTextures();
      if(textureSet && GetTargetTexture())
      {
        textureSet.SetTexture(0u, GetTargetTexture());
      }
      emitSignal = true;
    }
  }

  if(emitSignal)
  {
    mUploadCompletedSignal.Emit();
  }
}

void VectorAnimationRendererNative::SetSize(uint32_t width, uint32_t height)
{
  if(!Dali::Stage::IsCoreThread())
  {
    DALI_LOG_ERROR("SetSize should be called by Core Thread.\n");
    return;
  }

  if(mLoadFailed)
  {
    DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Load is failed. Do not make texture [%p]\n", this);
    return;
  }

  if(width == 0 || height == 0)
  {
    return;
  }

  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_UNLIKELY(mFinalized))
    {
      DALI_LOG_DEBUG_INFO("SetSize should be called after finalized.\n");
      return;
    }

    if(!mPreparedRenderingData && mCurrentRenderingData && (mCurrentRenderingData->mWidth == width && mCurrentRenderingData->mHeight == height))
    {
      return;
    }

    if(mPreparedRenderingData && (mPreparedRenderingData->mWidth == width && mPreparedRenderingData->mHeight == height))
    {
      return;
    }
    mPreparedRenderingData.reset();
  }

  std::shared_ptr<RenderingData> preparedRenderingData = CreateRenderingData();
  preparedRenderingData->mWidth                        = width;
  preparedRenderingData->mHeight                       = height;

  PrepareTarget(preparedRenderingData);

  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      mPreparedRenderingData = preparedRenderingData;
    }
  }

  DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "width = %d, height = %d [%p]\n", preparedRenderingData->mWidth, preparedRenderingData->mHeight, this);

  if(mTargetWidth == width && mTargetHeight == height)
  {
    return;
  }

  mTargetWidth       = width;
  mTargetHeight      = height;
  mPendingSizeUpdate = true;
}

void VectorAnimationRendererNative::UpdateSizeIfNeeded()
{
  if(mPendingSizeUpdate)
  {
    if(mAnimation && mAnimation->picture())
    {
      if(mEnableAspectFit)
      {
        ApplyAspectFitSize(mAnimation->picture(), mDefaultWidth, mDefaultHeight, mTargetWidth, mTargetHeight);
      }
      else
      {
        // When aspect fit is disabled, stretch to fill the entire target size
        mAnimation->picture()->size(static_cast<float>(mTargetWidth), static_cast<float>(mTargetHeight));
        mAnimation->picture()->translate(0.0f, 0.0f);
      }
    }
    mPendingSizeUpdate = false;
  }
}

void VectorAnimationRendererNative::RenderStopped()
{
  FreeReleasedBuffers();
}

void VectorAnimationRendererNative::FreeReleasedBuffers()
{
}

uint32_t VectorAnimationRendererNative::GetTotalFrameNumber() const
{
  return mTotalFrame;
}

float VectorAnimationRendererNative::GetFrameRate() const
{
  return mFrameRate;
}

void VectorAnimationRendererNative::GetDefaultSize(uint32_t& width, uint32_t& height) const
{
  width  = mDefaultWidth;
  height = mDefaultHeight;
}

void VectorAnimationRendererNative::GetLayerInfo(Property::Map& map) const
{
  Dali::Mutex::ScopedLock lock(mMutex);
  ParseLottieMetadata();
  map = mCachedLayerInfo;
}

bool VectorAnimationRendererNative::GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const
{
  Dali::Mutex::ScopedLock lock(mMutex);
  ParseLottieMetadata();

  const Property::Value* value = mCachedMarkerInfo.Find(Dali::StringView(marker.c_str(), marker.size()));
  if(value)
  {
    const Property::Array* arr = value->GetArray();
    if(arr && arr->Count() >= 2)
    {
      startFrame = static_cast<uint32_t>(arr->GetElementAt(0).Get<int>());
      endFrame   = static_cast<uint32_t>(arr->GetElementAt(1).Get<int>());
      return true;
    }
  }

  return false;
}

void VectorAnimationRendererNative::GetMarkerInfo(Property::Map& map) const
{
  Dali::Mutex::ScopedLock lock(mMutex);
  ParseLottieMetadata();
  map = mCachedMarkerInfo;
}

void VectorAnimationRendererNative::InvalidateBuffer()
{
  Dali::Mutex::ScopedLock lock(mMutex);
  mResourceReady = false;
}

void VectorAnimationRendererNative::AddPropertyValueCallback(const std::string& keyPath, VectorProperty property, CallbackBase* callback, int32_t id)
{
  Dali::Mutex::ScopedLock lock(mMutex);
  if(!mAnimation || !callback)
  {
    delete callback;
    return;
  }

  static auto wrapper = [](float frameNo, void* value, void* data) -> bool {
      auto cbObj = static_cast<PropertyCallback*>(data);
      if (!cbObj || !cbObj->callback) return false;

      Property::Value val = CallbackBase::ExecuteReturn<Property::Value>(*(cbObj->callback), cbObj->id, cbObj->property, static_cast<uint32_t>(frameNo));

      switch (cbObj->property) {
          case VectorProperty::FILL_COLOR:
          case VectorProperty::STROKE_COLOR: {
              struct TVGRGB32 { int32_t r, g, b; };
              auto color = static_cast<TVGRGB32*>(value);
              Vector3 vec(1.0f, 1.0f, 1.0f);
              if (val.Get(vec)) {
                  color->r = std::min(255, std::max(0, (int)(vec.r * 255.0f)));
                  color->g = std::min(255, std::max(0, (int)(vec.g * 255.0f)));
                  color->b = std::min(255, std::max(0, (int)(vec.b * 255.0f)));
                  return true;
              }
              break;
          }
          case VectorProperty::TRANSFORM_OPACITY:
          case VectorProperty::FILL_OPACITY:
          case VectorProperty::STROKE_OPACITY: {
              auto opacity = static_cast<uint8_t*>(value);
              float o = 100.0f;
              if (val.Get(o)) {
                  *opacity = std::min(255, std::max(0, (int)(o / 100.0f * 255.0f)));
                  return true;
              }
              break;
          }
          case VectorProperty::STROKE_WIDTH:
          case VectorProperty::TRANSFORM_ROTATION: {
              auto scalar = static_cast<float*>(value);
              float s = 0.0f;
              if (val.Get(s)) {
                  *scalar = s;
                  return true;
              }
              break;
          }
          case VectorProperty::TRIM_START: {
              auto scalar = static_cast<float*>(value);
              float s = 0.0f;
              if (val.Get(s)) {
                  *scalar = s;
                  return true;
              }
              break;
          }
          case VectorProperty::TRIM_END: {
              auto range = static_cast<float*>(value);
              Vector2 vec2;
              if (val.Get(vec2)) {
                  range[0] = vec2.x;
                  range[1] = vec2.y;
                  return true;
              }
              float s = 0.0f;
              if (val.Get(s)) {
                  range[0] = 0.0f;
                  range[1] = s;
                  return true;
              }
              break;
          }
          case VectorProperty::TRANSFORM_POSITION:
          case VectorProperty::TRANSFORM_SCALE: {
              struct TVGPoint { float x, y; };
              auto pt = static_cast<TVGPoint*>(value);
              Vector2 vec2(0.0f, 0.0f);
              if (val.Get(vec2)) {
                  pt->x = vec2.x;
                  pt->y = vec2.y;
                  return true;
              }
              break;
          }
          default: break;
      }
      return false;
  };

  // Common path for all properties
  tvg::LottiePropertyId lottieProp = tvg::LottiePropertyId::Unknown;
  switch (property) {
    case VectorProperty::FILL_COLOR:       lottieProp = tvg::LottiePropertyId::FillColor;          break;
    case VectorProperty::FILL_OPACITY:     lottieProp = tvg::LottiePropertyId::FillOpacity;        break;
    case VectorProperty::STROKE_COLOR:     lottieProp = tvg::LottiePropertyId::StrokeColor;        break;
    case VectorProperty::STROKE_OPACITY:   lottieProp = tvg::LottiePropertyId::StrokeOpacity;      break;
    case VectorProperty::STROKE_WIDTH:     lottieProp = tvg::LottiePropertyId::StrokeWidth;        break;
    case VectorProperty::TRIM_START:       lottieProp = tvg::LottiePropertyId::TrimStart;          break;
    case VectorProperty::TRIM_END:         lottieProp = tvg::LottiePropertyId::TrimEnd;            break;
    case VectorProperty::TRANSFORM_POSITION: lottieProp = tvg::LottiePropertyId::TransformPosition; break;
    case VectorProperty::TRANSFORM_SCALE:  lottieProp = tvg::LottiePropertyId::TransformScale;     break;
    case VectorProperty::TRANSFORM_ROTATION: lottieProp = tvg::LottiePropertyId::TransformRotation; break;
    case VectorProperty::TRANSFORM_OPACITY: lottieProp = tvg::LottiePropertyId::TransformOpacity;  break;
    default: delete callback; return; // unsupported – must free since callback ownership not yet transferred
  }

  // Remove existing callback for the same keypath and property to prevent bloat/conflict
  mPropertyCallbacks.erase(std::remove_if(mPropertyCallbacks.begin(), mPropertyCallbacks.end(),
                                         [&](const std::shared_ptr<PropertyCallback>& p) {
                                           return p->keyPath == keyPath && p->property == property;
                                         }),
                          mPropertyCallbacks.end());

  auto cb = std::make_shared<PropertyCallback>();
  cb->keyPath  = keyPath;
  cb->property = property;
  cb->callback = std::unique_ptr<CallbackBase>(callback);
  cb->id       = id;
  mPropertyCallbacks.push_back(cb);

  static_cast<tvg::LottieAnimation*>(mAnimation.get())->overrideProperty(keyPath.c_str(), lottieProp, wrapper, cb.get());
}

void VectorAnimationRendererNative::KeepRasterizedBuffer()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(DALI_UNLIKELY(mFinalized))
  {
    return;
  }

  mEnableFixedCache = true;
  mDecodedBuffers.clear();
}

void VectorAnimationRendererNative::SetEnableAspectFit(bool enable)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(DALI_UNLIKELY(mFinalized))
  {
    return;
  }

  if(mEnableAspectFit != enable)
  {
    mEnableAspectFit = enable;
    mPendingSizeUpdate = true;  // Trigger size update to apply new setting
  }
}

bool VectorAnimationRendererNative::IsEnableAspectFit() const
{
  Dali::Mutex::ScopedLock lock(mMutex);
  return mEnableAspectFit;
}

VectorAnimationRendererNative::UploadCompletedSignalType& VectorAnimationRendererNative::UploadCompletedSignal()
{
  return mUploadCompletedSignal;
}

void VectorAnimationRendererNative::NotifyEvent()
{
  bool emitSignal = false;
  {
    Dali::Mutex::ScopedLock lock(mMutex);

    if(mResourceReadyTriggered)
    {
      DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Set Texture [%p]\n", this);

      if(mRenderer && GetTargetTexture())
      {
        Dali::TextureSet textureSet = mRenderer.GetTextures();
        if(textureSet)
        {
          textureSet.SetTexture(0u, GetTargetTexture());
        }
      }
      mResourceReadyTriggered = false;
      emitSignal              = true;
    }
    OnNotify();
  }

  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    mPreviousRenderingData.clear();
  }

  if(emitSignal)
  {
    mUploadCompletedSignal.Emit();
  }
}

bool VectorAnimationRendererNative::IsRenderReady() const
{
  return mResourceReady;
}

// --- Lottie JSON parsing helpers ---

namespace
{

size_t SkipJsonValue(const std::string& json, size_t pos)
{
  while(pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
  {
    ++pos;
  }

  if(pos >= json.size()) return std::string::npos;

  char ch = json[pos];

  if(ch == '"')
  {
    ++pos;
    while(pos < json.size())
    {
      if(json[pos] == '\\')
      {
        pos += 2;
        continue;
      }
      if(json[pos] == '"') return pos + 1;
      ++pos;
    }
    return std::string::npos;
  }

  if(ch == '{' || ch == '[')
  {
    char open  = ch;
    char close = (ch == '{') ? '}' : ']';
    int  depth = 1;
    ++pos;
    bool inString = false;
    while(pos < json.size() && depth > 0)
    {
      if(inString)
      {
        if(json[pos] == '\\')
        {
          ++pos;
        }
        else if(json[pos] == '"')
        {
          inString = false;
        }
      }
      else
      {
        if(json[pos] == '"')
        {
          inString = true;
        }
        else if(json[pos] == open)
        {
          ++depth;
        }
        else if(json[pos] == close)
        {
          --depth;
        }
      }
      ++pos;
    }
    return pos;
  }

  while(pos < json.size() && json[pos] != ',' && json[pos] != '}' && json[pos] != ']' &&
        json[pos] != ' ' && json[pos] != '\t' && json[pos] != '\n' && json[pos] != '\r')
  {
    ++pos;
  }
  return pos;
}

std::string ExtractJsonString(const std::string& json, size_t pos, size_t& endPos)
{
  while(pos < json.size() && json[pos] != '"') ++pos;
  if(pos >= json.size())
  {
    endPos = std::string::npos;
    return {};
  }

  ++pos;
  std::string result;
  while(pos < json.size())
  {
    if(json[pos] == '\\' && pos + 1 < json.size())
    {
      result += json[pos + 1];
      pos += 2;
      continue;
    }
    if(json[pos] == '"')
    {
      endPos = pos + 1;
      return result;
    }
    result += json[pos];
    ++pos;
  }
  endPos = std::string::npos;
  return {};
}

float ExtractJsonNumber(const std::string& json, size_t pos, size_t& endPos)
{
  while(pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')) ++pos;
  size_t start = pos;
  while(pos < json.size() && json[pos] != ',' && json[pos] != '}' && json[pos] != ']' &&
        json[pos] != ' ' && json[pos] != '\t' && json[pos] != '\n' && json[pos] != '\r')
  {
    ++pos;
  }
  endPos = pos;
  try
  {
    return std::stof(json.substr(start, pos - start));
  }
  catch(...)
  {
    return 0.0f;
  }
}

void ParseJsonArray(const std::string& jsonContent, const char* arrayKey, size_t keyLen,
                    const char* nameKey, const char* val1Key, const char* val2Key,
                    bool val2IsDuration, bool convertTimeToFrame, float frameRate,
                    Property::Map& outMap)
{
  // Find the top-level array by looking for the pattern at the root level
  // We need to skip any nested arrays with the same key (e.g., assets[].layers)
  std::string searchKey = std::string("\"") + arrayKey + "\"";
  size_t arrPos = 0;
  int depth = 0;
  bool inString = false;

  // Search for the top-level occurrence of the key
  for(size_t i = 0; i < jsonContent.size(); ++i)
  {
    char ch = jsonContent[i];

    if(inString)
    {
      if(ch == '\\' && i + 1 < jsonContent.size())
      {
        ++i;  // skip escaped character
        continue;
      }
      if(ch == '"')
      {
        inString = false;
      }
    }
    else
    {
      if(ch == '{' || ch == '[')
      {
        ++depth;
      }
      else if(ch == '}' || ch == ']')
      {
        --depth;
      }
      else if(ch == '"')
      {
        // Check if this is the key we're looking for at top level (depth == 1)
        if(depth == 1 && jsonContent.compare(i, searchKey.length(), searchKey) == 0)
        {
          arrPos = i;
          break;
        }
        inString = true;
      }
    }
  }

  if(arrPos == 0) return;

  size_t arrStart = jsonContent.find('[', arrPos + keyLen + 2);
  if(arrStart == std::string::npos) return;

  size_t pos = arrStart + 1;
  while(pos < jsonContent.size())
  {
    while(pos < jsonContent.size() && (jsonContent[pos] == ' ' || jsonContent[pos] == '\t' ||
                                       jsonContent[pos] == '\n' || jsonContent[pos] == '\r' || jsonContent[pos] == ','))
    {
      ++pos;
    }

    if(pos >= jsonContent.size() || jsonContent[pos] == ']') break;

    if(jsonContent[pos] == '{')
    {
      size_t      objEnd = SkipJsonValue(jsonContent, pos);
      std::string obj    = jsonContent.substr(pos, objEnd - pos);

      std::string name;
      float       v1 = 0.0f, v2 = 0.0f;
      bool        hasName = false, hasV1 = false, hasV2 = false;

      size_t opos = 0;
      while(opos < obj.size())
      {
        size_t keyStart = obj.find('"', opos);
        if(keyStart == std::string::npos) break;

        size_t      keyEndPos;
        std::string key = ExtractJsonString(obj, keyStart, keyEndPos);
        if(keyEndPos == std::string::npos) break;

        size_t colonPos = obj.find(':', keyEndPos);
        if(colonPos == std::string::npos) break;
        size_t valPos = colonPos + 1;

        if(key == nameKey)
        {
          name    = ExtractJsonString(obj, valPos, keyEndPos);
          hasName = true;
          opos    = keyEndPos;
        }
        else if(key == val1Key)
        {
          v1    = ExtractJsonNumber(obj, valPos, keyEndPos);
          hasV1 = true;
          opos  = keyEndPos;
        }
        else if(key == val2Key)
        {
          v2    = ExtractJsonNumber(obj, valPos, keyEndPos);
          hasV2 = true;
          opos  = keyEndPos;
        }
        else
        {
          opos = SkipJsonValue(obj, valPos);
          if(opos == std::string::npos) break;
        }
      }

      if(hasName && hasV1 && hasV2)
      {
        int frame1, frame2;

        if(convertTimeToFrame && frameRate > 0.0f)
        {
          // Convert time in seconds to frame number (for markers: tm and dr are in seconds)
          frame1 = static_cast<int>(v1 * frameRate);
          frame2 = val2IsDuration ? static_cast<int>((v1 + v2) * frameRate) : static_cast<int>(v2 * frameRate);
        }
        else
        {
          // Values are already frame numbers (for layers: ip and op are frame numbers)
          frame1 = static_cast<int>(v1);
          frame2 = val2IsDuration ? static_cast<int>(v1 + v2) : static_cast<int>(v2);
        }

        Property::Array frames;
        frames.PushBack(Property::Value(frame1));
        frames.PushBack(Property::Value(frame2));
        outMap.Add(Dali::String(name.c_str()), frames);
      }

      pos = objEnd;
    }
    else
    {
      pos = SkipJsonValue(jsonContent, pos);
      if(pos == std::string::npos) break;
    }
  }
}

} // unnamed namespace

void VectorAnimationRendererNative::ParseLottieMetadata() const
{
  if(mMetadataParsed)
  {
    return;
  }
  mMetadataParsed = true;

  std::string jsonContent = mJsonData;
  if(jsonContent.empty() && !mUrl.empty())
  {
    std::ifstream file(mUrl, std::ios::binary);
    if(file.is_open())
    {
      std::ostringstream ss;
      ss << file.rdbuf();
      jsonContent = ss.str();
    }
  }

  if(jsonContent.empty())
  {
    return;
  }

  // "layers": [{"nm":"name", "ip":0, "op":60, ...}, ...]
  // For layers: ip and op are already frame numbers, no conversion needed
  ParseJsonArray(jsonContent, "layers", 6, "nm", "ip", "op", false, false, mFrameRate, mCachedLayerInfo);

  // "markers": [{"cm":"name", "tm":10, "dr":20}, ...]
  // For markers: tm (time) and dr (duration) are in seconds, need to convert to frame numbers
  ParseJsonArray(jsonContent, "markers", 7, "cm", "tm", "dr", true, true, mFrameRate, mCachedMarkerInfo);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
