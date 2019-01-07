#ifndef DALI_GRAPHICS_VULKAN_API_PIPELINE_FACTORY_H
#define DALI_GRAPHICS_VULKAN_API_PIPELINE_FACTORY_H

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

#include <dali/graphics-api/graphics-api-pipeline-factory.h>

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

class Pipeline;

class PipelineCache;

/**
 * @brief Interface class for ShaderFactory types in the graphics API.
 */
class PipelineFactory : public Dali::Graphics::PipelineFactory
{
public:

  PipelineFactory( Controller& controller );

  Dali::Graphics::PipelineFactory& SetPipelineCache( VulkanAPI::PipelineCache& pipelineCache );

  Dali::Graphics::PipelineFactory& SetColorBlendState( const Dali::Graphics::ColorBlendState& state ) override;

  Dali::Graphics::PipelineFactory& SetShaderState( const Dali::Graphics::ShaderState& state ) override;

  Dali::Graphics::PipelineFactory& SetViewportState( const Dali::Graphics::ViewportState& state ) override;

  Dali::Graphics::PipelineFactory& SetFramebufferState( const Dali::Graphics::FramebufferState& state ) override;

  Dali::Graphics::PipelineFactory& SetBasePipeline( Dali::Graphics::Pipeline& pipeline ) override;

  Dali::Graphics::PipelineFactory& SetDepthStencilState( Dali::Graphics::DepthStencilState state ) override;

  Dali::Graphics::PipelineFactory& SetRasterizationState( const Dali::Graphics::RasterizationState& state ) override;

  Dali::Graphics::PipelineFactory& SetVertexInputState( const Dali::Graphics::VertexInputState& state ) override;

  Dali::Graphics::PipelineFactory& SetInputAssemblyState( const Dali::Graphics::InputAssemblyState& state ) override;

  Dali::Graphics::PipelineFactory& SetDynamicStateMask( const Dali::Graphics::PipelineDynamicStateMask mask ) override;

  Dali::Graphics::PipelineFactory& SetOldPipeline( std::unique_ptr<Dali::Graphics::Pipeline> oldPipeline ) override;

  // To be called when getting new factory
  void Reset();

  std::unique_ptr< Dali::Graphics::Pipeline > Create() override;

  uint32_t GetHashCode() const;

  struct Info
  {
    Dali::Graphics::DepthStencilState depthStencilState;
    Dali::Graphics::ColorBlendState colorBlendState;
    Dali::Graphics::ShaderState shaderState;
    Dali::Graphics::ViewportState viewportState;
    Dali::Graphics::FramebufferState framebufferState;
    Dali::Graphics::RasterizationState rasterizationState;
    Dali::Graphics::VertexInputState vertexInputState;
    Dali::Graphics::InputAssemblyState inputAssemblyState;
    Dali::Graphics::PipelineDynamicStateMask dynamicStateMask;
  };

  const Info& GetCreateInfo() const
  {
    return mInfo;
  }

public:

  Info mInfo;
  Controller& mController;
  Vulkan::Graphics& mGraphics;
  VulkanAPI::PipelineCache* mPipelineCache;
  VulkanAPI::Pipeline* mBasePipeline;
  mutable uint32_t mHashCode;
  std::unique_ptr<Dali::Graphics::Pipeline> mOldPipeline { nullptr };
};

} // namespace API
} // namespace Graphics
} // namespace Dali

#endif // DALI_GRAPHICS_VULKAN_API_PIPELINE_FACTORY_H
