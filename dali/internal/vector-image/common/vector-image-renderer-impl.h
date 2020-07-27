#ifndef DALI_INTERNAL_VECTOR_IMAGE_RENDERER_IMPL_H
#define DALI_INTERNAL_VECTOR_IMAGE_RENDERER_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/vector-image-renderer.h>
#include <dali/internal/vector-image/common/vector-image-renderer-plugin-proxy.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class VectorImageRenderer;
using VectorImageRendererPtr = IntrusivePtr< VectorImageRenderer >;

/**
 * Dali internal VectorImageRenderer.
 */
class VectorImageRenderer : public BaseObject, public ConnectionTracker
{
public:

  /**
   * @brief Creates a VectorImageRenderer object.
   *
   * @return A handle to a newly allocated VectorImageRenderer
   */
  static VectorImageRendererPtr New();

  /**
   * @brief Initializes member data.
   */
  void Initialize();

  /**
   * @copydoc Dali::VectorImageRenderer::SetBuffer()
   */
  void SetBuffer( Dali::Devel::PixelBuffer &buffer );

  /**
   * @copydoc Dali::VectorImageRenderer::Render()
   */
  bool Render(float scale);

  /**
   * @copydoc Dali::VectorImageRenderer::Load()
   */
  bool Load( const std::string& url );

  /**
   * @copydoc Dali::VectorImageRenderer::Load()
   */
  bool Load( const char *data, uint32_t size );

  /**
   * @copydoc Dali::VectorImageRenderer::GetDefaultSize()
   */
  void GetDefaultSize( uint32_t& width, uint32_t& height ) const;

  VectorImageRenderer( const VectorImageRenderer& ) = delete;
  VectorImageRenderer& operator=( VectorImageRenderer& )  = delete;
  VectorImageRenderer( VectorImageRenderer&& ) = delete;
  VectorImageRenderer& operator=( VectorImageRenderer&& )  = delete;

private:

  /**
   * @brief Constructor
   */
  VectorImageRenderer();

  /**
   * @brief Destructor.
   */
  ~VectorImageRenderer() = default;

private:

  VectorImageRendererPluginProxy mPlugin;
};

} // namespace Adaptor

} // namespace Internal

inline static Internal::Adaptor::VectorImageRenderer& GetImplementation( Dali::VectorImageRenderer& renderer )
{
  DALI_ASSERT_ALWAYS( renderer && "VectorImageRenderer handle is empty." );

  BaseObject& handle = renderer.GetBaseObject();

  return static_cast< Internal::Adaptor::VectorImageRenderer& >( handle );
}

inline static const Internal::Adaptor::VectorImageRenderer& GetImplementation( const Dali::VectorImageRenderer& renderer )
{
  DALI_ASSERT_ALWAYS( renderer && "VectorImageRenderer handle is empty." );

  const BaseObject& handle = renderer.GetBaseObject();

  return static_cast< const Internal::Adaptor::VectorImageRenderer& >( handle );
}

} // namespace Dali

#endif // DALI_INTERNAL_VECTOR_IMAGE_RENDERER_IMPL_H
