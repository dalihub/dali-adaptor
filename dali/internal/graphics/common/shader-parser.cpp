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
 *
 */

#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/common/shader-parser.h>
#include <sstream>

// Happy trick for Tizen platform! since 2025-04-22. eunkiki.hong@samsung.com
// Since uniform buffer reduce fps near 10% than before, let we ignore uniform block feature
// except several default SharedUniformBlocks, like "VisualVertBlock".
// If we resolve performance issue, please remove below code!

#ifdef _ARCH_ARM_ // Only do this for MALI driver.
#define IGNORE_UNIFORM_BLOCKS_FOR_NORMAL_CASES 1
#endif

#if IGNORE_UNIFORM_BLOCKS_FOR_NORMAL_CASES
#include <unordered_set>
#endif

namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_SHADER_PARSER");
#endif

const std::vector<std::string> MATRIX_DATA_TYPE_TOKENS{"mat3", "mat4", "mat2"};

inline bool IsWordChar(const char c)
{
  return ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         ('a' <= c && c <= 'z') ||
         (c == '_') ||
         (c == '#');
}

#if IGNORE_UNIFORM_BLOCKS_FOR_NORMAL_CASES
const std::unordered_set<std::string_view> gExceptUniformBlockNames{
  "VisualVertBlock",         ///< Default VisualRenderer properties
  "GaussianBlurSampleBlock", ///< GaussianBlur effects
  "PunctualLightBlock",      ///< For Scene3D::Light
  "ShadowVertBlock",         ///< For Scene3D::Light with Shadow
  "SharedFragmentBlock",     ///< For uniform-blocks.example
  "SharedVertexBlock",       ///< For instance-rendering.example
  "UtcVertBlock",            ///< For UTC
  "UtcFragBlock",            ///< For UTC
};
#endif
} // namespace

namespace Dali::Internal::ShaderParser
{
/**
 * @brief Tokenizes a single line of code.
 * Parse word tokens, which had number and alphabet, and underline.
 * @note The string ownership will be moved to CodeLine's line value.
 * @param line A string containing a single line of code.
 * @return A CodeLine object containing the line of code and its tokens.
 */
CodeLine TokenizeLine(std::string line)
{
  CodeLine lineOfCode;
  if(!line.empty())
  {
    uint32_t ei = 0;

    bool wordState    = false;
    bool commentState = false;

    uint32_t wordStart = 0;

    while(ei < line.size())
    {
      const char c = line[ei];
      if(IsWordChar(c))
      {
        if(!wordState)
        {
          wordState = true;
          wordStart = ei;
        }
        commentState = false;
      }
      else
      {
        if(wordState)
        {
          lineOfCode.tokens.push_back(CodeTokenPair(wordStart, ei - wordStart));
          wordState = false;
        }

        if(c == '/')
        {
          if(commentState)
          {
            // Comment found! Ignore remaind characters
            --ei;
            break;
          }
          commentState = true;
        }
        else
        {
          commentState = false;
        }
      }
      ++ei;
    }

    if(wordState)
    {
      lineOfCode.tokens.push_back(CodeTokenPair(wordStart, ei - wordStart));
    }

    lineOfCode.line = std::move(line);
  }
  return lineOfCode;
}

std::string_view GetToken(CodeLine& line, int i)
{
  // Function allows retrieving a token from start and from the
  // end of line. Negative 'i' retrieves token from the end. For example:
  // GetToken( line, -1 ) - retrieves last token
  // GetToken( line, 0 ) - retrieves first token
  // GetToken( line, 1 ) - retrieves second (counting from 0) token
  if(i < 0)
  {
    i = int(line.tokens.size()) + i;
  }
  if(i < 0 || i >= int(line.tokens.size()))
  {
    return "";
  }
  return std::string_view(&line.line[line.tokens[i].first], line.tokens[i].second);
}

/**
 * Function is looking for any token from the given list.
 *
 * @return return -1 if no token found, or index of token that has been found first
 */
int HasAnyToken(CodeLine& line, const std::vector<std::string>& tokens)
{
  for(const auto& token : line.tokens)
  {
    uint32_t i   = 0;
    auto     str = std::string_view(&line.line[token.first], token.second);
    for(const auto& token2 : tokens)
    {
      if(str == token2)
      {
        return i;
      }
      ++i;
    }
  }
  return -1;
}

bool HasToken(CodeLine& line, std::string_view tokenToFind)
{
  for(const auto& token : line.tokens)
  {
    auto str = std::string_view(&line.line[token.first], token.second);
    if(str == tokenToFind)
    {
      return true;
    }
  }
  return false;
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

  int legacyPrefixCount = 0;

  while(std::getline(ss, line))
  {
    // turn ignoring on
    if(line.substr(0, 12) == "//@ignore:on")
    {
      legacyPrefixCount -= line.size() + 1;
      ignoreLines = true;
      continue;
    }

    // turn ignoring off
    if(ignoreLines)
    {
      legacyPrefixCount -= line.size() + 1;
      if(line.substr(0, 13) == "//@ignore:off")
      {
        ignoreLines = false;
      }
      continue;
    }

    // Do not tokenize legacy-prefix codes if they exist
    if(line.substr(0, 20) == "//@legacy-prefix-end")
    {
      char* end;
      legacyPrefixCount = std::strtoul(reinterpret_cast<const char*>(line.data()) + 21, &end, 10);
    }
    if(legacyPrefixCount > 0)
    {
      legacyPrefixCount -= line.size() + 1;

      CodeLine lineOfCode;
      lineOfCode.line = std::move(line);
      output->codeLines.emplace_back(std::move(lineOfCode));
      lineNumber++;
      continue;
    }

    CodeLine lineOfCode = TokenizeLine(std::move(line));

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
    if(!lineOfCode.tokens.empty())
    {
      if(GetToken(lineOfCode, 0) == "#extension")
      {
        // Emplace extension code and skip this line, without adding it to the codeLines.
        output->extensions.emplace_back(std::move(lineOfCode.line));
        continue;
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

    bool       isInputToken      = (token == "INPUT");
    const bool isFlatKeywordUsed = (token == "FLAT");
    if(isFlatKeywordUsed && stage != ShaderStage::VERTEX)
    {
      // Check secondary token if first token is flat.
      token        = GetToken(l, 1);
      isInputToken = (token == "INPUT");
    }

    if(isInputToken)
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

        ss << "layout(location = " << location << ") " << (isFlatKeywordUsed ? "flat " : "") << "in" << l.line.substr(l.tokens[isFlatKeywordUsed].first + l.tokens[isFlatKeywordUsed].second).c_str() << "\n";
        outString += ss.str();
        return true;
      }
      else if(lang >= OutputLanguage::GLSL_3 && lang <= OutputLanguage::GLSL_3_MAX)
      {
        ss << (isFlatKeywordUsed ? "flat " : "") << "in" << l.line.substr(l.tokens[isFlatKeywordUsed].first + l.tokens[isFlatKeywordUsed].second).c_str() << "\n";
        outString += ss.str();
        return true;
      }
      else if(lang == OutputLanguage::GLSL_100_ES)
      {
        if(stage == ShaderStage::VERTEX)
        {
          ss << "attribute" << l.line.substr(l.tokens[isFlatKeywordUsed].first + l.tokens[isFlatKeywordUsed].second).c_str() << "\n";
        }
        else
        {
          ss << "varying" << l.line.substr(l.tokens[isFlatKeywordUsed].first + l.tokens[isFlatKeywordUsed].second).c_str() << "\n";
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

    bool       isOutputToken     = (token == "OUTPUT");
    const bool isFlatKeywordUsed = (token == "FLAT");
    if(isFlatKeywordUsed && stage != ShaderStage::FRAGMENT)
    {
      // Check secondary token if first token is flat.
      token         = GetToken(l, 1);
      isOutputToken = (token == "OUTPUT");
    }
    if(isOutputToken)
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

          // Dev note : SPIRV don't need to ad flat keyword at Vertex shader.
          // SPIRV requires storing locations
          ss << "layout(location=" << location << ") out" << l.line.substr(l.tokens[isFlatKeywordUsed].first + l.tokens[isFlatKeywordUsed].second).c_str() << "\n";
          outString += ss.str();
        }
        else
        {
          // for fragment shader the gl_FragColor is our output
          // we will use gl_FragColor, in such shader we have only single output
          ss << "layout(location=0) out" << l.line.substr(l.tokens[isFlatKeywordUsed].first + l.tokens[isFlatKeywordUsed].second).c_str() << "\n";
          outString += ss.str();
        }
        return true;
      }
      else if(lang >= OutputLanguage::GLSL_3 && lang <= OutputLanguage::GLSL_3_MAX)
      {
        std::stringstream ss;
        ss << (isFlatKeywordUsed ? "flat " : "") << "out"
           << l.line.substr(l.tokens[isFlatKeywordUsed].first + l.tokens[isFlatKeywordUsed].second).c_str() << "\n";
        outString += ss.str();
        return true;
      }
      else if(lang == OutputLanguage::GLSL_100_ES)
      {
        std::stringstream ss;
        if(stage == ShaderStage::VERTEX)
        {
          ss << "varying" << l.line.substr(l.tokens[isFlatKeywordUsed].first + l.tokens[isFlatKeywordUsed].second).c_str() << "\n";
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
        if(lang == OutputLanguage::GLSL_100_ES ||
           (lang >= OutputLanguage::GLSL_3 && lang <= OutputLanguage::GLSL_3_MAX))
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
    auto token            = GetToken(l, 0);
    auto uniformBlockName = GetToken(l, 1);

    auto localBinding = binding;
    bool blockReused  = false;
    if(!program.uniformBlocks.empty())
    {
      auto it = std::find_if(program.uniformBlocks.begin(), program.uniformBlocks.end(), [&uniformBlockName](const std::pair<std::string, uint32_t>& item)
      { return (item.first == uniformBlockName); });
      if(it != program.uniformBlocks.end())
      {
        localBinding = (*it).second;
        blockReused  = true;
      }
    }

    if(!blockReused)
    {
      program.uniformBlocks.emplace_back(std::make_pair(uniformBlockName, localBinding));
    }

    if(token == "UNIFORM_BLOCK")
    {
      bool gles3plus = false;
      if(lang == OutputLanguage::SPIRV_GLSL)
      {
        ss << "layout(set=0, binding=" << localBinding << ", std140) uniform" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
        // update binding point only if UBO has been bound to the new binding point
        // duplication UBOs between stages should not increment binding point
        if(!blockReused)
        {
          binding++;
        }
        gles3plus = true;
      }
      else if(lang >= OutputLanguage::GLSL_3 && lang <= OutputLanguage::GLSL_3_MAX)
      {
#if IGNORE_UNIFORM_BLOCKS_FOR_NORMAL_CASES
        if(gExceptUniformBlockNames.find(uniformBlockName) != gExceptUniformBlockNames.end())
#endif
        {
          ss << "layout(std140) uniform" << l.line.substr(l.tokens[0].first + l.tokens[0].second).c_str() << "\n";
          gles3plus = true;
        }
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
      else
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

    bool       isOutputToken     = (token == "OUTPUT");
    const bool isFlatKeywordUsed = (token == "FLAT");
    if(isFlatKeywordUsed)
    {
      token         = GetToken(line, 1);
      isOutputToken = (token == "OUTPUT");
    }

    if(isOutputToken)
    {
      auto varname = GetToken(line, -1);

      // The location depends on type, anything larger than vec4 (4 floats)
      // will require adjusting the location, otherwise the location override error
      // will occur.
      auto locationsUsed = 1;
      auto tokenIndex    = HasAnyToken(line, MATRIX_DATA_TYPE_TOKENS);
      if(tokenIndex >= 0)
      {
        // get last character which indicates how many vec4-sizes the type uses
        locationsUsed = int(std::string_view(MATRIX_DATA_TYPE_TOKENS[tokenIndex]).back()) - '0';
      }

      program.varyings[varname] = location;
      location += locationsUsed;
    }
  }
  // Verify
  // For now, we don't verify anything. Just skip this codes.
  // for(auto& line : program.fragmentShader.codeLines)
  // {
  //   auto token = GetToken(line, 0);

  //   bool       isInputToken      = (token == "INPUT");
  //   const bool isFlatKeywordUsed = (token == "FLAT");
  //   if(isFlatKeywordUsed)
  //   {
  //     token = GetToken(line, 1);
  //     isInputToken = (token == "INPUT");
  //   }

  //   if(isInputToken)
  //   {
  //     auto varname = GetToken(line, -1);
  //   }
  // }
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
     language != OutputLanguage::GLSL_100_ES)
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
      if(language == OutputLanguage::GLSL_100_ES)
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
        if(stage == ShaderStage::VERTEX)
        {
          if(language < OutputLanguage::GLSL_3_MAX)
          {
            outString += "#define INSTANCE_INDEX gl_InstanceID\n";
            outString += "#define VERTEX_INDEX gl_VertexID\n";
          }
          else
          {
            outString += "#define INSTANCE_INDEX gl_InstanceIndex\n";
            outString += "#define VERTEX_INDEX gl_VertexIndex\n";
          }
        }

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
    if(parseInfo.language >= OutputLanguage::GLSL_3 &&
       parseInfo.language < OutputLanguage::GLSL_3_MAX)
    {
      std::string version("#version " + std::to_string(int(parseInfo.language)) + " es\n");
      program.vertexShader.output += version;
      program.fragmentShader.output += version;
    }
    else if(parseInfo.language == OutputLanguage::GLSL_100_ES)
    {
      program.vertexShader.output += "#version 100\n";
      program.fragmentShader.output += "#version 100\n";
    }
    else if(parseInfo.language == OutputLanguage::SPIRV_GLSL)
    {
      program.vertexShader.output += "#version 430\n";
      program.fragmentShader.output += "#version 430\n";
    }

    // Define extensions follow after version.
    for(const auto& ext : program.vertexShader.extensions)
    {
      program.vertexShader.output += ext + "\n";
    }
    for(const auto& ext : program.fragmentShader.extensions)
    {
      program.fragmentShader.output += ext + "\n";
    }

    program.vertexShader.output += parseInfo.vertexShaderPrefix;
    program.fragmentShader.output += parseInfo.fragmentShaderPrefix;

    if(parseInfo.language == OutputLanguage::GLSL_100_ES)
    {
      // redefine 'flat' qualifier
      program.vertexShader.output += "#define flat\n";
      program.fragmentShader.output += "#define flat\n";
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

      // Define extensions follow after version.
      for(const auto& ext : program.vertexShader.extensions)
      {
        program.vertexShader.output += ext + "\n";
      }
      for(const auto& ext : program.fragmentShader.extensions)
      {
        program.fragmentShader.output += ext + "\n";
      }

      program.vertexShader.output += parseInfo.vertexShaderPrefix;
      program.fragmentShader.output += parseInfo.fragmentShaderPrefix;

      auto language = parseInfo.language;
      if(parseInfo.language != OutputLanguage::SPIRV_GLSL)
      {
        if(parseInfo.outputVersion < 200)
        {
          language = OutputLanguage::GLSL_100_ES;
        }
        else
        {
          language = OutputLanguage(parseInfo.outputVersion);
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

      // Define extensions follow after version.
      for(const auto& ext : program.vertexShader.extensions)
      {
        program.vertexShader.output += ext + "\n";
      }
      for(const auto& ext : program.fragmentShader.extensions)
      {
        program.fragmentShader.output += ext + "\n";
      }

      program.vertexShader.output += parseInfo.vertexShaderPrefix;
      program.fragmentShader.output += parseInfo.fragmentShaderPrefix;

      auto language = parseInfo.language;
      if(parseInfo.language != OutputLanguage::SPIRV_GLSL)
      {
        if(parseInfo.outputVersion < 200)
        {
          language = OutputLanguage::GLSL_100_ES;
        }
        else
        {
          language = OutputLanguage(parseInfo.outputVersion);
        }
      }

      ProcessStage(program, ShaderStage::FRAGMENT, language);
      output[1] = std::move(program.fragmentShader.output);
    }
  }
  DALI_LOG_INFO(gLogFilter, Debug::General, "ShaderParserInfo: legacy version: vsh: %u fsh: %u\n\n", parseInfo.vertexShaderLegacyVersion, parseInfo.fragmentShaderLegacyVersion);
  DALI_LOG_INFO(gLogFilter, Debug::General, "ShaderParserInfo: language: %s\n", parseInfo.language == OutputLanguage::GLSL_100_ES ? "GLSL_100_ES" : parseInfo.language == OutputLanguage::GLSL_3      ? "GLSL_3/300ES"
                                                                                                                                                  : parseInfo.language == OutputLanguage::GLSL_310_ES ? "GLSL_310_ES"
                                                                                                                                                  : parseInfo.language == OutputLanguage::GLSL_320_ES ? "GLSL_320_ES"
                                                                                                                                                  : parseInfo.language == OutputLanguage::SPIRV_GLSL  ? "SPIRV_GLSL"
                                                                                                                                                                                                      : "Unknown")
  DALI_LOG_INFO(gLogFilter, Debug::General, "ShaderParserInfo: outputVersion: %u\n\n", parseInfo.outputVersion);
  DALI_LOG_INFO(gLogFilter, Debug::General, "Output Vertex shader:\n%s", output[0].c_str());
  DALI_LOG_INFO(gLogFilter, Debug::General, "Output Fragment shader:\n%s", output[1].c_str());
}

} // namespace Dali::Internal::ShaderParser
