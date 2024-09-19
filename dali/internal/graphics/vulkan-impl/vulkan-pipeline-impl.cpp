/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline-impl.h>

// EXTERNAL INCLUDES
#include <memory>
#include <vulkan/vulkan.hpp>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/window-system/common/window-render-surface.h>

namespace Dali::Graphics::Vulkan
{
namespace
{
constexpr vk::CompareOp ConvCompareOp(const CompareOp in)
{
  switch(in)
  {
    case CompareOp::NEVER:
    {
      return vk::CompareOp::eNever;
    }
    case CompareOp::LESS:
    {
      return vk::CompareOp::eLess;
    }
    case CompareOp::EQUAL:
    {
      return vk::CompareOp::eEqual;
    }
    case CompareOp::LESS_OR_EQUAL:
    {
      return vk::CompareOp::eLessOrEqual;
    }
    case CompareOp::GREATER:
    {
      return vk::CompareOp::eGreater;
    }
    case CompareOp::NOT_EQUAL:
    {
      return vk::CompareOp::eNotEqual;
    }
    case CompareOp::GREATER_OR_EQUAL:
    {
      return vk::CompareOp::eGreaterOrEqual;
    }
    case CompareOp::ALWAYS:
    {
      return vk::CompareOp::eAlways;
    }
  }
  return vk::CompareOp{};
};

constexpr vk::StencilOp ConvStencilOp(const StencilOp in)
{
  switch(in)
  {
    case StencilOp::DECREMENT_AND_CLAMP:
    {
      return vk::StencilOp::eDecrementAndClamp;
    }
    case StencilOp::DECREMENT_AND_WRAP:
    {
      return vk::StencilOp::eDecrementAndWrap;
    }
    case StencilOp::INCREMENT_AND_CLAMP:
    {
      return vk::StencilOp::eIncrementAndClamp;
    }
    case StencilOp::INCREMENT_AND_WRAP:
    {
      return vk::StencilOp::eIncrementAndWrap;
    }
    case StencilOp::INVERT:
    {
      return vk::StencilOp::eInvert;
    }
    case StencilOp::KEEP:
    {
      return vk::StencilOp::eKeep;
    }
    case StencilOp::REPLACE:
    {
      return vk::StencilOp::eReplace;
    }
    case StencilOp::ZERO:
    {
      return vk::StencilOp::eZero;
    }
  }
  return vk::StencilOp{};
};

constexpr vk::StencilOpState ConvStencilOpState(const StencilOpState& in)
{
  vk::StencilOpState out;
  out.compareOp   = ConvCompareOp(in.compareOp);
  out.depthFailOp = ConvStencilOp(in.depthFailOp);
  out.compareMask = in.compareMask;
  out.failOp      = ConvStencilOp(in.failOp);
  out.passOp      = ConvStencilOp(in.passOp);
  out.reference   = in.reference;
  out.writeMask   = in.writeMask;
  return out;
};
} // namespace

/**
 * Copy of pipeline state, can be also used for internal caching
 */
struct PipelineImpl::PipelineState
{
  PipelineState()  = default;
  ~PipelineState() = default;

  // for maintaining correct lifecycle, the owned program
  // wrapper must be created
  UniquePtr<Program> program;

  ColorBlendState    colorBlendState;
  DepthStencilState  depthStencilState;
  ProgramState       programState;
  ViewportState      viewportState;
  RasterizationState rasterizationState;
  VertexInputState   vertexInputState;
  InputAssemblyState inputAssemblyState;
  RenderTarget*      renderTarget;
  PipelineCache*     pipelineCache{};
};

PipelineImpl::PipelineImpl(const Graphics::PipelineCreateInfo& createInfo, VulkanGraphicsController& controller, PipelineCache* pipelineCache)
: mController(controller)
{
  // the creation is deferred so it's needed to copy certain parts of the CreateInfo structure
  mPipelineState = std::make_unique<PipelineImpl::PipelineState>();

  // Make copies of structured pass by pointers and replace
  // stored create info structure fields
  CopyStateIfSet(createInfo.inputAssemblyState, mPipelineState->inputAssemblyState, &mCreateInfo.inputAssemblyState);
  CopyStateIfSet(createInfo.vertexInputState, mPipelineState->vertexInputState, &mCreateInfo.vertexInputState);
  CopyStateIfSet(createInfo.rasterizationState, mPipelineState->rasterizationState, &mCreateInfo.rasterizationState);
  CopyStateIfSet(createInfo.programState, mPipelineState->programState, &mCreateInfo.programState);
  CopyStateIfSet(createInfo.colorBlendState, mPipelineState->colorBlendState, &mCreateInfo.colorBlendState);
  CopyStateIfSet(createInfo.depthStencilState, mPipelineState->depthStencilState, &mCreateInfo.depthStencilState);
  CopyStateIfSet(createInfo.viewportState, mPipelineState->viewportState, &mCreateInfo.viewportState);
  mCreateInfo.renderTarget = createInfo.renderTarget;

  InitializePipeline();
}

const PipelineCreateInfo& PipelineImpl::GetCreateInfo() const
{
  return mCreateInfo;
}

VulkanGraphicsController& PipelineImpl::GetController() const
{
  return mController;
}

void PipelineImpl::Bind()
{
}

vk::Pipeline PipelineImpl::GetVkPipeline() const
{
  return mVkPipelines[0].pipeline;
}

void PipelineImpl::Retain()
{
  //++mRefCount;
}

void PipelineImpl::Release()
{
  //--mRefCount;
}

uint32_t PipelineImpl::GetRefCount() const
{
  return 0; //mRefCount;
}

PipelineImpl::~PipelineImpl() = default;

void PipelineImpl::InitializePipeline()
{
  auto                           vkDevice    = mController.GetGraphicsDevice().GetLogicalDevice();
  auto                           programImpl = static_cast<const Vulkan::Program*>(mCreateInfo.programState->program)->GetImplementation();
  auto&                          reflection  = programImpl->GetReflection();
  vk::GraphicsPipelineCreateInfo gfxPipelineInfo;
  gfxPipelineInfo.setLayout(reflection.GetVkPipelineLayout());
  gfxPipelineInfo.setStageCount(programImpl->GetVkPipelineShaderStageCreateInfoList().size());
  gfxPipelineInfo.setStages(programImpl->GetVkPipelineShaderStageCreateInfoList());
  gfxPipelineInfo.setBasePipelineHandle(nullptr);
  gfxPipelineInfo.setBasePipelineIndex(0);

  // 1. PipelineVertexInputStateCreateInfo
  vk::PipelineVertexInputStateCreateInfo visInfo;
  InitializeVertexInputState(visInfo);
  gfxPipelineInfo.setPVertexInputState(&visInfo);

  // 2. PipelineInputAssemblyStateCreateInfo
  vk::PipelineInputAssemblyStateCreateInfo iasInfo;
  InitializeInputAssemblyState(iasInfo);
  gfxPipelineInfo.setPInputAssemblyState(&iasInfo);

  // 3. PipelineTessellationStateCreateInfo - We don't support this one
  gfxPipelineInfo.setPTessellationState(nullptr);

  // 4. PipelineViewportStateCreateInfo
  vk::PipelineViewportStateCreateInfo viewInfo;
  InitializeViewportState(viewInfo);
  gfxPipelineInfo.setPViewportState(&viewInfo);

  // 5. PipelineRasterizationStateCreateInfo
  vk::PipelineRasterizationStateCreateInfo rsInfo;
  InitializeRasterizationState(rsInfo);
  gfxPipelineInfo.setPRasterizationState(&rsInfo);

  // 6. PipelineMultisampleStateCreateInfo
  vk::PipelineMultisampleStateCreateInfo msInfo;

  gfxPipelineInfo.setPMultisampleState(&msInfo);

  // 7. PipelineDepthStencilStateCreateInfo
  vk::PipelineDepthStencilStateCreateInfo dsInfo;
  gfxPipelineInfo.setPDepthStencilState(InitializeDepthStencilState(dsInfo) ? &dsInfo : nullptr);

  // 8. PipelineColorBlendStateCreateInfo
  vk::PipelineColorBlendStateCreateInfo bsInfo;
  InitializeColorBlendState(bsInfo);
  gfxPipelineInfo.setPColorBlendState(&bsInfo);

  // 9. PipelineDynamicStateCreateInfo
  vk::PipelineDynamicStateCreateInfo dynInfo;
  dynInfo.setDynamicStates(mDynamicStates);
  gfxPipelineInfo.setPDynamicState(&dynInfo);

  auto& allocator = mController.GetGraphicsDevice().GetAllocator();

  auto rtImpl = static_cast<Vulkan::RenderTarget*>(mCreateInfo.renderTarget);

  auto framebuffer = rtImpl->GetFramebuffer();
  auto surface     = rtImpl->GetSurface();

  FramebufferImpl* fbImpl = nullptr;
  if(surface)
  {
    auto& gfxDevice = mController.GetGraphicsDevice();
    auto  surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
    auto  swapchain = gfxDevice.GetSwapchainForSurfaceId(surfaceId);
    fbImpl          = swapchain->GetCurrentFramebuffer();
  }
  else if(framebuffer)
  {
    fbImpl = framebuffer->GetImpl();
  }

  auto renderPassCount = fbImpl->GetRenderPassCount();
  for(auto i = 0u; i < renderPassCount; ++i)
  {
    RenderPassImpl* impl       = fbImpl->GetRenderPass(i);
    gfxPipelineInfo.renderPass = impl->GetVkHandle();
    gfxPipelineInfo.subpass    = 0;

    if(gfxPipelineInfo.pColorBlendState)
    {
      auto attachmentCount = impl->GetAttachments().size();

      if(attachmentCount != mBlendStateAttachments.size())
      {
        // Make sure array is 1
        mBlendStateAttachments.resize(1);

        // make it the right size
        mBlendStateAttachments.resize(attachmentCount);

        // Fill with defaults
        std::fill(mBlendStateAttachments.begin() + 1, mBlendStateAttachments.end(), mBlendStateAttachments[0]);
        const_cast<vk::PipelineColorBlendStateCreateInfo*>(gfxPipelineInfo.pColorBlendState)->attachmentCount = attachmentCount;
        const_cast<vk::PipelineColorBlendStateCreateInfo*>(gfxPipelineInfo.pColorBlendState)->pAttachments    = mBlendStateAttachments.data();
      }
    }

    vk::Pipeline vkPipeline;
    VkAssert(vkDevice.createGraphicsPipelines(VK_NULL_HANDLE,
                                              1,
                                              &gfxPipelineInfo,
                                              &allocator,
                                              &vkPipeline));

    RenderPassPipelinePair item;
    item.renderPass = nullptr;
    item.pipeline   = vkPipeline;
    mVkPipelines.emplace_back(item);
  }
}

const Vulkan::Program* PipelineImpl::GetProgram() const
{
  return static_cast<const Vulkan::Program*>(mCreateInfo.programState->program);
}

void PipelineImpl::InitializeVertexInputState(vk::PipelineVertexInputStateCreateInfo& out)
{
  std::vector<vk::VertexInputBindingDescription> bindings;
  std::transform(mCreateInfo.vertexInputState->bufferBindings.begin(),
                 mCreateInfo.vertexInputState->bufferBindings.end(),
                 std::back_inserter(bindings),
                 [](const VertexInputState::Binding& in) -> vk::VertexInputBindingDescription {
                   vk::VertexInputBindingDescription out;
                   out.setInputRate((in.inputRate == VertexInputRate::PER_VERTEX ? vk::VertexInputRate::eVertex : vk::VertexInputRate::eInstance));
                   out.setBinding(0u); // To be filled later using indices
                   out.setStride(in.stride);
                   return out;
                 });

  // Assign bindings
  for(auto i = 0u; i < bindings.size(); ++i)
  {
    bindings[i].binding = i;
  }

  std::vector<vk::VertexInputAttributeDescription> attrs;
  std::transform(mCreateInfo.vertexInputState->attributes.begin(),
                 mCreateInfo.vertexInputState->attributes.end(),
                 std::back_inserter(attrs),
                 [](const VertexInputState::Attribute& in) -> vk::VertexInputAttributeDescription {
                   vk::VertexInputAttributeDescription out;
                   out.setBinding(in.binding);
                   out.setLocation(in.location);
                   out.setOffset(in.offset);
                   VertexInputFormat format = in.format;
                   switch(format)
                   {
                     case VertexInputFormat::FVECTOR2:
                     {
                       out.setFormat(vk::Format::eR32G32Sfloat);
                       break;
                     }
                     case VertexInputFormat::FVECTOR3:
                     {
                       out.setFormat(vk::Format::eR32G32B32Sfloat);
                       break;
                     }
                     case VertexInputFormat::FVECTOR4:
                     {
                       out.setFormat(vk::Format::eR32G32B32A32Sfloat);
                       break;
                     }
                     case VertexInputFormat::IVECTOR2:
                     {
                       out.setFormat(vk::Format::eR32G32Sint);
                       break;
                     }
                     case VertexInputFormat::IVECTOR3:
                     {
                       out.setFormat(vk::Format::eR32G32B32Sint);
                       break;
                     }
                     case VertexInputFormat::IVECTOR4:
                     {
                       out.setFormat(vk::Format::eR32G32B32A32Sint);
                       break;
                     }
                     case VertexInputFormat::FLOAT:
                     {
                       out.setFormat(vk::Format::eR32Sfloat);
                       break;
                     }
                     case VertexInputFormat::INTEGER:
                     {
                       out.setFormat(vk::Format::eR32Sint);
                       break;
                     }
                     case VertexInputFormat::UNDEFINED:
                     default:
                     {
                       out.setFormat(vk::Format::eUndefined);
                       DALI_LOG_ERROR("Vulkan vertex format undefined!\n");
                     }
                   };
                   return out;
                 });

  mVertexInputAttributeDescriptionList = attrs;
  mVertexInputBindingDescriptionList   = bindings;
  out.setVertexAttributeDescriptions(mVertexInputAttributeDescriptionList);
  out.setVertexBindingDescriptions(mVertexInputBindingDescriptionList);
}

void PipelineImpl::InitializeInputAssemblyState(vk::PipelineInputAssemblyStateCreateInfo& out) const
{
  auto gfxInputAssembly = mCreateInfo.inputAssemblyState;
  switch(gfxInputAssembly->topology)
  {
    case PrimitiveTopology::POINT_LIST:
    {
      out.setTopology(vk::PrimitiveTopology::ePointList);
      break;
    }
    case PrimitiveTopology::LINE_LIST:
    {
      out.setTopology(vk::PrimitiveTopology::eLineList);
      break;
    }
    case PrimitiveTopology::LINE_LOOP:
    {
      out.setTopology({});
      DALI_LOG_ERROR("LINE_LOOP topology isn't supported by Vulkan!\n");
      break;
    }
    case PrimitiveTopology::LINE_STRIP:
    {
      out.setTopology(vk::PrimitiveTopology::eLineStrip);
      break;
    }
    case PrimitiveTopology::TRIANGLE_LIST:
    {
      out.setTopology(vk::PrimitiveTopology::eTriangleList);
      break;
    }
    case PrimitiveTopology::TRIANGLE_STRIP:
    {
      out.setTopology(vk::PrimitiveTopology::eTriangleStrip);
      break;
    }
    case PrimitiveTopology::TRIANGLE_FAN:
    {
      out.setTopology(vk::PrimitiveTopology::eTriangleFan);
      break;
    }
    default:
    {
      DALI_LOG_ERROR("Unknown topology!\n");
    }
  }
  out.setPrimitiveRestartEnable(gfxInputAssembly->primitiveRestartEnable);
}

void PipelineImpl::InitializeViewportState(vk::PipelineViewportStateCreateInfo& out)
{
  auto gfxViewportState = mCreateInfo.viewportState;

  // if there is no gfx viewport state provided then we assume
  // it's going to be a dynamic state
  if(gfxViewportState)
  {
    // build viewport
    mViewport.x        = gfxViewportState->viewport.x;
    mViewport.y        = gfxViewportState->viewport.y;
    mViewport.width    = gfxViewportState->viewport.width;
    mViewport.height   = gfxViewportState->viewport.height;
    mViewport.minDepth = gfxViewportState->viewport.minDepth;
    mViewport.maxDepth = gfxViewportState->viewport.maxDepth;

    mScissor.offset = vk::Offset2D{gfxViewportState->scissor.x, gfxViewportState->scissor.y};
    mScissor.extent = vk::Extent2D{gfxViewportState->scissor.width, gfxViewportState->scissor.height};

    out.setViewportCount(1);
    out.setPViewports(&mViewport);
    out.setScissorCount(1);
    out.setPScissors(&mScissor);
  }
  else
  {
    out.setViewportCount(1);
    out.setScissorCount(1);
    // enable dynamic state, otherwise it's an error
    mDynamicStates.emplace_back(vk::DynamicState::eViewport);
    mDynamicStates.emplace_back(vk::DynamicState::eScissor);
  }
}

void PipelineImpl::InitializeMultisampleState(vk::PipelineMultisampleStateCreateInfo& out)
{
  out = vk::PipelineMultisampleStateCreateInfo{}; // TODO: decide what to set when we start rendering something
}

void PipelineImpl::InitializeRasterizationState(vk::PipelineRasterizationStateCreateInfo& out) const
{
  auto gfxRastState = mCreateInfo.rasterizationState;

  out.setFrontFace([gfxRastState]() {
    return gfxRastState->frontFace == FrontFace::CLOCKWISE ? vk::FrontFace::eClockwise : vk::FrontFace::eCounterClockwise;
  }());

  out.setPolygonMode([polygonMode = gfxRastState->polygonMode]() {
    switch(polygonMode)
    {
      case PolygonMode::FILL:
      {
        return vk::PolygonMode::eFill;
      }
      case PolygonMode::LINE:
      {
        return vk::PolygonMode::eLine;
      }
      case PolygonMode::POINT:
      {
        return vk::PolygonMode::ePoint;
      }
    }
    return vk::PolygonMode{};
  }());

  out.setCullMode([cullMode = gfxRastState->cullMode]() -> vk::CullModeFlagBits {
    switch(cullMode)
    {
      case CullMode::NONE:
      {
        return vk::CullModeFlagBits::eNone;
      }
      case CullMode::BACK:
      {
        return vk::CullModeFlagBits::eBack;
      }
      case CullMode::FRONT:
      {
        return vk::CullModeFlagBits::eFront;
      }
      case CullMode::FRONT_AND_BACK:
      {
        return vk::CullModeFlagBits::eFrontAndBack;
      }
    }
    return {};
  }());

  out.setLineWidth(1.0f);         // Line with hardcoded to 1.0f
  out.setDepthClampEnable(false); // no depth clamp
}

bool PipelineImpl::InitializeDepthStencilState(vk::PipelineDepthStencilStateCreateInfo& out)
{
  auto& in = mCreateInfo.depthStencilState;

  if(in)
  {
    out.setBack(ConvStencilOpState(in->back));
    out.setFront(ConvStencilOpState(in->front));
    out.setDepthTestEnable(in->depthTestEnable);
    out.setDepthWriteEnable(in->depthWriteEnable);
    out.setDepthBoundsTestEnable(false);
    out.setMinDepthBounds({});
    out.setMaxDepthBounds({});
    out.setStencilTestEnable(in->stencilTestEnable);
    out.setDepthCompareOp(ConvCompareOp(in->depthCompareOp));
    return true;
  }
  return false;
}

void PipelineImpl::InitializeColorBlendState(vk::PipelineColorBlendStateCreateInfo& out)
{
  auto in = mCreateInfo.colorBlendState;

  auto ConvLogicOp = [](LogicOp in) {
    switch(in)
    {
      case LogicOp::CLEAR:
      {
        return vk::LogicOp::eClear;
      }
      case LogicOp::AND:
      {
        return vk::LogicOp::eAnd;
      }
      case LogicOp::AND_REVERSE:
      {
        return vk::LogicOp::eAndReverse;
      }
      case LogicOp::COPY:
      {
        return vk::LogicOp::eCopy;
      }
      case LogicOp::AND_INVERTED:
      {
        return vk::LogicOp::eAndInverted;
      }
      case LogicOp::NO_OP:
      {
        return vk::LogicOp::eNoOp;
      }
      case LogicOp::XOR:
      {
        return vk::LogicOp::eXor;
      }
      case LogicOp::OR:
      {
        return vk::LogicOp::eOr;
      }
      case LogicOp::NOR:
      {
        return vk::LogicOp::eNor;
      }
      case LogicOp::EQUIVALENT:
      {
        return vk::LogicOp::eEquivalent;
      }
      case LogicOp::INVERT:
      {
        return vk::LogicOp::eInvert;
      }
      case LogicOp::OR_REVERSE:
      {
        return vk::LogicOp::eOrReverse;
      }
      case LogicOp::COPY_INVERTED:
      {
        return vk::LogicOp::eCopyInverted;
      }
      case LogicOp::OR_INVERTED:
      {
        return vk::LogicOp::eOrInverted;
      }
      case LogicOp::NAND:
      {
        return vk::LogicOp::eNand;
      }
      case LogicOp::SET:
      {
        return vk::LogicOp::eSet;
      }
    }
    return vk::LogicOp{};
  };

  auto ConvBlendOp = [](BlendOp in) {
    switch(in)
    {
      case BlendOp::ADD:
      {
        return vk::BlendOp::eAdd;
      }
      case BlendOp::SUBTRACT:
      {
        return vk::BlendOp::eSubtract;
      }
      case BlendOp::REVERSE_SUBTRACT:
      {
        return vk::BlendOp::eReverseSubtract;
      }
      case BlendOp::MIN:
      {
        return vk::BlendOp::eMin;
      }
      case BlendOp::MAX:
      {
        return vk::BlendOp::eMax;
      }
      case BlendOp::MULTIPLY:
      {
        return vk::BlendOp::eMultiplyEXT;
      }
      case BlendOp::SCREEN:
      {
        return vk::BlendOp::eScreenEXT;
      }
      case BlendOp::OVERLAY:
      {
        return vk::BlendOp::eOverlayEXT;
      }
      case BlendOp::DARKEN:
      {
        return vk::BlendOp::eDarkenEXT;
      }
      case BlendOp::LIGHTEN:
      {
        return vk::BlendOp::eLightenEXT;
      }
      case BlendOp::COLOR_DODGE:
      {
        return vk::BlendOp::eColordodgeEXT;
      }
      case BlendOp::COLOR_BURN:
      {
        return vk::BlendOp::eColorburnEXT;
      }
      case BlendOp::HARD_LIGHT:
      {
        return vk::BlendOp::eHardlightEXT;
      }
      case BlendOp::SOFT_LIGHT:
      {
        return vk::BlendOp::eSoftlightEXT;
      }
      case BlendOp::DIFFERENCE:
      {
        return vk::BlendOp::eDifferenceEXT;
      }
      case BlendOp::EXCLUSION:
      {
        return vk::BlendOp::eExclusionEXT;
      }
      case BlendOp::HUE:
      {
        return vk::BlendOp::eHslHueEXT;
      }
      case BlendOp::SATURATION:
      {
        return vk::BlendOp::eHslSaturationEXT;
      }
      case BlendOp::COLOR:
      {
        return vk::BlendOp::eHslColorEXT;
      }
      case BlendOp::LUMINOSITY:
      {
        return vk::BlendOp::eHslLuminosityEXT;
      }
    }
    return vk::BlendOp{};
  };

  auto ConvBlendFactor = [](BlendFactor in) {
    switch(in)
    {
      case BlendFactor::ZERO:
      {
        return vk::BlendFactor::eZero;
      }
      case BlendFactor::ONE:
      {
        return vk::BlendFactor::eOne;
      }
      case BlendFactor::SRC_COLOR:
      {
        return vk::BlendFactor::eSrcColor;
      }
      case BlendFactor::ONE_MINUS_SRC_COLOR:
      {
        return vk::BlendFactor::eOneMinusSrcColor;
      }
      case BlendFactor::DST_COLOR:
      {
        return vk::BlendFactor::eDstColor;
      }
      case BlendFactor::ONE_MINUS_DST_COLOR:
      {
        return vk::BlendFactor::eOneMinusDstColor;
      }
      case BlendFactor::SRC_ALPHA:
      {
        return vk::BlendFactor::eSrcAlpha;
      }
      case BlendFactor::ONE_MINUS_SRC_ALPHA:
      {
        return vk::BlendFactor::eOneMinusSrcAlpha;
      }
      case BlendFactor::DST_ALPHA:
      {
        return vk::BlendFactor::eDstAlpha;
      }
      case BlendFactor::ONE_MINUS_DST_ALPHA:
      {
        return vk::BlendFactor::eOneMinusDstAlpha;
      }
      case BlendFactor::CONSTANT_COLOR:
      {
        return vk::BlendFactor::eConstantColor;
      }
      case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
      {
        return vk::BlendFactor::eOneMinusConstantColor;
      }
      case BlendFactor::CONSTANT_ALPHA:
      {
        return vk::BlendFactor::eConstantAlpha;
      }
      case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
      {
        return vk::BlendFactor::eOneMinusConstantAlpha;
      }
      case BlendFactor::SRC_ALPHA_SATURATE:
      {
        return vk::BlendFactor::eSrcAlphaSaturate;
      }
      case BlendFactor::SRC1_COLOR:
      {
        return vk::BlendFactor::eSrc1Color;
      }
      case BlendFactor::ONE_MINUS_SRC1_COLOR:
      {
        return vk::BlendFactor::eOneMinusSrc1Color;
      }
      case BlendFactor::SRC1_ALPHA:
      {
        return vk::BlendFactor::eSrc1Alpha;
      }
      case BlendFactor::ONE_MINUS_SRC1_ALPHA:
      {
        return vk::BlendFactor::eOneMinusSrc1Alpha;
      }
    }
    return vk::BlendFactor{};
  };

  out.setLogicOpEnable(in->logicOpEnable);
  out.setLogicOp(ConvLogicOp(in->logicOp));

  // We don't know how many attachments we will blend but gfx api assumes single attachment
  mBlendStateAttachments.clear();
  mBlendStateAttachments.emplace_back();
  auto& att = mBlendStateAttachments.back();

  att.setAlphaBlendOp(ConvBlendOp(in->alphaBlendOp));
  att.setBlendEnable(in->blendEnable);
  //att.setColorWriteMask()
  att.setColorBlendOp(ConvBlendOp(in->colorBlendOp));
  att.setColorWriteMask(vk::ColorComponentFlags(in->colorComponentWriteBits));
  att.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
  att.setDstAlphaBlendFactor(ConvBlendFactor(in->dstAlphaBlendFactor));
  att.setDstColorBlendFactor(ConvBlendFactor(in->dstColorBlendFactor));
  att.setSrcAlphaBlendFactor(ConvBlendFactor(in->srcAlphaBlendFactor));
  att.setSrcColorBlendFactor(ConvBlendFactor(in->srcColorBlendFactor));
  out.setAttachments(mBlendStateAttachments);

  std::copy(in->blendConstants, in->blendConstants + 4, out.blendConstants.begin());
}

} // namespace Dali::Graphics::Vulkan
