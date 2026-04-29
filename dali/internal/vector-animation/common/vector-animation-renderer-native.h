#ifndef DALI_INTERNAL_VECTOR_ANIMATION_RENDERER_NATIVE_H
#define DALI_INTERNAL_VECTOR_ANIMATION_RENDERER_NATIVE_H

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

// EXTERNAL INCLUDES
#include <thorvg.h>
#include <thorvg_lottie.h>
#include <dali/devel-api/adaptor-framework/vector-animation-renderer-plugin.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/devel-api/common/vector-wrapper.h>
#include <dali/devel-api/threading/mutex.h>
#include <atomic>
#include <memory>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * @brief Built-in vector animation renderer using ThorVG backend.
 *
 * Provides native implementation of VectorAnimationRendererPlugin with ThorVG SwCanvas.
 * Subclasses implement platform-specific rendering target via CreateRenderingData(),
 * PrepareTarget(), IsTargetPrepared(), and GetTargetTexture().
 *
 * Thread safety: mMutex for main state, mRenderingDataMutex for rendering data,
 * atomic members for lock-free render thread access.
 */
class VectorAnimationRendererNative : public VectorAnimationRendererPlugin
{
public:
  /**
   * @brief Factory method. Creates platform-specific subclass instance.
   * @return A new instance of the platform-specific VectorAnimationRendererNative.
   */
  static VectorAnimationRendererNative* Create();

  VectorAnimationRendererNative();
  virtual ~VectorAnimationRendererNative();

  // VectorAnimationRendererPlugin interface

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Finalize()
   */
  void Finalize() override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Load(const std::string&)
   */
  bool Load(const std::string& url) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Load(const Dali::Vector<uint8_t>&)
   */
  bool Load(const Dali::Vector<uint8_t>& data) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::SetRenderer(Dali::Renderer)
   */
  void SetRenderer(Dali::Renderer renderer) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::SetSize(uint32_t, uint32_t)
   */
  void SetSize(uint32_t width, uint32_t height) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::Render(uint32_t)
   */
  virtual bool Render(uint32_t frameNumber) = 0;

  /**
   * @brief Updates ThorVG picture size if a size update is pending.
   * Must be called under mMutex lock.
   */
  void UpdateSizeIfNeeded();

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::RenderStopped()
   */
  void RenderStopped() override;

  /**
   * @brief Frees rendering buffers no longer in use.
   * Overridable by subclasses for platform-specific buffer release.
   */
  virtual void FreeReleasedBuffers();

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetTotalFrameNumber()
   */
  uint32_t GetTotalFrameNumber() const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetFrameRate()
   */
  float GetFrameRate() const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetDefaultSize(uint32_t&, uint32_t&)
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetLayerInfo(Property::Map&)
   */
  void GetLayerInfo(Property::Map& map) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetMarkerInfo(const std::string&, uint32_t&, uint32_t&)
   */
  bool GetMarkerInfo(const std::string& marker, uint32_t& startFrame, uint32_t& endFrame) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::GetMarkerInfo(Property::Map&)
   */
  void GetMarkerInfo(Property::Map& map) const override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::InvalidateBuffer()
   */
  void InvalidateBuffer() override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::UploadCompletedSignal()
   */
  UploadCompletedSignalType& UploadCompletedSignal() override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::AddPropertyValueCallback(const std::string&, VectorProperty, CallbackBase*, int32_t)
   * @note ThorVG backend does not yet support dynamic property callbacks; the callback is deleted.
   */
  void AddPropertyValueCallback(const std::string& keyPath, VectorProperty property, CallbackBase* callback, int32_t id) override;

  /**
   * @copydoc Dali::VectorAnimationRendererPlugin::KeepRasterizedBuffer()
   */
  void KeepRasterizedBuffer() override;

  /**
   * @brief Returns true if the animation resource is loaded and ready for display.
   */
  bool IsRenderReady() const;

  /**
   * @brief Called by VectorAnimationRendererEventManager in the main thread when a frame is ready.
   * Updates the texture set and emits UploadCompleted signal if needed.
   */
  void NotifyEvent();

protected:
  /**
   * @brief Rendering data container with target texture and dimensions.
   * Subclasses may extend with platform-specific data.
   */
  class RenderingData
  {
  public:
    Dali::Texture mTexture;  ///< Target texture for the rendered frame
    uint32_t      mWidth{0};  ///< Width in pixels
    uint32_t      mHeight{0}; ///< Height in pixels
  };

  class PropertyCallback
  {
  public:
    PropertyCallback() = default;
    ~PropertyCallback() = default;

    PropertyCallback(PropertyCallback&&) = default;
    PropertyCallback& operator=(PropertyCallback&&) = default;
    PropertyCallback(const PropertyCallback&) = delete;
    PropertyCallback& operator=(const PropertyCallback&) = delete;

    std::string                   keyPath;
    VectorProperty                property{VectorProperty::FILL_COLOR};
    std::unique_ptr<CallbackBase> callback;
    int32_t                       id{0};
  };

  /// @brief Creates a new RenderingData instance. Overridable for subclass extension.
  virtual std::shared_ptr<RenderingData> CreateRenderingData() = 0;

  /// @brief Prepares the rendering target (e.g., allocate buffers, create textures).
  virtual void PrepareTarget(std::shared_ptr<RenderingData> renderingData) = 0;

  /// @brief Returns true if the rendering target is prepared.
  virtual bool IsTargetPrepared() = 0;

  /// @brief Returns the current target texture.
  virtual Dali::Texture GetTargetTexture() = 0;

  VectorAnimationRendererNative(const VectorAnimationRendererNative&)            = delete;
  VectorAnimationRendererNative(VectorAnimationRendererNative&&)                 = delete;
  VectorAnimationRendererNative& operator=(const VectorAnimationRendererNative&) = delete;
  VectorAnimationRendererNative& operator=(VectorAnimationRendererNative&&)      = delete;

protected:
  /// @brief Registers with VectorAnimationRendererEventManager. Call from subclass constructor.
  void Initialize();

  /// @brief Parses Lottie JSON metadata (layers, markers) and caches results. Must be called under mMutex.
  void ParseLottieMetadata() const;

  /// @brief Emits the UploadCompleted signal.
  void EmitUploadCompleted();

  /// @brief Called during Finalize() for subclass-specific cleanup.
  virtual void OnFinalize() = 0;

  /// @brief Called during NotifyEvent() for subclass-specific post-render handling.
  virtual void OnNotify() = 0;

protected:

  // ThorVG core
  std::unique_ptr<tvg::SwCanvas>  mCanvas;     ///< ThorVG software canvas
  std::unique_ptr<tvg::Animation> mAnimation;   ///< ThorVG animation controller

  // DALi rendering
  UploadCompletedSignalType mUploadCompletedSignal;  ///< Upload completed signal

  // Buffer cache
  std::vector<std::pair<std::vector<uint8_t>, bool>> mDecodedBuffers; ///< Decoded frame buffer cache {pixel data, valid}

  // Rendering data lifecycle (protected by mRenderingDataMutex)
  std::vector<std::shared_ptr<RenderingData>> mPreviousRenderingData;  ///< Previous data awaiting release
  std::shared_ptr<RenderingData>              mPreparedRenderingData;  ///< Prepared data (pending activation)
  std::shared_ptr<RenderingData>              mCurrentRenderingData;   ///< Current active data

  // Source data
  std::string mUrl;       ///< File path of loaded animation
  std::string mJsonData;  ///< Raw JSON string for metadata parsing

  // Cached metadata (protected by mMutex, lazy-parsed)
  mutable Property::Map mCachedLayerInfo;   ///< Cached layer info: {name -> [startFrame, endFrame]}
  mutable Property::Map mCachedMarkerInfo;  ///< Cached marker info: {name -> [startFrame, endFrame]}

  // Mutexes
  mutable Dali::Mutex mMutex;              ///< Protects main renderer state
  mutable Dali::Mutex mRenderingDataMutex; ///< Protects rendering data lifecycle

  // DALi rendering (handle types)
  Dali::Renderer mRenderer;  ///< Associated DALi Renderer

  // Animation properties (4-byte types)
  std::atomic<uint32_t> mTotalFrame;      ///< Total frame count
  std::atomic<uint32_t> mDefaultWidth;    ///< Default width from source file
  std::atomic<uint32_t> mDefaultHeight;   ///< Default height from source file
  uint32_t              mTargetWidth;     ///< Target rendering width
  uint32_t              mTargetHeight;    ///< Target rendering height
  std::atomic<float>    mFrameRate;       ///< Frame rate (FPS)
  float                 mDuration;        ///< Duration in seconds

  // Dynamic property callbacks (STL container)
  std::vector<std::shared_ptr<PropertyCallback>> mPropertyCallbacks; ///< Dynamic property callbacks

  // Atomic bools (accessed from multiple threads, must not use bit-fields)
  std::atomic<bool> mResourceReady;          ///< Resource loaded and ready
  std::atomic<bool> mResourceReadyTriggered; ///< Resource-ready event triggered
  std::atomic<bool> mPendingSizeUpdate;      ///< Pending size update for ThorVG picture

  // Non-atomic bools (main thread only or mutex-protected)
  bool mFinalized : 1;                ///< Finalized flag
  bool mLoadFailed : 1;               ///< Load failure flag
  bool mEnableFixedCache : 1;         ///< Fixed cache mode flag
  mutable bool mMetadataParsed : 1;   ///< Metadata parsed flag
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_VECTOR_ANIMATION_RENDERER_NATIVE_H