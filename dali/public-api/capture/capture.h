#ifndef DALI_CAPTURE_H
#define DALI_CAPTURE_H

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

// EXTERNAL HEADERS
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/actors/camera-actor.h>
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class Capture;
}
} // namespace DALI_INTERNAL

/**
 * @brief Capture snapshots the current scene and save as a file.
 *
 * @SINCE_1_3.4
 *
 * Applications should follow the example below to create capture :
 *
 * @code
 * Capture capture = Capture::New();
 * @endcode
 *
 * If required, you can also connect class member function to a signal :
 *
 * @code
 * capture.FinishedSignal().Connect(this, &CaptureSceneExample::OnCaptureFinished);
 * @endcode
 *
 * At the connected class member function, you can know whether capture finish state.
 *
 * @code
 * void CaptureSceneExample::OnCaptureFinished( Capture capture, Capture::FinishState state )
 * {
 *   if ( state == Capture::FinishState::SUCCEEDED )
 *   {
 *     // Do something
 *   }
 *   else
 *   {
 *     // Do something
 *   }
 * }
 * @endcode
 */
class DALI_ADAPTOR_API Capture : public BaseHandle
{
public:
  /**
   * @brief The enumerations used for checking capture success
   * @SINCE_1_3_4
   */
  enum class FinishState
  {
    SUCCEEDED, ///< Succeeded in saving the result after capture
    FAILED     ///< Failed to capture by time out or to save the result
  };

  /**
   * @brief Typedef for finished signals sent by this class.
   *
   * @SINCE_1_3_4
   */
  typedef Signal<void(Capture, Capture::FinishState)> CaptureFinishedSignalType;

  /**
   * @brief Create an uninitialized Capture; this can be initialized with Actor::New().
   *
   * @SINCE_1_3_4
   *
   * Calling member functions with an uninitialized Dali::Object is not allowed.
   */
  Capture();

  /**
   * @brief Create an initialized Capture.
   *
   * @SINCE_1_3_4
   *
   * @return A handle to a newly allocated Dali resource.
   * @note Projection mode of default cameraActor is Dali::Camera::PERSPECTIVE_PROJECTION
   */
  static Capture New();

  /**
   * @brief Create an initialized Capture.
   *
   * @SINCE_1_3_4
   *
   * @param[in] cameraActor An initialized CameraActor.
   * @return A handle to a newly allocated Dali resource.
   */
  static Capture New(Dali::CameraActor cameraActor);

  /**
   * @brief Downcast an Object handle to Capture handle.
   *
   * @SINCE_1_3_4
   *
   * If handle points to a Capture object the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle to An object.
   * @return handle to a Capture object or an uninitialized handle.
   */
  static Capture DownCast(BaseHandle handle);

  /**
   * @brief Dali::Actor is intended as a base class.
   *
   * @SINCE_1_3_4
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~Capture();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @SINCE_1_3_4
   *
   * @param[in] copy A reference to the copied handle.
   */
  Capture(const Capture& copy);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @SINCE_1_3_4
   *
   * @param[in] rhs  A reference to the copied handle.
   * @return A reference to this.
   */
  Capture& operator=(const Capture& rhs);

  /**
   * @brief Move constructor.
   *
   * @SINCE_1_9.24
   * @param[in] rhs A reference to the moved handle
   */
  Capture(Capture&& rhs) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * @SINCE_1_9.24
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this handle
   */
  Capture& operator=(Capture&& rhs) noexcept;

  /**
   * @brief Start capture and save the image as a file.
   *
   * @SINCE_1_9.27
   * @param[in] source source actor to be used for capture.
   *            This source must be added on the window in advance.
   * @param[in] position top-left position of area to be captured
   *            this position is defined in the window.
   * @param[in] size captured size.
   * @param[in] path image file path to be saved as a file.
   *            If path is empty string, the captured result is not be saved as a file.
   * @param[in] clearColor background color of captured scene
   * @note suppose that we want to capture actor 'A'. And, the actor 'A' is overlapped by another actor 'B' that is not a child of 'A'.
   *       in this case, if source is root of scene, the captured image includes a part of actor 'B' on the 'A'.
   *       however, if source is just actor 'A', the result includes only 'A'.
   */
  void Start(Actor source, const Vector2& position, const Vector2& size, const std::string& path, const Vector4& clearColor);

  /**
   * @brief Start capture and save the image as a file.
   *
   * @SINCE_1_9.12
   *
   * @param[in] source source actor to be used for capture.
   *            This source must be added on the window in advance.
   * @param[in] size captured size.
   * @param[in] path image file path to be saved as a file.
   *            If path is empty string, the captured result is not be saved as a file.
   * @param[in] clearColor background color of captured scene
   * @param[in] quality The value to control image quality for jpeg file format in the range [1, 100]
   */
  void Start(Actor source, const Vector2& size, const std::string& path, const Vector4& clearColor, const uint32_t quality);

  /**
   * @brief Start capture and save the image as a file.
   *
   * @SINCE_1_3_4
   *
   * @param[in] source source actor to be used for capture.
   *            This source must be added on the window in advance.
   * @param[in] size captured size.
   * @param[in] path image file path to be saved as a file.
   *            If path is empty string, the captured result is not be saved as a file.
   * @param[in] clearColor background color of captured scene
   */
  void Start(Actor source, const Vector2& size, const std::string& path, const Vector4& clearColor);

  /**
   * @brief Start capture and save the image as a file.
   *
   * @SINCE_1_3_4
   *
   * @param[in] source source actor to be used for capture.
   *            This source must be added on the window in advance.
   * @param[in] size captured size.
   * @param[in] path image file path to be saved as a file.
   *            If path is empty string, the captured result is not be saved as a file.
   * @note Clear color is transparent.
   */
  void Start(Actor source, const Vector2& size, const std::string& path);

  /**
   * @brief Set result image quality in case of jpeg
   *
   * @param[in] quality The value to control image quality for jpeg file format in the range [1, 100]
   */
  void SetImageQuality(uint32_t quality);

  /**
   * @brief Get NativeImageSourcePtr that is saved captured image.
   *
   * @SINCE_1_9.10
   *
   * @return NativeImageSourcePtr Captured result that can be rendered with DALi
   */
  Dali::NativeImageSourcePtr GetNativeImageSource() const;

  /**
   * @brief Get finished signal.
   *
   * @SINCE_1_3_4
   *
   * @return finished signal instance.
   */
  CaptureFinishedSignalType& FinishedSignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief This constructor is used by New() methods.
   *
   * @SINCE_1_3_4
   *
   * @param[in] internal A pointer to a newly allocated Dali resource.
   */
  explicit DALI_INTERNAL Capture(Internal::Adaptor::Capture* internal);
  /// @endcond
};

/**
 * @}
 */

} // namespace Dali

#endif // DALI_CAPTURE_H
