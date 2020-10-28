#ifndef DALI_INTERNAL_VECTOR_IMAGE_RENDERER_PLUGIN_PROXY_H
#define DALI_INTERNAL_VECTOR_IMAGE_RENDERER_PLUGIN_PROXY_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/vector-image-renderer-plugin.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Proxy class to dynamically load, use and unload vector image renderer plugin.
 */
class VectorImageRendererPluginProxy
{
public:

  /**
   * @brief Constructor
   */
  VectorImageRendererPluginProxy( std::string sharedObjectName );

  /**
   * @brief Destructor
   */
  ~VectorImageRendererPluginProxy();

  /**
   * @brief Query whether the plugin is valid or not.
   * @return True if valid, false otherwise
   */
  bool IsValid() const;

  /**
   * @copydoc Dali::VectorImageRendererPlugin::Load()
   */
  bool Load(const Vector<uint8_t>& data);

  /**
   * @copydoc Dali::VectorImageRendererPlugin::Rasterize()
   */
  bool Rasterize(Dali::Devel::PixelBuffer& buffer, float scale);

  /**
   * @copydoc Dali::VectorImageRendererPlugin::GetDefaultSize()
   */
  void GetDefaultSize(uint32_t& width, uint32_t& height) const;

  // Not copyable or movable
  VectorImageRendererPluginProxy( const VectorImageRendererPluginProxy& ) = delete; ///< Deleted copy constructor
  VectorImageRendererPluginProxy( VectorImageRendererPluginProxy&& ) = delete; ///< Deleted move constructor
  VectorImageRendererPluginProxy& operator=( const VectorImageRendererPluginProxy& ) = delete; ///< Deleted copy assignment operator
  VectorImageRendererPluginProxy& operator=( VectorImageRendererPluginProxy&& ) = delete; ///< Deleted move assignment operator

private:

  /**
   * Dynamically loads the plugin.
   */
  void InitializePlugin();

private:

  using CreateVectorImageRendererFunction = Dali::VectorImageRendererPlugin* (*)();

  std::string                        mSharedObjectName;   ///< Shared object name
  void*                              mLibHandle;          ///< Handle for the loaded library
  Dali::VectorImageRendererPlugin*   mPlugin;             ///< Plugin handle

  CreateVectorImageRendererFunction  mCreateVectorImageRendererPtr;   ///< Function pointer called in adaptor to create a plugin instance

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_VECTOR_IMAGE_RENDERER_PLUGIN_PROXY_H
