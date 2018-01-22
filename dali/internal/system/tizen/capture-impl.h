#ifndef DALI_INTERNAL_CAPTURE_H
#define DALI_INTERNAL_CAPTURE_H

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

// EXTERNAL INCLUDES
#include <string>
#include <memory>
#include <tbm_surface.h>
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/common/dali-common.h>
#include <dali/public-api/render-tasks/render-task.h>
#include <dali/public-api/rendering/texture.h>
#include <dali/public-api/rendering/frame-buffer.h>

// INTERNAL INCLUDES
#include <native-image-source.h>
#include <timer.h>
#include <capture.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class Capture;
typedef IntrusivePtr<Capture> CapturePtr;

class Capture : public BaseObject, public ConnectionTracker
{
public:
  /**
   * @brief Constructor.
   */
  Capture();

  Capture( Dali::CameraActor cameraActor );

  /**
   * @copydoc Dali::Capture::New
   */
  static CapturePtr New();

  /**
   * @copydoc Dali::Capture::New
   */
  static CapturePtr New( Dali::CameraActor cameraActor );

  /**
   * @copydoc Dali::Capture::Start
   */
  void Start( Dali::Actor source, const Dali::Vector2& size, const std::string &path, const Dali::Vector4& clearColor );

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
   * @brief Create surface.
   *
   * @param[in] size of surface.
   */
  void CreateSurface( const Dali::Vector2& size );

  /**
   * @brief Delete surface.
   */
  void DeleteSurface();

  /**
   * @brief Clear surface with color.
   *
   * @param[in] size of clear aread.
   */
  void ClearSurface( const Dali::Vector2& size );

  /**
   * @brief Query whether surface is created or not.
   *
   * @return True is surface is created.
   */
  bool IsSurfaceCreated();

  /**
   * @brief Create native image source.
   */
  void CreateNativeImageSource();

  /**
   * @brief Delete native image source.
   */
  void DeleteNativeImageSource();

  /**
   * @brief Query whether native image source is created or not.
   *
   * @return True is native image source is created.
   */
  bool IsNativeImageSourceCreated();

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
   * @param[in] source is captured.
   * @param[in] clearColor background color
   */
  void SetupRenderTask( Dali::Actor source, const Dali::Vector4& clearColor );

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
   * @param[in] size is surface size.
   * @param[in] clearColor is clear color of surface.
   * @param[in] source is captured.
   */
  void SetupResources( const Dali::Vector2& size, const Dali::Vector4& clearColor, Dali::Actor source );

  /**
   * @brief Unset resources for capture.
   */
  void UnsetResources();

  /**
   * @brief Callback when render is finished.
   *
   * @param[in] task is used for capture.
   */
  void OnRenderFinished( Dali::RenderTask& task );

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
  bool Save();

  /**
   * @brief Checks privilege for Capture
   *
   * @param[in] privilege The capture privilege
   * @return True if input is capture privilege, false otherwise
   */
  bool CheckPrivilege( const char* privilege ) const;

private:

  // Undefined
  Capture( const Capture& );

  // Undefined
  Capture& operator=( const Capture& rhs );

private:
  Dali::Texture                               mNativeTexture;
  Dali::FrameBuffer                           mFrameBuffer;
  Dali::RenderTask                            mRenderTask;
  Dali::Actor                                 mParent;
  Dali::Actor                                 mSource;
  Dali::CameraActor                           mCameraActor;
  Dali::Timer                                 mTimer;           ///< For timeout.
  Dali::Capture::CaptureFinishedSignalType    mFinishedSignal;
  std::string                                 mPath;
  Dali::NativeImageSourcePtr                  mNativeImageSourcePtr;  ///< pointer to surface image
  tbm_surface_h                               mTbmSurface;
};

}  // End of namespace Adaptor
}  // End of namespace Internal

// Helpers for public-api forwarding methods

inline Internal::Adaptor::Capture& GetImpl( Dali::Capture& captureWorker)
{
  DALI_ASSERT_ALWAYS( captureWorker && "Capture handle is empty" );

  BaseObject& handle = captureWorker.GetBaseObject();

  return static_cast< Internal::Adaptor::Capture& >( handle );
}

inline const Internal::Adaptor::Capture& GetImpl( const Dali::Capture& captureWorker )
{
  DALI_ASSERT_ALWAYS( captureWorker && "Capture handle is empty" );

  const BaseObject& handle = captureWorker.GetBaseObject();

  return static_cast< const Internal::Adaptor::Capture& >( handle );
}

}  // End of namespace Dali

#endif // DALI_INTERNAL_CAPTURE_H
