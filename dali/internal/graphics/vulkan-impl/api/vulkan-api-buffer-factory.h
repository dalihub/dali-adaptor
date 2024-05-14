#ifndef DALI_GRAPHICS_VULKAN_API_BUFFER_FACTORY_H
#define DALI_GRAPHICS_VULKAN_API_BUFFER_FACTORY_H

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

#include <dali/graphics/vulkan/internal/vulkan-types.h>
#include <dali/graphics-api/graphics-api-buffer-factory.h>
#include <dali/graphics-api/graphics-api-buffer.h>
#include <dali/graphics-api/graphics-api-types.h>


namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
class Graphics;
}
namespace VulkanAPI
{
class Controller;

class BufferFactory : public Dali::Graphics::BufferFactory
{
public:

  explicit BufferFactory( Controller& controller );

  VulkanAPI::BufferFactory& SetUsageFlags( Dali::Graphics::BufferUsageFlags usage ) override;

  VulkanAPI::BufferFactory& SetSize( uint32_t size ) override;

  PointerType Create() const override;

private:

  Controller& mController;
  Vulkan::Graphics& mGraphics;
  vk::BufferUsageFlags mUsage;
  Dali::Graphics::Buffer::UsageHint mUsageHints;
  uint32_t mSize;
};

} // namespace VulkanAPI
} // namespace Graphics
} // namespace Dali

#endif // DALI_GRAPHICS_VULKAN_API_BUFFER_FACTORY_H
