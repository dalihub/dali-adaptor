/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
 */

#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/graphics/gles-impl/egl-graphics-controller-debug.h>
#include <dali/internal/graphics/gles-impl/gles-graphics-render-target.h>

#include <unistd.h>
#include <cstdio>
#include <filesystem>
#include <queue>

namespace Dali::Graphics
{
#if defined(DEBUG_ENABLED)
const char* GRAPHICS_CMDBUF_OUTFILE_ENV = "GRAPHICS_CMDBUF_OUTFILE";
const char* GRAPHICS_DUMP_TRIGGER_FILE  = "/tmp/dump_cmd_buf";

std::string DumpCompareOp(Graphics::CompareOp compareOp)
{
  switch(compareOp)
  {
    case Graphics::CompareOp::NEVER:
      return "Graphics::CompareOp::NEVER";
      break;
    case Graphics::CompareOp::LESS:
      return "Graphics::CompareOp::LESS";
      break;
    case Graphics::CompareOp::EQUAL:
      return "Graphics::CompareOp::EQUAL";
      break;
    case Graphics::CompareOp::LESS_OR_EQUAL:
      return "Graphics::CompareOp::LESS_OR_EQUAL";
      break;
    case Graphics::CompareOp::GREATER:
      return "Graphics::CompareOp::GREATER";
      break;
    case Graphics::CompareOp::NOT_EQUAL:
      return "Graphics::CompareOp::NOT_EQUAL";
      break;
    case Graphics::CompareOp::GREATER_OR_EQUAL:
      return "Graphics::CompareOp::GREATER_OR_EQUAL";
      break;
    case Graphics::CompareOp::ALWAYS:
      return "Graphics::CompareOp::ALWAYS";
      break;
  }
  return "UNKNOWN";
}

std::string DumpStencilOp(Graphics::StencilOp stencilOp)
{
  switch(stencilOp)
  {
    case Graphics::StencilOp::KEEP:
      return "Graphics::StencilOp::KEEP";
      break;
    case Graphics::StencilOp::ZERO:
      return "Graphics::StencilOp::ZERO";
      break;
    case Graphics::StencilOp::REPLACE:
      return "Graphics::StencilOp::REPLACE";
      break;
    case Graphics::StencilOp::INCREMENT_AND_CLAMP:
      return "Graphics::StencilOp::INCREMENT_AND_CLAMP";
      break;
    case Graphics::StencilOp::DECREMENT_AND_CLAMP:
      return "Graphics::StencilOp::DECREMENT_AND_CLAMP";
      break;
    case Graphics::StencilOp::INVERT:
      return "Graphics::StencilOp::INVERT";
      break;
    case Graphics::StencilOp::INCREMENT_AND_WRAP:
      return "Graphics::StencilOp::INCREMENT_AND_WRAP";
      break;
    case Graphics::StencilOp::DECREMENT_AND_WRAP:
      return "Graphics::StencilOp::DECREMENT_AND_WRAP";
      break;
  }
  return "UNKNOWN";
}

void DumpCommandBuffer(GraphicsFrameDump& frameDump, const GLES::CommandBuffer* commandBuffer)
{
  bool     first{true};
  uint32_t count  = 0u;
  FILE*    output = frameDump.output;

  fprintf(output, "{ \"level\":\"%s\",\"cmds\":[\n", commandBuffer->GetCreateInfo().level == CommandBufferLevel::PRIMARY ? "PRIMARY" : "SECONDARY");

  const auto command = commandBuffer->GetCommands(count);
  for(auto i = 0u; i < count; ++i)
  {
    auto& cmd = command[i];
    if(!first)
    {
      fprintf(output, ",");
    }
    first = false;
    // process command
    switch(cmd.type)
    {
      case GLES::CommandType::FLUSH:
      {
        fprintf(output, "{\"Cmd\":\"FLUSH\"}\n");
        break;
      }
      case GLES::CommandType::BIND_TEXTURES:
      {
        fprintf(output, "{\"Cmd\":\"BIND_TEXTURES\"}\n");
        break;
      }
      case GLES::CommandType::BIND_VERTEX_BUFFERS:
      {
        fprintf(output, "{\"Cmd\":\"BIND_VERTEX_BUFFERS\"}\n");
        break;
      }
      case GLES::CommandType::BIND_UNIFORM_BUFFER:
      {
        fprintf(output, "{\"Cmd\":\"BIND_UNIFORM_BUFFERS\"}\n");
        break;
      }
      case GLES::CommandType::BIND_INDEX_BUFFER:
      {
        fprintf(output, "{\"Cmd\":\"BIND_INDEX_BUFFERS\"}\n");
        break;
      }
      case GLES::CommandType::BIND_SAMPLERS:
      {
        fprintf(output, "{\"Cmd\":\"BIND_SAMPLERS\"}\n");
        break;
      }
      case GLES::CommandType::BIND_PIPELINE:
      {
        fprintf(output, "{\"Cmd\":\"BIND_PIPELINE\"}\n");
        break;
      }
      case GLES::CommandType::DRAW:
      {
        fprintf(output,
                "{\"Cmd\":\"DRAW\",\n"
                " \"vertexCount\":%d,\n"
                " \"instanceCount\":%d,\n"
                " \"firstInstance\":%d,\n"
                " \"firstVertex\":%d}\n",
                cmd.draw.draw.vertexCount,
                cmd.draw.draw.instanceCount,
                cmd.draw.draw.firstInstance,
                cmd.draw.firstOffset);
        break;
      }
      case GLES::CommandType::DRAW_INDEXED:
      {
        fprintf(output,
                "{\"Cmd\":\"DRAW_INDEXED\",\n"
                " \"indexCount\":%d,\n"
                " \"vertexOffset\":%d,\n"
                " \"firstIndex\":%d,\n"
                " \"instanceCount\":%d}\n",
                cmd.draw.drawIndexed.indexCount,
                cmd.draw.drawIndexed.vertexOffset,
                cmd.draw.firstOffset,
                cmd.draw.drawIndexed.instanceCount);
        break;
      }
      case GLES::CommandType::DRAW_NATIVE:
      {
        fprintf(output, "{\"Cmd\":\"DRAW_NATIVE\"}\n");
        break;
      }
      case GLES::CommandType::DRAW_INDEXED_INDIRECT:
      {
        fprintf(output,
                "{\"Cmd\":\"DRAW_INDEXED_INDIRECT\",\n"
                " \"offset\":%d,\n"
                " \"drawCount\":%d,\n"
                " \"stride\":%d}\n",
                cmd.draw.firstOffset,
                cmd.draw.drawIndexedIndirect.drawCount,
                cmd.draw.drawIndexedIndirect.stride);
        break;
      }
      case GLES::CommandType::SET_SCISSOR: // @todo Consider correcting for orientation here?
      {
        fprintf(output, "{\"Cmd\":\"SET_SCISSOR\",\n\"region\":[%d,%d,%d,%d]\n}\n", cmd.scissor.region.x, cmd.scissor.region.y, cmd.scissor.region.width, cmd.scissor.region.height);
        break;
      }
      case GLES::CommandType::SET_SCISSOR_TEST:
      {
        fprintf(output, "{\"Cmd\":\"SET_SCISSOR_TEST\",\n\"enable\":%s\n}\n", (cmd.scissorTest.enable ? "\"true\"" : "\"false\""));
        break;
      }
      case GLES::CommandType::SET_VIEWPORT: // @todo Consider correcting for orientation here?
      {
        fprintf(output, "{\"Cmd\":\"SET_VIEWPORT\",\n\"region\":[%f,%f,%f,%f]\n}\n", cmd.viewport.region.x, cmd.viewport.region.y, cmd.viewport.region.width, cmd.viewport.region.height);
        break;
      }
      case GLES::CommandType::SET_COLOR_MASK:
      {
        fprintf(output, "{\"Cmd\":\"SET_COLOR_MASK\",\n\"enable\":%s\n}\n", (cmd.colorMask.enabled ? "\"true\"" : "\"false\""));
        break;
      }
      case GLES::CommandType::CLEAR_STENCIL_BUFFER:
      {
        fprintf(output, "{\"Cmd\":\"CLEAR_STENCIL_BUFFER\"}\n");
        break;
      }
      case GLES::CommandType::CLEAR_DEPTH_BUFFER:
      {
        fprintf(output, "{\"Cmd\":\"CLEAR_DEPTH_BUFFER\"}\n");
        break;
      }

      case GLES::CommandType::SET_STENCIL_TEST_ENABLE:
      {
        fprintf(output, "{\"Cmd\":\"SET_STENCIL_TEST_ENABLE\",\n\"enable\":%s\n}\n", (cmd.stencilTest.enabled ? "\"true\"" : "\"false\""));
        break;
      }

      case GLES::CommandType::SET_STENCIL_STATE:
      {
        fprintf(output,
                "{\"Cmd\":\"STENCIL_STATE\",\n"
                "\"compareOp\":\"%s\",\n"
                "\"reference\":\"0x%x\",\n"
                "\"compareMask\":\"0x%x\"\n"
                "\"failOp\":\"%s\",\n"
                "\"depthFailOp\":\"%s\",\n"
                "\"passOp\":\"%s\"\n}",
                DumpCompareOp(cmd.stencilState.compareOp).c_str(),
                cmd.stencilState.reference,
                cmd.stencilState.compareMask,
                DumpStencilOp(cmd.stencilState.failOp).c_str(),
                DumpStencilOp(cmd.stencilState.depthFailOp).c_str(),
                DumpStencilOp(cmd.stencilState.passOp).c_str());
        break;
      }

      case GLES::CommandType::SET_STENCIL_WRITE_MASK:
      {
        fprintf(output, "{\"Cmd\":\"SET_STENCIL_WRITE_MASK\",\n\"mask\":%d\n}\n", cmd.stencilWriteMask.mask);
        break;
      }

      case GLES::CommandType::SET_DEPTH_COMPARE_OP:
      {
        fprintf(output,
                "{\"Cmd\":\"SET_DEPTH_COMPARE_OP\",\n"
                "\"compareOp\":\"%s\"\n}\n",
                DumpCompareOp(cmd.depth.compareOp).c_str());
        break;
      }
      case GLES::CommandType::SET_DEPTH_TEST_ENABLE:
      {
        fprintf(output, "{\"Cmd\":\"SET_DEPTH_TEST_ENABLE\",\n\"enable\":%s\n}\n", (cmd.depth.testEnabled ? "\"true\"" : "\"false\""));
        break;
      }
      case GLES::CommandType::SET_DEPTH_WRITE_ENABLE:
      {
        fprintf(output, "{\"Cmd\":\"SET_DEPTH_WRITE_ENABLE\",\n\"enable\":%s\n}\n", (cmd.depth.writeEnabled ? "\"true\"" : "\"false\""));
        break;
      }

      case GLES::CommandType::BEGIN_RENDERPASS:
      {
        const GLES::BeginRenderPassDescriptor& descriptor = *(cmd.beginRenderPass.descriptor.Ptr());
        fprintf(output,
                "{\"Cmd\":\"BEGIN_RENDER_PASS\",\n"
                "\"renderTarget\":\"%p\",\n"
                "\"renderPass\":\"%p\",\n"
                "\"renderArea\":[%d,%d,%d,%d],\n",
                descriptor.renderTarget,
                descriptor.renderPass,
                descriptor.renderArea.x,
                descriptor.renderArea.y,
                descriptor.renderArea.width,
                descriptor.renderArea.height);
        fprintf(output, "\"clearValues\":[");
        bool firstV = true;
        for(auto i = 0u; i < descriptor.clearValuesCount; ++i)
        {
          auto value = descriptor.clearValues.Ptr()[i];
          if(!firstV)
          {
            fprintf(output, ",");
          }
          firstV = false;
          fprintf(output, "[%f,%f,%f,%f]", value.color.r, value.color.g, value.color.b, value.color.a);
        }
        fprintf(output, "]\n}");

        frameDump.renderTargets.insert(descriptor.renderTarget);
        break;
      }
      case GLES::CommandType::END_RENDERPASS:
      {
        fprintf(output, "{\"Cmd\":\"END_RENDER_PASS\"}\n");
        break;
      }
      case GLES::CommandType::READ_PIXELS:
      {
        fprintf(output, "{\"Cmd\":\"READ_PIXELS\"}\n");
        break;
      }
      case GLES::CommandType::PRESENT_RENDER_TARGET:
      {
        fprintf(output,
                "{\"Cmd\":\"PRESENT_RENDER_TARGET\",\n"
                "\"targetToPresent\":\"%p\"\n}\n",
                cmd.presentRenderTarget.targetToPresent);
        frameDump.renderTargets.insert(cmd.presentRenderTarget.targetToPresent);
        break;
      }
      case GLES::CommandType::EXECUTE_COMMAND_BUFFERS:
      {
        fprintf(output, "{\"Cmd\":\"EXECUTE_COMMAND_BUFFERS\",\n\"buffers\":[");
        bool firstBuf{true};
        for(auto i = 0u; i < cmd.executeCommandBuffers.buffersCount; ++i)
        {
          const auto buf = cmd.executeCommandBuffers.buffers.Ptr()[i];
          if(!firstBuf)
          {
            fprintf(output, ", ");
          }
          firstBuf = false;
          DumpCommandBuffer(frameDump, buf);
        }
        fprintf(output, "]\n}");
        break;
      }
    }
  }
  fprintf(output, "]}\n");
}

int CloseJson(FILE* fp)
{
  fflush(fp);
  fclose(fp);
  return 0;
}

GraphicsFrameDump::GraphicsFrameDump()
: outputStream(nullptr, nullptr)
{
}

void GraphicsFrameDump::Start()
{
  if(IsDumpFrame())
  {
    if(!output && !outputStream)
    {
      const char* outfile = Dali::EnvironmentVariable::GetEnvironmentVariable(GRAPHICS_CMDBUF_OUTFILE_ENV);
      if(outfile)
      {
        char* filename;
        int   numChars = asprintf(&filename, "%s.%03d.json", outfile, fileCount);
        if(numChars > 0)
        {
          outputStream = UniqueFilePtr(std::fopen(filename, "w"), CloseJson);
          output       = outputStream.get();
          free(filename);
        }
      }
      if(!output)
      {
        output = stderr;
      }
    }

    if(firstFrame)
    {
      fprintf(output, "{\"CommandQueueSubmission\":[\n");
    }
    else
    {
      fprintf(output, ", \n");
    }
    firstFrame   = false;
    firstBuffer  = true;
    dumpingFrame = true;
    fprintf(output, "{\"Queue #%d\":[\n", frameCount);
  }
}

void GraphicsFrameDump::DumpCommandBuffer(const GLES::CommandBuffer* cmdBuf)
{
  if(dumpingFrame)
  {
    if(!firstBuffer)
    {
      fprintf(output, ", \n");
    }
    firstBuffer = false;
    Graphics::DumpCommandBuffer(*this, cmdBuf);
  }
}

void GraphicsFrameDump::DumpRenderTargets()
{
  if(!renderTargets.empty())
  {
    fprintf(output, ",\"RenderTargets\":[");
    bool first = true;
    for(auto renderTarget : renderTargets)
    {
      if(!first)
      {
        fprintf(output, ",");
      }
      first = false;

      fprintf(output,
              "{\n\"ptr\":\"%p\",\n"
              "   \"surface\":\"%p\",\n"
              "   \"framebuffer\":\"%p\",\n"
              "   \"extent\":[%d, %d],\n"
              "   \"preTransform\":\"%x\"\n}",
              renderTarget,
              renderTarget->GetCreateInfo().surface,
              renderTarget->GetCreateInfo().framebuffer,
              renderTarget->GetCreateInfo().extent.width,
              renderTarget->GetCreateInfo().extent.height,
              renderTarget->GetCreateInfo().preTransform);
    }
    fprintf(output, "]\n");
  }
}

void GraphicsFrameDump::End()
{
  if(dumpingFrame)
  {
    fprintf(output, "]}\n");
  }
  dumpingFrame = false;
}

bool GraphicsFrameDump::IsDumpFrame()
{
  bool dump = false;

  frameCount++;

  if(frameCount < NTH_FRAME)
  {
    dump = true;
  }
  else if(frameCount == NTH_FRAME)
  {
    fprintf(output, "\n]\n");
    DumpRenderTargets();
    fprintf(output, "\n}\n");
    fflush(output);
    outputStream.reset();
    output = nullptr;
    renderTargets.clear();
  }
  else if(std::filesystem::exists(GRAPHICS_DUMP_TRIGGER_FILE))
  {
    fileCount++;
    frameCount = 0;
    firstFrame = true;
    dump       = true;
    unlink(GRAPHICS_DUMP_TRIGGER_FILE);
  }

  return dump;
}
#endif
} // namespace Dali::Graphics
