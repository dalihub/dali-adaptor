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

#include <dali/graphics/vulkan/api/internal/vulkan-pipeline-cache-debug.h>
#include <dali/graphics/vulkan/api/internal/vulkan-api-pipeline-impl.h>
#include <dali/graphics/vulkan/api/vulkan-api-pipeline.h>
#include <dali/graphics-api/graphics-api-types-debug.h>
#include <sstream>

namespace Dali
{
namespace Graphics
{
namespace VulkanAPI
{
#if defined(DEBUG_ENABLED)

std::string PipelineCacheDebug::DebugPrintPipelineInfo( const PipelineFactory::Info& info ) const
{
  std::stringstream ss;
  // shader
  ss << "  Shader: " << info.shaderState.shaderProgram << std::endl;

  // vertex input state
  ss << "  VI    : " << std::endl;
  {
    // attributes
    int k = 0;
    for (auto &&i : info.vertexInputState.attributes)
    {
      ss << "    attribute[" << k++ << "]" << std::endl;
      ss << "       binding  = " << i.binding << std::endl;
      ss << "       offset   = " << i.offset << std::endl;
      ss << "       location = " << i.location << std::endl;
    }
    // bindings
    k = 0;
    for (auto &&i : info.vertexInputState.bufferBindings)
    {
      ss << "    binding[" << k++ << "]" << std::endl;
      ss << "       inputRate = " << Dali::Graphics::Debug::str(i.inputRate) << std::endl;
      ss << "       stride    = " << i.stride << std::endl;
    }
  }

  // vertex input state
  ss << "  IA    : " << std::endl;
  ss << "    primitiveRestartEnable: " << info.inputAssemblyState.primitiveRestartEnable << std::endl;
  ss << "    topology              : " << Dali::Graphics::Debug::str(info.inputAssemblyState.topology) << std::endl;

  // depth/stencil
  const auto& ds = info.depthStencilState;
  ss << "  DS    : " << std::endl;
  ss << "    depthTestEnable: " << ds.depthTestEnable << std::endl;
  ss << "    depthWriteEnable: " << ds.depthWriteEnable << std::endl;
  ss << "    depthCompareOp: " << Dali::Graphics::Debug::str(ds.depthCompareOp) << std::endl;
  ss << "    stencilTestEnable: " << ds.stencilTestEnable << std::endl;

  // rasterization state
  const auto& rs = info.rasterizationState;
  ss << "  RS    : " << std::endl;
  ss << "    polygonMode : " << Dali::Graphics::Debug::str(rs.polygonMode) << std::endl;
  ss << "    frontFace   : " << Dali::Graphics::Debug::str(rs.frontFace) << std::endl;
  ss << "    cullMode    : " << Dali::Graphics::Debug::str(rs.cullMode) << std::endl;

  const auto& cb = info.colorBlendState;
  ss << "  CB    : " << std::endl;
  ss << "    colorBlendOp        : " << Dali::Graphics::Debug::str(cb.colorBlendOp) << std::endl;
  ss << "    alphaBlendOp        : " << Dali::Graphics::Debug::str(cb.alphaBlendOp) << std::endl;
  ss << "    blendConstants      : " << cb.blendConstants[0] << ", " << cb.blendConstants[1] << ", " << cb.blendConstants[2] << ", " << cb.blendConstants[3] << std::endl;
  ss << "    srcColorBlendFactor : " << Dali::Graphics::Debug::str(cb.srcColorBlendFactor) << std::endl;
  ss << "    srcAlphaBlendFactor : " << Dali::Graphics::Debug::str(cb.srcAlphaBlendFactor) << std::endl;
  ss << "    dstColorBlendFactor : " << Dali::Graphics::Debug::str(cb.dstColorBlendFactor) << std::endl;
  ss << "    dstAlphaBlendFactor : " << Dali::Graphics::Debug::str(cb.dstAlphaBlendFactor) << std::endl;
  ss << "    blendEnable         : " << cb.blendEnable << std::endl;
  ss << "    logicOpEnable       : " << cb.logicOpEnable << std::endl;
  ss << "    logicOp             : " << "UNDEFINED" << std::endl;
  ss << "    colorComponentWriteBits : " << cb.colorComponentWriteBits << std::endl;

  const auto& vp = info.viewportState;
  ss << "  VP    : " << std::endl;
  ss << "    viewport        : { " << vp.viewport.x << ", "
     << vp.viewport.y << ", "
     << vp.viewport.width << ", "
     << vp.viewport.height << " } " << std::endl;
  return ss.str();
}

std::string PipelineCacheDebug::DebugPrintPipeline( const VulkanAPI::Pipeline& pipeline ) const
{
  std::stringstream ss;
  auto impl = pipeline.GetImplementation();

  // find pipeline in cache
  const PipelineFactory::Info* info { nullptr };
  for( auto&& i : mPipelineCache.mCacheMap )
  {
    for (auto&& j : i.second)
    {
      if( j.pipelineImpl.get() == impl )
      {
        info = j.info.get();
        break;
      }
    }
    if( info )
    {
      break;
    }
  }

  if(!info)
  {
    return "Invalid pipeline!";
  }
  ss << "Pipeline id = " << &pipeline << ":" << std::endl;
  ss << DebugPrintPipelineInfo(*info);
  return ss.str();
}

std::string PipelineCacheDebug::DebugPrint() const
{
  std::stringstream ss;
  for( auto&& cacheEntry : mPipelineCache.mCacheMap )
  {
    for( auto&& info: cacheEntry.second )
    {
      ss << "Pipeline id = " << info.pipelineImpl.get() << ":" << std::endl;
      ss << DebugPrintPipelineInfo( *info.info.get() );
    }
  }
  return ss.str();
}
#endif

} // namespace VulkanAPI
} // namespace Graphics
} // namespace Dali
