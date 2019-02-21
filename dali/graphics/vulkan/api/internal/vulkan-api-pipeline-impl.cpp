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

#include <dali/graphics/vulkan/api/internal/vulkan-pipeline-cache.h>
#include <dali/graphics/vulkan/api/internal/vulkan-api-pipeline-impl.h>
#include <dali/graphics/vulkan/api/vulkan-api-pipeline-factory.h>
#include <dali/graphics/vulkan/api/vulkan-api-pipeline.h>
#include <dali/graphics/vulkan/api/vulkan-api-controller.h>
#include <dali/graphics/vulkan/internal/spirv/vulkan-spirv.h>
#include <dali/graphics/vulkan/internal/vulkan-shader.h>
#include <dali/graphics/vulkan/internal/vulkan-command-buffer.h>
#include <dali/graphics/vulkan/internal/vulkan-swapchain.h>
#include <dali/graphics/vulkan/vulkan-graphics.h>
#include <dali/graphics/vulkan/internal/vulkan-framebuffer.h>

#include <dali/graphics/vulkan/api/vulkan-api-shader.h>

#include <array>
#include <dali/graphics/vulkan/api/vulkan-api-framebuffer.h>

namespace Dali
{
namespace Graphics
{
namespace VulkanAPI
{
namespace Internal
{
using Vulkan::VkAssert;
namespace
{
constexpr vk::BlendOp ConvertBlendOp( Dali::Graphics::BlendOp blendOp )
{
  switch( blendOp )
  {
    case Dali::Graphics::BlendOp::ADD:
      return vk::BlendOp::eAdd;
    case Dali::Graphics::BlendOp::SUBTRACT:
      return vk::BlendOp::eSubtract;
    case Dali::Graphics::BlendOp::REVERSE_SUBTRACT:
      return vk::BlendOp::eReverseSubtract;
    case Dali::Graphics::BlendOp::MIN:
      return vk::BlendOp::eMin;
    case Dali::Graphics::BlendOp::MAX:
      return vk::BlendOp::eMax;
  }
  return vk::BlendOp{};
}

constexpr vk::BlendFactor ConvertBlendFactor( Dali::Graphics::BlendFactor blendFactor )
{
  switch( blendFactor )
  {
    case Dali::Graphics::BlendFactor::ZERO:
      return vk::BlendFactor::eZero;
    case Dali::Graphics::BlendFactor::ONE:
      return vk::BlendFactor::eOne;
    case Dali::Graphics::BlendFactor::SRC_COLOR:
      return vk::BlendFactor::eSrcColor;
    case Dali::Graphics::BlendFactor::ONE_MINUS_SRC_COLOR:
      return vk::BlendFactor::eOneMinusSrcAlpha;
    case Dali::Graphics::BlendFactor::DST_COLOR:
      return vk::BlendFactor::eDstColor;
    case Dali::Graphics::BlendFactor::ONE_MINUS_DST_COLOR:
      return vk::BlendFactor::eOneMinusDstColor;
    case Dali::Graphics::BlendFactor::SRC_ALPHA:
      return vk::BlendFactor::eSrcAlpha;
    case Dali::Graphics::BlendFactor::ONE_MINUS_SRC_ALPHA:
      return vk::BlendFactor::eOneMinusSrcAlpha;
    case Dali::Graphics::BlendFactor::DST_ALPHA:
      return vk::BlendFactor::eDstAlpha;
    case Dali::Graphics::BlendFactor::ONE_MINUS_DST_ALPHA:
      return vk::BlendFactor::eOneMinusDstAlpha;
    case Dali::Graphics::BlendFactor::CONSTANT_COLOR:
      return vk::BlendFactor::eConstantColor;
    case Dali::Graphics::BlendFactor::ONE_MINUS_CONSTANT_COLOR:
      return vk::BlendFactor::eOneMinusConstantColor;
    case Dali::Graphics::BlendFactor::CONSTANT_ALPHA:
      return vk::BlendFactor::eConstantAlpha;
    case Dali::Graphics::BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
      return vk::BlendFactor::eOneMinusConstantAlpha;
    case Dali::Graphics::BlendFactor::SRC_ALPHA_SATURATE:
      return vk::BlendFactor::eSrcAlphaSaturate;
    case Dali::Graphics::BlendFactor::SRC1_COLOR:
      return vk::BlendFactor::eSrc1Color;
    case Dali::Graphics::BlendFactor::ONE_MINUS_SRC1_COLOR:
      return vk::BlendFactor::eOneMinusSrc1Color;
    case Dali::Graphics::BlendFactor::SRC1_ALPHA:
      return vk::BlendFactor::eSrc1Alpha;
    case Dali::Graphics::BlendFactor::ONE_MINUS_SRC1_ALPHA:
      return vk::BlendFactor::eOneMinusSrc1Alpha;
  }
  return vk::BlendFactor{};
}

constexpr vk::CompareOp ConvertCompareOp( Dali::Graphics::CompareOp compareOp )
{
  switch( compareOp )
  {
    case Dali::Graphics::CompareOp::NEVER:
      return vk::CompareOp::eNever;
    case Dali::Graphics::CompareOp::LESS:
      return vk::CompareOp::eLess;
    case Dali::Graphics::CompareOp::EQUAL:
      return vk::CompareOp::eEqual;
    case Dali::Graphics::CompareOp::LESS_OR_EQUAL:
      return vk::CompareOp::eLessOrEqual;
    case Dali::Graphics::CompareOp::GREATER:
      return vk::CompareOp::eGreater;
    case Dali::Graphics::CompareOp::NOT_EQUAL:
      return vk::CompareOp::eNotEqual;
    case Dali::Graphics::CompareOp::GREATER_OR_EQUAL:
      return vk::CompareOp::eGreaterOrEqual;
    case Dali::Graphics::CompareOp::ALWAYS:
      return vk::CompareOp::eAlways;
  }
  return vk::CompareOp{};
}

constexpr vk::PrimitiveTopology ConvertPrimitiveTopology( Dali::Graphics::PrimitiveTopology topology )
{
  using Dali::Graphics::Pipeline;
  switch( topology )
  {
    case Dali::Graphics::PrimitiveTopology::POINT_LIST:
      return vk::PrimitiveTopology::ePointList;
    case Dali::Graphics::PrimitiveTopology::LINE_LIST:
      return vk::PrimitiveTopology::eLineList;
    case Dali::Graphics::PrimitiveTopology::LINE_STRIP:
      return vk::PrimitiveTopology::eLineStrip;
    case Dali::Graphics::PrimitiveTopology::TRIANGLE_LIST:
      return vk::PrimitiveTopology::eTriangleList;
    case Dali::Graphics::PrimitiveTopology::TRIANGLE_STRIP:
      return vk::PrimitiveTopology::eTriangleStrip;
    case Dali::Graphics::PrimitiveTopology::TRIANGLE_FAN:
      return vk::PrimitiveTopology::eTriangleFan;
  }
  return vk::PrimitiveTopology{};
}

constexpr vk::CullModeFlagBits ConvertCullMode( Dali::Graphics::CullMode cullMode )
{
  switch( cullMode )
  {
    case Dali::Graphics::CullMode::NONE:
      return vk::CullModeFlagBits::eNone;
    case Dali::Graphics::CullMode::FRONT:
      return vk::CullModeFlagBits::eFront;
    case Dali::Graphics::CullMode::BACK:
      return vk::CullModeFlagBits::eBack;
    case Dali::Graphics::CullMode::FRONT_AND_BACK:
      return vk::CullModeFlagBits::eFrontAndBack;
  }
  return vk::CullModeFlagBits{};
}

constexpr vk::PolygonMode ConvertPolygonMode( Dali::Graphics::PolygonMode polygonMode )
{
  switch( polygonMode )
  {
    case Dali::Graphics::PolygonMode::FILL:
      return vk::PolygonMode::eFill;
    case Dali::Graphics::PolygonMode::LINE:
      return vk::PolygonMode::eLine;
    case Dali::Graphics::PolygonMode::POINT:
      return vk::PolygonMode::ePoint;
  }
  return vk::PolygonMode{};
}

constexpr vk::FrontFace ConvertFrontFace( Dali::Graphics::FrontFace frontFace )
{
  switch( frontFace )
  {
    case Dali::Graphics::FrontFace::CLOCKWISE:
      return vk::FrontFace::eClockwise;
    case Dali::Graphics::FrontFace::COUNTER_CLOCKWISE:
      return vk::FrontFace::eCounterClockwise;
  }
  return vk::FrontFace{};
}


constexpr vk::StencilOp ConvertStencilOp( Dali::Graphics::StencilOp stencilOp )
{
  switch( stencilOp )
  {
    case Dali::Graphics::StencilOp::KEEP: return vk::StencilOp::eKeep;
    case Dali::Graphics::StencilOp::ZERO: return vk::StencilOp::eZero;
    case Dali::Graphics::StencilOp::REPLACE: return vk::StencilOp::eReplace;
    case Dali::Graphics::StencilOp::INCREMENT_AND_CLAMP: return vk::StencilOp::eIncrementAndClamp;
    case Dali::Graphics::StencilOp::DECREMENT_AND_CLAMP: return vk::StencilOp::eDecrementAndClamp;
    case Dali::Graphics::StencilOp::INVERT: return vk::StencilOp::eInvert;
    case Dali::Graphics::StencilOp::INCREMENT_AND_WRAP: return vk::StencilOp::eIncrementAndWrap;
    case Dali::Graphics::StencilOp::DECREMENT_AND_WRAP: return vk::StencilOp::eDecrementAndWrap;
  }
  return {};
}

}

struct Pipeline::PipelineCreateInfo
{
  PipelineFactory::Info info;
};

struct Pipeline::VulkanPipelineState
{
  VulkanPipelineState() = default;

  ~VulkanPipelineState() = default;

  vk::PipelineColorBlendStateCreateInfo colorBlend;
  std::vector< vk::PipelineColorBlendAttachmentState > colorBlendAttachmentState;

  vk::PipelineDepthStencilStateCreateInfo depthStencil;
  vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
  vk::PipelineMultisampleStateCreateInfo multisample;
  vk::PipelineRasterizationStateCreateInfo rasterization;
  vk::PipelineTessellationStateCreateInfo tesselation;
  vk::PipelineVertexInputStateCreateInfo vertexInput;

  /**
   * Dynamic state configuration
   */
  struct DynamicState
  {
    vk::PipelineDynamicStateCreateInfo stateCreateInfo;
    std::vector<vk::DynamicState>      stateList;
  } dynamicState;

  /**
   * Viewport state configuration
   */
  struct ViewportState
  {
    vk::PipelineViewportStateCreateInfo createInfo;
    std::vector< vk::Viewport > viewports;
    std::vector< vk::Rect2D > scissors;
  } viewport;

  vk::Pipeline pipeline {};
  vk::PipelineLayout pipelineLayout {};

  vk::RenderPass renderpass {};
};

Pipeline::Pipeline( Vulkan::Graphics& graphics, Controller& controller, const PipelineFactory* factory )
: mGraphics( controller.GetGraphics() ),
  mController( controller )
{
  mHashCode = factory->GetHashCode();

  // copy info
  mCreateInfo = std::unique_ptr< PipelineCreateInfo >( new PipelineCreateInfo( { factory->GetCreateInfo() } ) );

  mPipelineCache = factory->mPipelineCache;
}

Pipeline::~Pipeline()
{
  if( mVulkanPipelineState )
  {
    auto device = mGraphics.GetDevice();
    auto descriptorSetLayouts = mVkDescriptorSetLayouts;
    auto allocator = &mGraphics.GetAllocator();
    auto pipelineLayout = mVulkanPipelineState->pipelineLayout;
    auto pipeline = mVulkanPipelineState->pipeline;

    mVulkanPipelineState = nullptr;

    // Discard unused descriptor set layouts
    mGraphics.DiscardResource( [ device, descriptorSetLayouts, pipeline, pipelineLayout, allocator ]() {

      // Destroy pipeline
      device.destroyPipeline( pipeline, allocator );

      // Destroy pipeline layout
      device.destroyPipelineLayout( pipelineLayout, allocator );

      // Destroy descriptor set layouts
      for( const auto& descriptorSetLayout : descriptorSetLayouts )
      {
        device.destroyDescriptorSetLayout( descriptorSetLayout, allocator );
      }

    } );
  }
}

uint32_t GetLocationIndex( const std::vector< Vulkan::SpirV::SPIRVVertexInputAttribute >& attribs, uint32_t location )
{
  auto retval = 0u;

  for( auto&& attr : attribs )
  {
    if( attr.location == location )
    {
      return retval;
    }
    retval++;
  }
  return -1u;
}

bool Pipeline::Initialise()
{
  if( !mVulkanPipelineState )
  {
    mVulkanPipelineState = std::unique_ptr< VulkanPipelineState >( new VulkanPipelineState() );
  }

  if( !mVulkanPipelineState->pipelineLayout )
  {
    mVulkanPipelineState->pipelineLayout = PreparePipelineLayout();
  }

  return bool(mVulkanPipelineState->pipelineLayout);
}


bool Pipeline::Compile()
{
  if( mVulkanPipelineState->pipeline )
  {
    return false;
  }
  // get shaders
  const auto& shader = static_cast<const VulkanAPI::Shader*>(mCreateInfo->info.shaderState.shaderProgram);
  auto vertexShader = shader->GetShader( vk::ShaderStageFlagBits::eVertex );
  auto fragmentShader = shader->GetShader( vk::ShaderStageFlagBits::eFragment );

  // retrieve input attributes descriptions
  std::vector< Vulkan::SpirV::SPIRVVertexInputAttribute > attribs{};
  vertexShader->GetSPIRVReflection()
              .GetVertexInputAttributes( attribs );

  std::vector< vk::VertexInputAttributeDescription > attributeDescriptions{};
  std::vector< vk::VertexInputBindingDescription > bindingDescriptions{};

  const auto& bufferBindings = mCreateInfo->info.vertexInputState.bufferBindings;
  const auto& attributes = mCreateInfo->info.vertexInputState.attributes;

  // vertex buffer bindings match order of buffers
  for( uint32_t bindingIndex = 0u; bindingIndex < bufferBindings.size(); ++bindingIndex )
  {
    bindingDescriptions.emplace_back( vk::VertexInputBindingDescription{}
                                              .setBinding( bindingIndex )
                                              .setInputRate(
                                                      bufferBindings[bindingIndex].inputRate ==
                                                      Dali::Graphics::VertexInputRate::PER_VERTEX
                                                      ? vk::VertexInputRate::eVertex
                                                      : vk::VertexInputRate::eInstance )
                                              .setStride( bufferBindings[bindingIndex].stride ) );
  }

  // create attribute descriptions
  if( !attribs.empty() && attribs.size() == attribs.size() )
  {
    for( auto&& vb : attributes )
    {
      attributeDescriptions.emplace_back( vk::VertexInputAttributeDescription{}
                                                  .setBinding( vb.binding )
                                                  .setFormat( attribs[GetLocationIndex( attribs, vb.location )].format )
                                                  .setLocation( vb.location )
                                                  .setOffset( vb.offset ) );
    }
  }
  else // incompatible pipeline
  {
    return false;
  }

  // prepare vertex input state
  auto vertexInputState = vk::PipelineVertexInputStateCreateInfo{}
          .setVertexBindingDescriptionCount( Vulkan::U32( bindingDescriptions.size() ) )
          .setPVertexBindingDescriptions( bindingDescriptions.data() )
          .setVertexAttributeDescriptionCount( Vulkan::U32( attributeDescriptions.size() ) )
          .setPVertexAttributeDescriptions( attributeDescriptions.data() );

  std::vector< vk::PipelineShaderStageCreateInfo > shaderStages =
          {
                  vk::PipelineShaderStageCreateInfo{}
                          .setModule( vertexShader->GetVkHandle() )
                          .setStage( vk::ShaderStageFlagBits::eVertex )
                          .setPName( "main" ),
                  vk::PipelineShaderStageCreateInfo{}
                          .setModule( fragmentShader->GetVkHandle() )
                          .setStage( vk::ShaderStageFlagBits::eFragment )
                          .setPName( "main" )
          };

  auto framebuffer = static_cast<const VulkanAPI::Framebuffer*>(mCreateInfo->info.framebufferState.framebuffer);
  auto swapchain = mGraphics.GetSwapchainForFBID( 0 );
  auto framebufferObject = framebuffer ? framebuffer->GetFramebufferRef() : swapchain->GetCurrentFramebuffer();

  vk::GraphicsPipelineCreateInfo pipelineInfo;
  pipelineInfo
          .setSubpass( 0 )
          .setRenderPass( framebufferObject->GetRenderPass() )
          .setBasePipelineHandle( nullptr )
          .setBasePipelineIndex( 0 )
          .setLayout( mVulkanPipelineState->pipelineLayout )
          .setPColorBlendState( PrepareColorBlendStateCreateInfo() )
          .setPDepthStencilState( PrepareDepthStencilStateCreateInfo() )
          .setPDynamicState( PrepareDynamicStateCreatInfo() )
          .setPInputAssemblyState( PrepareInputAssemblyStateCreateInfo() )
          .setPMultisampleState( PrepareMultisampleStateCreateInfo() )
          .setPRasterizationState( PrepareRasterizationStateCreateInfo() )
          .setPTessellationState( PrepareTesselationStateCreateInfo() )
          .setPVertexInputState( &vertexInputState )
          .setPViewportState( PrepareViewportStateCreateInfo() )
          .setPStages( shaderStages.data() )
          .setStageCount( Vulkan::U32( shaderStages.size() ) );

  mVulkanPipelineState->renderpass = pipelineInfo.renderPass;
  mVulkanPipelineState->pipeline = VkAssert( mGraphics.GetDevice().createGraphicsPipeline( mGraphics. GetVulkanPipelineCache(), pipelineInfo, mGraphics.GetAllocator( "PIPELINE" ) ) );

  return true;
}


const vk::PipelineColorBlendStateCreateInfo* Pipeline::PrepareColorBlendStateCreateInfo()
{
  const auto& factoryInfo = mCreateInfo->info;

  // blending enabled
  bool blendEnable = factoryInfo.colorBlendState.blendEnable;

  // color write mask
  vk::ColorComponentFlags colorWriteMask{};
  colorWriteMask |= ( factoryInfo.colorBlendState.colorComponentWriteBits & 1 ) ? vk::ColorComponentFlagBits::eR
                                                                                : vk::ColorComponentFlagBits{};
  colorWriteMask |= ( factoryInfo.colorBlendState.colorComponentWriteBits & 2 ) ? vk::ColorComponentFlagBits::eG
                                                                                : vk::ColorComponentFlagBits{};
  colorWriteMask |= ( factoryInfo.colorBlendState.colorComponentWriteBits & 4 ) ? vk::ColorComponentFlagBits::eB
                                                                                : vk::ColorComponentFlagBits{};
  colorWriteMask |= ( factoryInfo.colorBlendState.colorComponentWriteBits & 8 ) ? vk::ColorComponentFlagBits::eA
                                                                                : vk::ColorComponentFlagBits{};

  auto srcColorBlendFactor = ConvertBlendFactor( factoryInfo.colorBlendState.srcColorBlendFactor );
  auto dstColorBlendFactor = ConvertBlendFactor( factoryInfo.colorBlendState.dstColorBlendFactor );
  auto srcAlphaBlendFactor = ConvertBlendFactor( factoryInfo.colorBlendState.srcAlphaBlendFactor );
  auto dstAlphaBlendFactor = ConvertBlendFactor( factoryInfo.colorBlendState.dstAlphaBlendFactor );
  auto colorBlendOp = ConvertBlendOp( factoryInfo.colorBlendState.colorBlendOp );
  auto alphaBlendOp = ConvertBlendOp( factoryInfo.colorBlendState.alphaBlendOp );

  auto blendAttachmentState = vk::PipelineColorBlendAttachmentState{}
          .setBlendEnable( vk::Bool32( blendEnable ) )
          .setColorWriteMask( colorWriteMask )
          .setSrcColorBlendFactor( srcColorBlendFactor )
          .setDstColorBlendFactor( dstColorBlendFactor )
          .setSrcAlphaBlendFactor( srcAlphaBlendFactor )
          .setDstAlphaBlendFactor( dstAlphaBlendFactor )
          .setColorBlendOp( colorBlendOp )
          .setAlphaBlendOp( alphaBlendOp );

  mVulkanPipelineState->colorBlendAttachmentState.emplace_back( blendAttachmentState );

  mVulkanPipelineState->colorBlend
                      .setBlendConstants( { factoryInfo.colorBlendState.blendConstants[0],
                                            factoryInfo.colorBlendState.blendConstants[1],
                                            factoryInfo.colorBlendState.blendConstants[2],
                                            factoryInfo.colorBlendState.blendConstants[3] } )
                      .setAttachmentCount( 1 )
                      .setPAttachments( mVulkanPipelineState->colorBlendAttachmentState.data() );

  return &mVulkanPipelineState->colorBlend;
}

const vk::PipelineDepthStencilStateCreateInfo* Pipeline::PrepareDepthStencilStateCreateInfo()
{
  const auto& dsInfo = mCreateInfo->info.depthStencilState;
  auto retval = &( mVulkanPipelineState->depthStencil
                                .setDepthTestEnable( vk::Bool32( dsInfo.depthTestEnable ) )
                                .setDepthWriteEnable( vk::Bool32( dsInfo.depthWriteEnable ) )
                                .setDepthCompareOp( ConvertCompareOp( dsInfo.depthCompareOp ) )
                                .setDepthBoundsTestEnable( false )
                                .setStencilTestEnable( vk::Bool32( dsInfo.stencilTestEnable ) ) );

  if( dsInfo.stencilTestEnable )
  {
    auto frontOpState = vk::StencilOpState{}
      .setCompareOp( ConvertCompareOp( dsInfo.front.compareOp ) )
      .setCompareMask( dsInfo.front.compareMask )
      .setDepthFailOp( ConvertStencilOp( dsInfo.front.depthFailOp ) )
      .setFailOp( ConvertStencilOp( dsInfo.front.failOp ) )
      .setPassOp( ConvertStencilOp( dsInfo.front.passOp ) )
      .setReference( dsInfo.front.reference )
      .setWriteMask( dsInfo.front.writeMask );

    retval->setFront( frontOpState );
    retval->setBack( frontOpState );
  }

  return retval;
}

const vk::PipelineDynamicStateCreateInfo* Pipeline::PrepareDynamicStateCreatInfo()
{
  // if no mask set, disable dynamic states
  if( !mCreateInfo->info.dynamicStateMask )
  {
    return nullptr;
  }

  mVulkanPipelineState->dynamicState = {};

  const vk::DynamicState STATES[] =
  {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor,
    vk::DynamicState::eLineWidth,
    vk::DynamicState::eDepthBias,
    vk::DynamicState::eBlendConstants,
    vk::DynamicState::eDepthBounds,
    vk::DynamicState::eStencilCompareMask,
    vk::DynamicState::eStencilWriteMask,
    vk::DynamicState::eStencilReference
  };

  std::vector<vk::DynamicState> dynamicStates{};

  for( auto i = 0u; i < Dali::Graphics::PIPELINE_DYNAMIC_STATE_COUNT; ++i )
  {
    if(mCreateInfo->info.dynamicStateMask & (1u << i))
    {
      dynamicStates.push_back(STATES[i]);
    }
  }

  mVulkanPipelineState->dynamicState.stateList = std::move(dynamicStates);
  mVulkanPipelineState->dynamicState.stateCreateInfo.setDynamicStateCount( uint32_t(mVulkanPipelineState->dynamicState.stateList.size()) );
  mVulkanPipelineState->dynamicState.stateCreateInfo.setPDynamicStates( mVulkanPipelineState->dynamicState.stateList.data() );
  return &mVulkanPipelineState->dynamicState.stateCreateInfo;
}

const vk::PipelineInputAssemblyStateCreateInfo* Pipeline::PrepareInputAssemblyStateCreateInfo()
{
  const auto& iaInfo = mCreateInfo->info.inputAssemblyState;
  //@todo support topology and restart
  return &( mVulkanPipelineState->inputAssembly
                                .setPrimitiveRestartEnable( vk::Bool32( iaInfo.primitiveRestartEnable ) )
                                .setTopology( ConvertPrimitiveTopology( iaInfo.topology ) ) );
}

const vk::PipelineMultisampleStateCreateInfo* Pipeline::PrepareMultisampleStateCreateInfo()
{
  return &( mVulkanPipelineState->multisample
                                .setSampleShadingEnable( vk::Bool32( false ) )
                                .setRasterizationSamples( vk::SampleCountFlagBits::e1 )
                                .setAlphaToCoverageEnable( vk::Bool32( false ) )
                                .setMinSampleShading( 1.0f )
                                .setPSampleMask( nullptr ) );
}

const vk::PipelineRasterizationStateCreateInfo* Pipeline::PrepareRasterizationStateCreateInfo()
{
  const auto& rsInfo = mCreateInfo->info.rasterizationState;
  return &( mVulkanPipelineState->rasterization
                                .setCullMode( ConvertCullMode( rsInfo.cullMode ) )
                                .setDepthBiasClamp( 0.0f )
                                .setDepthBiasEnable( vk::Bool32( false ) )
                                .setDepthClampEnable( vk::Bool32( false ) )
                                .setFrontFace( ConvertFrontFace( rsInfo.frontFace ) )
                                .setPolygonMode( ConvertPolygonMode( rsInfo.polygonMode ) )
                                .setRasterizerDiscardEnable( vk::Bool32( false ) )
                                .setLineWidth( 1.0f ) );
}

const vk::PipelineViewportStateCreateInfo* Pipeline::PrepareViewportStateCreateInfo()
{
  const auto& vpInfo = mCreateInfo->info.viewportState;
  const auto& dynamicStateInfo = mCreateInfo->info.dynamicStateMask;

  // Use maximum viewport size in case we use dynamic viewport state
  auto width = dynamicStateInfo & Dali::Graphics::PipelineDynamicStateBits::VIEWPORT_BIT ?
               float( mGraphics.GetPhysicalDevice().getProperties().limits.maxFramebufferWidth ) : vpInfo.viewport.width;
  auto height = dynamicStateInfo & Dali::Graphics::PipelineDynamicStateBits::VIEWPORT_BIT ?
                float( mGraphics.GetPhysicalDevice().getProperties().limits.maxFramebufferHeight ) : vpInfo.viewport.height;

  // viewports
  mVulkanPipelineState->viewport.viewports.emplace_back( vpInfo.viewport.x, vpInfo.viewport.y, width, height );
  mVulkanPipelineState->viewport.viewports[0].setMinDepth( vpInfo.viewport.minDepth );
  mVulkanPipelineState->viewport.viewports[0].setMaxDepth( vpInfo.viewport.maxDepth );

  // scissors
  if( vpInfo.scissorTestEnable )
  {
    mVulkanPipelineState->viewport.scissors.emplace_back(vk::Rect2D({
                                                                      static_cast<int32_t>(vpInfo.scissor.x),
                                                                      static_cast<int32_t>(vpInfo.scissor.y)
                                                                    },
                                                                    {
                                                                      Vulkan::U32(vpInfo.scissor.width),
                                                                      Vulkan::U32(vpInfo.scissor.height)
                                                                    }));
  }
  else
  {
    mVulkanPipelineState->viewport.scissors.emplace_back(vk::Rect2D(
                          { static_cast<int32_t>(0), static_cast<int32_t>(0) },
                          { Vulkan::U32(width), Vulkan::U32(height) })
                        );
  }
  auto& viewState = mVulkanPipelineState->viewport;

  return &( mVulkanPipelineState->viewport.createInfo.
          setViewportCount( Vulkan::U32( viewState.viewports.size() ) ).
                                        setPViewports( viewState.viewports.data() ).
                                        setPScissors( viewState.scissors.data() ).
                                        setScissorCount( Vulkan::U32( viewState.scissors.size() ) ) );
}

const vk::PipelineTessellationStateCreateInfo* Pipeline::PrepareTesselationStateCreateInfo()
{
  return nullptr;
}

const vk::PipelineLayout Pipeline::PreparePipelineLayout()
{
  // descriptor set layout
  const auto& shader = static_cast<const VulkanAPI::Shader*>(mCreateInfo->info.shaderState.shaderProgram);
  auto vertexShader = shader->GetShader( vk::ShaderStageFlagBits::eVertex );
  auto fragmentShader = shader->GetShader( vk::ShaderStageFlagBits::eFragment );

  auto vshDsLayouts = vertexShader->GetSPIRVReflection()
                                  .GenerateDescriptorSetLayoutCreateInfo();
  auto fshDsLayouts = fragmentShader->GetSPIRVReflection()
                                    .GenerateDescriptorSetLayoutCreateInfo();

  decltype( vshDsLayouts ) layouts;
  layouts.resize( std::max( vshDsLayouts.size(), fshDsLayouts.size() ) );

  std::vector< vk::DescriptorSetLayout > dsLayouts;

  for( auto i = 0u; i < layouts.size(); ++i )
  {
    std::vector< vk::DescriptorSetLayoutBinding > dsBindings;

    // concatenate bindings per set
    if( !vshDsLayouts.empty() && vshDsLayouts[i].bindingCount )
    {
      dsBindings.insert(
              dsBindings.end(), vshDsLayouts[i].pBindings, vshDsLayouts[i].pBindings + vshDsLayouts[i].bindingCount );
    }
    if( !fshDsLayouts.empty() && fshDsLayouts[i].bindingCount )
    {
      dsBindings.insert(
              dsBindings.end(), fshDsLayouts[i].pBindings, fshDsLayouts[i].pBindings + fshDsLayouts[i].bindingCount );
    }

    GenerateDescriptorSetLayoutSignatures( dsBindings );

    layouts[i].pBindings = dsBindings.data();
    layouts[i].bindingCount = Vulkan::U32( dsBindings.size() );

    dsLayouts.emplace_back( VkAssert( mGraphics.GetDevice()
                                               .createDescriptorSetLayout( layouts[i], mGraphics.GetAllocator() ) ) );
  }

  mVkDescriptorSetLayouts = dsLayouts;

  // create layout
  auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo{}
    .setSetLayoutCount( Vulkan::U32( dsLayouts.size() ) )
    .setPSetLayouts( dsLayouts.data() )
    .setPPushConstantRanges( nullptr )
    .setPushConstantRangeCount( 0 );

  return VkAssert( mGraphics.GetDevice().createPipelineLayout( pipelineLayoutCreateInfo, mGraphics.GetAllocator("PIPELINELAYOUT") ) );
}

void Pipeline::GenerateDescriptorSetLayoutSignatures( const std::vector< vk::DescriptorSetLayoutBinding >& bindings )
{
  auto descriptorTypesAndCount = std::vector< std::tuple< uint32_t, Vulkan::DescriptorType > >{};

  std::for_each( bindings.cbegin(),
                 bindings.cend(),
                 [ & ]( const vk::DescriptorSetLayoutBinding binding ) {
                   auto value = binding.descriptorCount;
                   Vulkan::DescriptorType descriptorType;

                   switch( binding.descriptorType )
                   {
                     case vk::DescriptorType::eStorageImage:
                       descriptorType = Vulkan::DescriptorType::STORAGE_IMAGE;
                       break;
                     case vk::DescriptorType::eSampler:
                       descriptorType = Vulkan::DescriptorType::SAMPLER;
                       break;
                     case vk::DescriptorType::eSampledImage:
                       descriptorType = Vulkan::DescriptorType::SAMPLED_IMAGE;
                       break;
                     case vk::DescriptorType::eCombinedImageSampler:
                       descriptorType = Vulkan::DescriptorType::COMBINED_IMAGE_SAMPLER;
                       break;
                     case vk::DescriptorType::eUniformTexelBuffer:
                       descriptorType = Vulkan::DescriptorType::UNIFORM_TEXEL_BUFFER;
                       break;
                     case vk::DescriptorType::eStorageTexelBuffer:
                       descriptorType = Vulkan::DescriptorType::STORAGE_TEXEL_BUFFER;
                       break;
                     case vk::DescriptorType::eUniformBuffer:
                       descriptorType = Vulkan::DescriptorType::UNIFORM_BUFFER;
                       break;
                     case vk::DescriptorType::eStorageBuffer:
                       descriptorType = Vulkan::DescriptorType::STORAGE_BUFFER;
                       break;
                     case vk::DescriptorType::eUniformBufferDynamic:
                       descriptorType = Vulkan::DescriptorType::DYNAMIC_UNIFORM_BUFFER;
                       break;
                     case vk::DescriptorType::eStorageBufferDynamic:
                       descriptorType = Vulkan::DescriptorType::DYNAMIC_STORAGE_BUFFER;
                       break;
                     case vk::DescriptorType::eInputAttachment:
                       descriptorType = Vulkan::DescriptorType::INPUT_ATTACHMENT;
                       break;
                   }

                   auto found = std::find_if( descriptorTypesAndCount.begin(),
                                              descriptorTypesAndCount.end(),
                                              [ & ]( std::tuple< uint32_t, Vulkan::DescriptorType > tuple )
                                              {
                                                return descriptorType == std::get< Vulkan::DescriptorType >( tuple );
                                              } );

                   if( found == descriptorTypesAndCount.end() )
                   {
                     descriptorTypesAndCount.emplace_back( value, descriptorType );
                   }
                   else
                   {
                     auto& valueRef = std::get< uint32_t >( *found );
                     valueRef += value;
                   }
                 } );

  auto signature = Vulkan::DescriptorSetLayoutSignature{};
  std::for_each( descriptorTypesAndCount.begin(),
                 descriptorTypesAndCount.end(),
                 [ & ]( const std::tuple< uint32_t, Vulkan::DescriptorType > tuple )
                 {
                   signature.EncodeValue( std::get< uint32_t >( tuple ),
                                          std::get< Vulkan::DescriptorType >( tuple ) );
                 } );

  mDescriptorSetLayoutSignatures.emplace_back( signature );
}

void Pipeline::Reference()
{
  ++mRefCounter;
}

void Pipeline::Dereference()
{
  if( mRefCounter )
  {
    if( --mRefCounter == 0 )
    {
      Destroy();
    }
  }
}

void Pipeline::Destroy()
{
  if( mPipelineCache )
  {
    mPipelineCache->RemovePipeline( this );
  }
}

uint32_t Pipeline::GetHashCode() const
{
  return mHashCode;
}

const vk::Pipeline& Pipeline::GetVkPipeline() const
{
  return mVulkanPipelineState->pipeline;
}

const vk::PipelineLayout& Pipeline::GetVkPipelineLayout() const
{
  return mVulkanPipelineState->pipelineLayout;
}

const std::vector< vk::DescriptorSetLayout >& Pipeline::GetVkDescriptorSetLayouts() const
{
  return mVkDescriptorSetLayouts;
}

const std::vector< Vulkan::DescriptorSetLayoutSignature >& Pipeline::GetDescriptorSetLayoutSignatures() const
{
  return mDescriptorSetLayoutSignatures;
}

Dali::Graphics::PipelineDynamicStateMask Pipeline::GetDynamicStateMask() const
{
  return mCreateInfo->info.dynamicStateMask;
}

void Pipeline::Bind( Vulkan::RefCountedCommandBuffer& commandBuffer )
{
  commandBuffer->GetVkHandle().bindPipeline( vk::PipelineBindPoint::eGraphics, mVulkanPipelineState->pipeline );
}

bool Pipeline::HasDepthEnabled() const
{
  return mCreateInfo->info.depthStencilState.depthTestEnable || mCreateInfo->info.depthStencilState.depthWriteEnable;
}

bool Pipeline::HasStencilEnabled() const
{
  // @todo: work out proper condition
  return mCreateInfo->info.depthStencilState.stencilTestEnable;
}

} // Internal
} // VulkanAPI
} // Graphics
} // Dal
