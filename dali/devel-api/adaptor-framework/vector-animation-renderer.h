#ifndef DALI_VECTOR_ANIMATION_RENDERER_H
#define DALI_VECTOR_ANIMATION_RENDERER_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/rendering/renderer.h>

// INTERNAL INCLUDES
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
class VectorAnimationRenderer;
}
}

/**
 * @brief Used for rendering a vector animation file
 */
class DALI_ADAPTOR_API VectorAnimationRenderer : public BaseHandle
{
public:

  /// @brief UploadCompleted signal type.
  using UploadCompletedSignalType = Signal< void () >;

  /**
   * @brief Creates an initialized handle to a new VectorAnimationRenderer.
   *
   * @param[in] url The url of the vector animation file
   * @return A handle to a newly allocated VectorAnimationRenderer
   */
  static VectorAnimationRenderer New( const std::string& url );

  /**
   * @brief Creates an empty handle.
   * Use VectorAnimationRenderer::New() to create an initialized object.
   */
  VectorAnimationRenderer();

  /**
   * @brief Destructor.
   */
  ~VectorAnimationRenderer();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  VectorAnimationRenderer( const VectorAnimationRenderer& handle );

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param[in] rhs A reference to the copied handle
   * @return A reference to this
   */
  VectorAnimationRenderer& operator=( const VectorAnimationRenderer& rhs );

  /**
   * @brief Sets the renderer used to display the result image.
   *
   * @param[in] renderer The renderer used to display the result image
   */
  void SetRenderer( Renderer renderer );

  /**
   * @brief Sets the target image size.
   *
   * @param[in] width The target image width
   * @param[in] height The target image height
   */
  void SetSize( uint32_t width, uint32_t height );

  /**
   * @brief Renders the content to the target buffer synchronously.
   *
   * @param[in] frameNumber The frame number to be rendered
   * @return True if the rendering success, false otherwise.
   */
  bool Render( uint32_t frameNumber );

  /**
   * @brief Gets the total number of frames of the file
   *
   * @return The total number of frames
   */
  uint32_t GetTotalFrameNumber() const;

  /**
   * @brief Gets the frame rate of the file.
   *
   * @return The frame rate of the file
   */
  float GetFrameRate() const;

  /**
   * @brief Gets the default size of the file.
   *
   * @param[out] width The default width of the file
   * @param[out] height The default height of the file
   */
  void GetDefaultSize( uint32_t& width, uint32_t& height ) const;

  /**
   * @brief Gets the layer information of all the child layers.
   *
   * @param[out] map The layer information
   */
  void GetLayerInfo( Property::Map& map ) const;

public: // Signals

  /**
   * @brief Connect to this signal to be notified when the texture upload is completed.
   *
   * @return The signal to connect to.
   */
  UploadCompletedSignalType& UploadCompletedSignal();

public: // Not intended for application developers

  /// @cond internal
  /**
   * @brief The constructor.
   * @note  Not intended for application developers.
   *
   * @param[in] pointer A pointer to a newly allocated VectorAnimationRenderer
   */
  explicit DALI_INTERNAL VectorAnimationRenderer( Internal::Adaptor::VectorAnimationRenderer* internal );
  /// @endcond

};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_VECTOR_ANIMATION_RENDERER_H
