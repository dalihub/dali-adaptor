#ifndef DALI_INTERNAL_CAPTURE_H
#define DALI_INTERNAL_CAPTURE_H

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

// EXTERNAL INCLUDES
#include <dali/integration-api/processor-interface.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/object/weak-handle.h>
#include <dali/public-api/render-tasks/render-task.h>
#include <dali/public-api/rendering/frame-buffer.h>
#include <dali/public-api/rendering/texture.h>
#include <memory>
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/capture/capture.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class Capture;
typedef IntrusivePtr<Capture> CapturePtr;

class Capture : public BaseObject, public ConnectionTracker, public Integration::Processor
{
public:
  static constexpr uint32_t DEFAULT_QUALITY = 100;

  /**
   * @brief Constructor.
   */
  Capture();

  Capture(Dali::CameraActor cameraActor);

  /**
   * @copydoc Dali::Capture::New
   */
  static CapturePtr New();

  /**
   * @copydoc Dali::Capture::New
   */
  static CapturePtr New(Dali::CameraActor cameraActor);

  /**
   * @copydoc Dali::Capture::Start
   */
  void Start(Dali::Actor source, const Dali::Vector2& position, const Dali::Vector2& size, const std::string& path, const Dali::Vector4& clearColor, const uint32_t quality);

  /**
   * @copydoc Dali::Capture::Start
   */
  void Start(Dali::Actor source, const Dali::Vector2& position, const Dali::Vector2& size, const std::string& path, const Dali::Vector4& clearColor);

  /**
   * @copydoc Dali::Capture::SetImageQuality
   */
  void SetImageQuality(uint32_t quality);

  /**
   * @copydoc Dali::Capture::SetExclusive
   */
  void SetExclusive(bool exclusive);

  /**
   * @copydoc Dali::Capture::IsExclusive
   */
  bool IsExclusive() const;

  /**
   * @copydoc Dali::Capture::GetNativeImageSource
   */
  Dali::NativeImageSourcePtr GetNativeImageSource() const;

  /**
   * @copydoc Dali::Capture::GetCapturedBuffer
   */
  Dali::Devel::PixelBuffer GetCapturedBuffer();

  /**
   * @copydoc Dali::Capture::FinishedSignal
   */
  Dali::Capture::CaptureFinishedSignalType& FinishedSignal();

protected:
  /**
   * @brief A reference counted object may only be deleted by calling Unreference()
   */
  virtual ~Capture();

private:
  /**
   * @brief Create texture.
   */
  void CreateTexture(const Dali::Vector2& size);

  /**
   * @brief Delete native image source.
   */
  void DeleteNativeImageSource();

  /**
   * @brief Create frame buffer.
   */
  void CreateFrameBuffer();

  /**
   * @brief Delete frame buffer.
   */
  void DeleteFrameBuffer();

  /**
   * @brief Query whether frame buffer is created or not.
   *
   * @return True is frame buffer is created.
   */
  bool IsFrameBufferCreated();

  /**
   * @brief Setup render task.
   *
   * @param[in] position top-left position of area to be captured
   *            this position is defined in the window.
   * @param[in] size two dimensional size of area to be captured
   * @param[in] source sub-scene tree to be captured.
   * @param[in] clearColor background color
   */
  void SetupRenderTask(const Dali::Vector2& position, const Dali::Vector2& size, Dali::Actor source, const Dali::Vector4& clearColor);

  /**
   * @brief Unset render task.
   */
  void UnsetRenderTask();

  /**
   * @brief Query whether render task is setup or not.
   *
   * @return True is render task is setup.
   */
  bool IsRenderTaskSetup();

  /**
   * @brief Setup resources for capture.
   *
   * @param[in] position top-left position of area to be captured
   *            this position is defined in the window.
   * @param[in] size two dimensional size of area to be captured
   * @param[in] clearColor color to clear background surface.
   * @param[in] source sub-scene tree to be captured.
   */
  void SetupResources(const Dali::Vector2& position, const Dali::Vector2& size, const Dali::Vector4& clearColor, Dali::Actor source);

  /**
   * @brief Unset resources for capture.
   */
  void UnsetResources();

  /**
   * @brief Callback when render is finished.
   *
   * @param[in] task is used for capture.
   */
  void OnRenderFinished(Dali::RenderTask& task);

  /**
   * @brief Callback when timer is finished.
   *
   * @return True is timer start again.
   */
  bool OnTimeOut();

  /**
   * @brief Save framebuffer.
   *
   * @return True is success to save, false is fail.
   */
  bool SaveFile();

private:
  // Undefined
  Capture(const Capture&);

  // Undefined
  Capture& operator=(const Capture& rhs);

protected: // Implementation of Processor
  /**
   * @copydoc Dali::Integration::Processor::Process()
   */
  void Process(bool postProcessor) override;

  /**
   * @copydoc Dali::Integration::Processor::GetProcessorName()
   */
  std::string_view GetProcessorName() const override
  {
    return "Capture";
  }

private:
  uint32_t                                         mQuality;
  Dali::Texture                                    mTexture;
  Dali::WeakHandle<Dali::Integration::SceneHolder> mSceneHolderHandle;
  Dali::FrameBuffer                                mFrameBuffer;
  Dali::RenderTask                                 mRenderTask;
  Dali::Actor                                      mSource;
  Dali::CameraActor                                mCameraActor;
  Dali::Timer                                      mTimer; ///< For timeout.
  Dali::Capture::CaptureFinishedSignalType         mFinishedSignal;
  std::string                                      mPath;
  Dali::NativeImageSourcePtr                       mNativeImageSourcePtr; ///< pointer to surface image
  Dali::Devel::PixelBuffer                         mPixelBuffer;
  bool                                             mInCapture{false};
  bool                                             mIsExclusive{false};
  bool                                             mFileSave;
  bool                                             mUseDefaultCamera;                   // Whether we use default generated camera, or use inputed camera.
  bool                                             mSceneOffCameraAfterCaptureFinished; // Whether we need to scene-off after capture finished.
};

} // End of namespace Adaptor
} // End of namespace Internal

// Helpers for public-api forwarding methods

inline Internal::Adaptor::Capture& GetImpl(Dali::Capture& captureWorker)
{
  DALI_ASSERT_ALWAYS(captureWorker && "Capture handle is empty");

  BaseObject& handle = captureWorker.GetBaseObject();

  return static_cast<Internal::Adaptor::Capture&>(handle);
}

inline const Internal::Adaptor::Capture& GetImpl(const Dali::Capture& captureWorker)
{
  DALI_ASSERT_ALWAYS(captureWorker && "Capture handle is empty");

  const BaseObject& handle = captureWorker.GetBaseObject();

  return static_cast<const Internal::Adaptor::Capture&>(handle);
}

} // End of namespace Dali

#endif // DALI_INTERNAL_CAPTURE_H
