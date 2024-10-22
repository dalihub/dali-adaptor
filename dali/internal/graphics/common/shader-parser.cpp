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

#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/common/shader-parser.h>
#include <sstream>

namespace Dali::Internal::ShaderParser
{
CodeLine TokenizeLine(std::string line)
{
  std::regex word_regex("(\\w+)");
  auto       words_begin =
    std::sregex_iterator(line.begin(), line.end(), word_regex);
  auto words_end = std::sregex_iterator();

  CodeLine lineOfCode;
  lineOfCode.line = line;

  for(auto it = words_begin; it != words_end; ++it)
  {
    const std::smatch& match = *it;
    lineOfCode.tokens.emplace_back(match.position(), match.length());
  }
  return lineOfCode;
}

std::string GetToken(CodeLine& line, int i)
{
  // Function allows retrieving a token from start and from the
  // end of line. Negative 'i' retrieves token from the end. For example:
  // GetToken( line, -1 ) - retrieves last token
  // GetToken( line, 0 ) - retrieves first token
  // GetToken( line, 1 ) - retrieves second (counting from 0) token
  if(abs(i) >= line.tokens.size())
  {
    return "";
  }
  if(i < 0)
  {
    i = int(line.tokens.size()) + i;
  }
  return std::string(std::string_view(&line.line[line.tokens[i].first], line.tokens[i].second));
}

void TokenizeSource(Program& program, ShaderStage stage, std::istream& ss)
{
  Shader* output{nullptr};
  if(stage == ShaderStage::VERTEX)
  {
    output = &program.vertexShader;
  }
  else if(stage == ShaderStage::FRAGMENT)
  {
    output = &program.fragmentShader;
  }

  // Invalid shader stage
  if(output == nullptr)
  {
    return;
  }

  std::string line;
  bool        ignoreLines       = false;
  int         lineNumber        = 0;
  output->customOutputLineIndex = -1; // Assume using gl_FragColor in fragment shader, no index for custom output
  output->mainLine              = -1;
  while(std::getline(ss, line))
  {
    // turn ignoring on
    if(line.substr(0, 12) == "//@ignore:on")
    {
      ignoreLines = true;
      continue;
    }

    // turn ignoring off
    if(ignoreLines)
    {
      if(line.substr(0, 13) == "//@ignore:off")
      {
        ignoreLines = false;
      }
      continue;
    }

    CodeLine lineOfCode = TokenizeLine(line);

    // find out whether fragment shader contains OUTPUT
    if(!lineOfCode.tokens.empty() && stage == ShaderStage::FRAGMENT)
    {
      // Look for at least one OUTPUT int the fragment shader if written
      // for GLSL3. If there is no OUTPUT we assume programmers used GLSL2
      // gl_FragColor.
      if(output->customOutputLineIndex < 0)
      {
        if(GetToken(lineOfCode, 0) == "OUTPUT")
        {
          output->customOutputLineIndex = output->codeLines.size();
        }
      }
      // find main function
      if(output->mainLine < 0 && GetToken(lineOfCode, 0) == "void" && GetToken(lineOfCode, 1) == "main")
      {
        output->mainLine = lineNumber;
      }
    }

    output->codeLines.emplace_back(std::move(lineOfCode));
    lineNumber++;
  }
}

void TokenizeSource(Program& program, ShaderStage stage, const std::string& sourceCodeString)
{
  std::stringstream ss(sourceCodeString);
  TokenizeSource(program, stage, ss);
}

template<class IT>
bool ProcessTokenINPUT(IT& it, Program& program, OutputLanguage lang, ShaderStage stage)
{
  int&              attributeLocation = program.attributeLocation;
  auto&             l                 = *it;
  std::string&      outString         = ((stage == ShaderStage::VERTEX) ? program.vertexShader.output : program.fragmentShader.output);
  std::stringstream ss;
  if(l.tokens.size())
  {
    auto token = GetToken(l, 0);
    if(token == "INPUT")
    {
      auto varName = GetToken(l, -1);
      if(lang == OutputLanguage::SPIRV_GLSL)
      {
        // For vertex stage input locations are incremental
        int location = 0;
        if(stage == ShaderStage::VERTEX)
        {
          location = attributeLocation++;
        }
        else
        {
          auto iter = program.varyings.find(varName);
          if(iter != program.varyings.end())
          {
            location = (*iter).second;
          }
        }

        ss << "layout(location = " << location << ") in" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
        outString += ss.str();
        return true;
      }
      else if(lang == OutputLanguage::GLSL3)
      {
        ss << "in" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
        outString += ss.str();
        return true;
      }
      else if(lang == OutputLanguage::GLSL2)
      {
        if(stage == ShaderStage::VERTEX)
        {
          ss << "attribute" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
        }
        else
        {
          ss << "varying" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
        }
        outString += ss.str();
        return true;
      }
    }
  }
  return false;
}

template<class IT>
bool ProcessTokenOUTPUT(IT& it, Program& program, OutputLanguage lang, ShaderStage stage)
{
  std::string& outString = ((stage == ShaderStage::VERTEX) ? program.vertexShader.output : program.fragmentShader.output);
  auto&        l         = *it;
  if(l.tokens.size())
  {
    auto token = GetToken(l, 0);
    if(token == "OUTPUT")
    {
      if(lang == OutputLanguage::SPIRV_GLSL)
      {
        int               location = -1; // invalid location
        std::stringstream ss;
        if(stage == ShaderStage::VERTEX)
        {
          auto varName = GetToken(l, -1);
          // compare varyings map
          auto iter = program.varyings.find(varName);
          if(iter != program.varyings.end())
          {
            location = (*iter).second;
          }
          // SPIRV requires storing locations
          ss << "layout(location=" << location << ") out" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
          outString += ss.str();
        }
        else
        {
          // for fragment shader the gl_FragColor is our output
          // we will use OUT_COLOR, in such shader we have only single output
          auto varName = GetToken(l, -1);
          ss << "layout(location=0) out" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
          outString += ss.str();
        }
        return true;
      }
      else if(lang == OutputLanguage::GLSL3)
      {
        std::stringstream ss;
        ss << "out"
           << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
        outString += ss.str();
        return true;
      }
      else if(lang == OutputLanguage::GLSL2)
      {
        std::stringstream ss;
        if(stage == ShaderStage::VERTEX)
        {
          ss << "varying" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
          outString += ss.str();
        }
        else
        {
          // get output variable name
          auto& cl      = program.fragmentShader.codeLines[program.fragmentShader.customOutputLineIndex];
          auto  varname = GetToken(cl, -1);
          ss << "#define " << varname << " gl_FragColor\n";
          outString += ss.str();
        }
        return true;
      }
    }
  }
  return false;
}

template<class IT>
bool ProcessTokenUNIFORM(IT& it, Program& program, OutputLanguage lang, ShaderStage stage)
{
  auto& l = *it;
  if(l.tokens.size())
  {
    auto token = GetToken(l, 0);
    if(token == "UNIFORM")
    {
      int&              binding = program.samplerBinding;
      std::string&      outStr  = (stage == ShaderStage::VERTEX) ? program.vertexShader.output : program.fragmentShader.output;
      std::stringstream ss;
      {
        if(lang == OutputLanguage::GLSL2 || lang == OutputLanguage::GLSL3)
        {
          ss << "uniform" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
          outStr += ss.str();
        }
        else if(lang == OutputLanguage::SPIRV_GLSL)
        {
          ss << "layout(binding = " << binding++ << ") uniform" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
          outStr += ss.str();
        }
        else
        {
          DALI_LOG_ERROR("UNIFORM can't be interpreted for selected shader dialect!");
        }
      }
      return true;
    }
  }
  return false;
}

template<class IT>
bool ProcessTokenUNIFORM_BLOCK(IT& it, Program& program, OutputLanguage lang, ShaderStage stage)
{
  auto&             l       = *it;
  int&              binding = program.uboBinding;
  std::string&      outStr  = (stage == ShaderStage::VERTEX) ? program.vertexShader.output : program.fragmentShader.output;
  std::stringstream ss;
  if(l.tokens.size())
  {
    auto token = GetToken(l, 0);
    if(token == "UNIFORM_BLOCK")
    {
      bool gles3plus = false;
      if(lang == OutputLanguage::SPIRV_GLSL)
      {
        ss << "layout(set=0, binding=" << binding << ", std140) uniform" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
        binding++;
        gles3plus = true;
      }
      else if(lang == OutputLanguage::GLSL3)
      {
        ss << "layout(std140) uniform" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
        gles3plus = true;
      }
      if(gles3plus) // remove word UNIFORM for gles3+/spirv
      {
        // iterate block
        l = (*++it);
        while(l.line.find('}') == std::string::npos)
        {
          auto isUniform = (GetToken(l, 0) == "UNIFORM");
          if(isUniform)
          {
            ss << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
          }
          else
          {
            ss << l.line << "\n";
          }
          l = *(++it);
        }
        ss << "};\n";
      }
      else if(lang == OutputLanguage::GLSL2)
      {
        while(l.line.find('{') == std::string::npos)
        {
          l = *(++it);
        }
        l = *(++it);
        while(l.line.find('}') == std::string::npos)
        {
          auto isUniform = (GetToken(l, 0) == "UNIFORM");
          if(isUniform)
          {
            ss << "uniform " << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
          }
          else
          {
            ss << l.line << "\n";
          }
          l = *(++it);
        }
      }
    }
    auto str = ss.str();
    if(str.empty())
    {
      return false;
    }
    outStr += str;
    return true;
  }
  return false;
}

// Links inputs and outputs of two stages and assigns
// location
void LinkProgram(Program& program)
{
  int location = 0;
  for(auto& line : program.vertexShader.codeLines)
  {
    auto token = GetToken(line, 0);
    if(token == std::string("OUTPUT"))
    {
      auto varname              = GetToken(line, -1);
      program.varyings[varname] = location++;
    }
  }
  // Verify
  for(auto& line : program.fragmentShader.codeLines)
  {
    auto token = GetToken(line, 0);
    if(token == std::string("INPUT"))
    {
      auto varname = GetToken(line, -1);
    }
  }
}

void ProcessStage(Program& program, ShaderStage stage, OutputLanguage language)
{
  auto& codeLines = ((stage == ShaderStage::VERTEX) ? program.vertexShader.codeLines : program.fragmentShader.codeLines);
  auto& outString = ((stage == ShaderStage::VERTEX) ? program.vertexShader.output : program.fragmentShader.output);

  int  lineNum     = 0;
  bool textureDone = false;

  // add OUTPUT to the fragment shader if it's not defined (then we assume gl_FragColor has been used)
  if(stage == ShaderStage::FRAGMENT &&
     program.fragmentShader.customOutputLineIndex < 0 &&
     program.fragmentShader.mainLine >= 0 &&
     language != OutputLanguage::GLSL2)
  {
    // Push tokenized extra line into the code that defines the output
    // we add output as _glFragColor and define
    std::string line1 = "OUTPUT mediump vec4 _glFragColor;";
    program.fragmentShader.codeLines.insert(program.fragmentShader.codeLines.begin() + program.fragmentShader.mainLine, TokenizeLine(line1));
    line1 = "#define gl_FragColor _glFragColor";
    program.fragmentShader.codeLines.insert(program.fragmentShader.codeLines.begin() + program.fragmentShader.mainLine, TokenizeLine(line1));
  }

  for(auto it = codeLines.begin(); it != codeLines.end(); ++it)
  {
    auto& line = *it;
    if(lineNum > 0 && !textureDone)
    {
      textureDone = true;
      // For textures we will bring macros compatible with
      // GLES2 however, to turn GLES3 specific code back into GLES2
      // the more complex analysis is needed (turn texture() into texture2D, etc.)
      if(language == OutputLanguage::GLSL2)
      {
        outString += "#define TEXTURE texture2D\n";
        outString += "#define TEXTURE_CUBE textureCube\n";
        outString += "#define TEXTURE_LOD texture2DLod\n";
        outString += "#define TEXTURE_CUBE_LOD textureCubeLod\n";
      }
      else
      {
        outString += "#define TEXTURE texture\n";
        outString += "#define TEXTURE_CUBE texture\n";
        outString += "#define TEXTURE_LOD textureLod\n";
        outString += "#define TEXTURE_CUBE_LOD textureLod\n";

        // redefine textureCube (if used in old GLSL2 style)
        // The old-style GLES2 shaders will be still compatible
        outString += "#define textureCube texture\n";
        outString += "#define texture2D texture\n";
        outString += "#define texture2DLod textureLod\n";
        outString += "#define textureCubeLod textureLod\n";
      }
    }
    lineNum++;
    // no tokens (shouldn't happen?)
    if(line.tokens.empty())
    {
      outString += line.line + "\n";
      continue;
    }

    auto res = ProcessTokenINPUT(it, program, language, stage);
    if(!res)
    {
      res = ProcessTokenOUTPUT(it, program, language, stage);
    }
    if(!res)
    {
      res = ProcessTokenUNIFORM_BLOCK(it, program, language, stage);
    }
    if(!res)
    {
      res = ProcessTokenUNIFORM(it, program, language, stage);
    }
    if(!res)
    {
      outString += line.line + "\n";
    }
  }
}

void Parse(const ShaderParserInfo& parseInfo, std::vector<std::string>& output)
{
  auto vs = std::istringstream(*parseInfo.vertexShaderCode);
  auto fs = std::istringstream(*parseInfo.fragmentShaderCode);

  output.resize(2);

  // Create program
  Program program;

  if(parseInfo.vertexShaderLegacyVersion)
  {
    output[0] = *parseInfo.vertexShaderCode;
  }
  else
  {
    TokenizeSource(program, ShaderStage::VERTEX, vs);
  }

  if(parseInfo.fragmentShaderLegacyVersion)
  {
    output[1] = *parseInfo.fragmentShaderCode;
  }
  else
  {
    TokenizeSource(program, ShaderStage::FRAGMENT, fs);
  }

  // Pick the right GLSL dialect and version based on provided shaders
  if(parseInfo.vertexShaderLegacyVersion && parseInfo.fragmentShaderLegacyVersion)
  {
    // Not touching any shaders, return current code as output
    return;
  }
  else if(!parseInfo.vertexShaderLegacyVersion && !parseInfo.fragmentShaderLegacyVersion)
  {
    // Both shaders need processing and linking
    // Assign the shader version. Since both stages are being converted
    // the version can be assumed.
    if(parseInfo.language == OutputLanguage::GLSL3)
    {
      program.vertexShader.output += "#version 320 es\n";
      program.fragmentShader.output += "#version 320 es\n";
    }
    else if(parseInfo.language == OutputLanguage::GLSL2)
    {
      program.vertexShader.output += "#version 100\n";
      program.fragmentShader.output += "#version 100\n";

      // redefine 'flat' qualifier
      program.vertexShader.output += "#define flat\n";
      program.fragmentShader.output += "#define flat\n";
    }
    else if(parseInfo.language == OutputLanguage::SPIRV_GLSL)
    {
      program.vertexShader.output += "#version 430\n";
      program.fragmentShader.output += "#version 430\n";
    }

    // link inputs and outputs between vertex and fragment shader
    LinkProgram(program);

    ProcessStage(program, ShaderStage::VERTEX, parseInfo.language);
    ProcessStage(program, ShaderStage::FRAGMENT, parseInfo.language);

    output[0] = std::move(program.vertexShader.output);
    output[1] = std::move(program.fragmentShader.output);
  }
  else
  {
    // Case: only vertex shader is modern
    if(!parseInfo.vertexShaderLegacyVersion)
    {
      // update #version
      std::string suffix(parseInfo.outputVersion < 200 ? std::string("\n") : std::string(" es\n"));
      program.vertexShader.output += std::string("#version ") + std::to_string(parseInfo.outputVersion) + suffix;
      program.fragmentShader.output += std::string("#version ") + std::to_string(parseInfo.outputVersion) + suffix;

      auto language = parseInfo.language;
      if(parseInfo.language != OutputLanguage::SPIRV_GLSL)
      {
        if(parseInfo.outputVersion < 200)
        {
          language = OutputLanguage::GLSL2;
        }
        else
        {
          language = OutputLanguage::GLSL3;
        }
      }
      ProcessStage(program, ShaderStage::VERTEX, language);
      output[0] = std::move(program.vertexShader.output);
    }
    // Case: only fragment shader is modern
    else
    {
      // update #version
      std::string suffix(parseInfo.outputVersion < 200 ? std::string("\n") : std::string(" es\n"));
      program.vertexShader.output += std::string("#version ") + std::to_string(parseInfo.outputVersion) + suffix;
      program.fragmentShader.output += std::string("#version ") + std::to_string(parseInfo.outputVersion) + suffix;

      auto language = parseInfo.language;
      if(parseInfo.language != OutputLanguage::SPIRV_GLSL)
      {
        if(parseInfo.outputVersion < 200)
        {
          language = OutputLanguage::GLSL2;
        }
        else
        {
          language = OutputLanguage::GLSL3;
        }
      }

      ProcessStage(program, ShaderStage::FRAGMENT, language);
      output[1] = std::move(program.fragmentShader.output);
    }
  }
}

} // namespace Dali::Internal::ShaderParser
