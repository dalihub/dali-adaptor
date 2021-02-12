#ifndef DALI_GRAPHICS_GLES_REFLECTION_H
#define DALI_GRAPHICS_GLES_REFLECTION_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/graphics-api/graphics-reflection.h>
#include <dali/graphics-api/graphics-types.h>
#include <dali/integration-api/gl-abstraction.h>

namespace Dali
{
namespace Graphics
{
class EglGraphicsController;

namespace GLES
{
constexpr uint32_t ERROR_ATTRIBUTE_NOT_FOUND(-1u);

/**
 * Reflection object represents a single full graphics reflection state.
 *
 * The state involves compiled and linked shaders as well as state parameters
 * like blending, stencil, scissors, viewport etc.
 *
 * Some of the parameters can be modified by issuing commands but
 * the Reflection must mark those states
 * as dynamic.
 *
 */
class Reflection : public Dali::Graphics::Reflection
{
public:
  Reflection(Graphics::EglGraphicsController& controller);
  virtual ~Reflection();

  // not copyable
  Reflection(const Reflection&) = delete;
  Reflection& operator=(const Reflection&) = delete;

  /**
   * @brief Gets the location of a vertex attribute.
   *
   * @param [in] name The name of vertex attribute
   * @return The index of the vertex attribute in the shader
   */
  uint32_t GetVertexAttributeLocation(const std::string& name) const;

  /**
   * @brief Gets the format of a vertex attribute.
   *
   * @param [in] location The location of vertex attribute
   * @return The format of a vertex attribute
   */
  Dali::Graphics::VertexInputAttributeFormat GetVertexAttributeFormat(uint32_t location) const;

  /**
   * @brief Gets the name of a vertex attribute.
   *
   * @param [in] location The location of vertex attribute
   * @return The name of the vertex attribute
   */
  std::string GetVertexAttributeName(uint32_t location) const;

  /**
   * @brief Gets the locations of all the vertex attribute in the shader.
   *
   * @return A vector of the locations of all the vertex attributes in the shader
   */
  std::vector<uint32_t> GetVertexAttributeLocations() const;

  // Uniform blocks

  /**
   * @brief Gets the number of uniform blocks in the shader
   *
   * @return The number of uniform blocks
   */
  uint32_t GetUniformBlockCount() const;

  /**
   * @brief Gets the binding point to which the uniform block with the given index is binded.
   *
   * @param [in] index The index of the uniform block
   * @return The binding point
   */
  uint32_t GetUniformBlockBinding(uint32_t index) const;

  /**
   * @brief Gets the size of the uniform block with the given index.
   *
   * @param [in] index The index of the uniform block
   * @return The size of the uniform block
   */
  uint32_t GetUniformBlockSize(uint32_t index) const;

  /**
   * @brief Retrieves the information of the uniform block with the given index.
   *
   * The information includes the name, binding point, size, uniforms inside the uniform block, etc.
   *
   * @param [in] index The index of the uniform block
   * @param [out] out A structure that contains the information of the uniform block
   * @return Whether the uniform block exists or not
   */
  bool GetUniformBlock(uint32_t index, Dali::Graphics::UniformBlockInfo& out) const;

  /**
   * @brief Gets the binding points of all the uniform blocks in the shader.
   *
   * @return A vector of binding points
   */
  std::vector<uint32_t> GetUniformBlockLocations() const;

  /**
   * @brief Gets the name of uniform block with the given index.
   *
   * @param [in] blockIndex The index of the uniform block
   * @return The name of the uniform block
   */
  std::string GetUniformBlockName(uint32_t blockIndex) const;

  /**
   * @brief Gets the number of uniforms in the uniform block with the given index.
   *
   * @param [in] blockIndex The index of the uniform block
   * @return The number of uniforms in the uniform block
   */
  uint32_t GetUniformBlockMemberCount(uint32_t blockIndex) const;

  /**
   * @brief Gets the name of the uniform in the given location within the uniform block.
   *
   * @param [in] blockIndex The index of the uniform block
   * @param [in] memberLocation The location of the uniform within the uniform block
   * @return The name of the uniform
   */
  std::string GetUniformBlockMemberName(uint32_t blockIndex, uint32_t memberLocation) const;

  /**
   * @brief Gets the byte offset of the uniform in the given location within the uniform block.
   *
   * @param [in] blockIndex The index of the uniform block
   * @param [in] memberLocation The location of the uniform within the uniform block
   * @return The byte offset of the uniform
   */
  uint32_t GetUniformBlockMemberOffset(uint32_t blockIndex, uint32_t memberLocation) const;

  // Named uniforms

  /**
   * @brief Gets the information of the uniform by its name.
   *
   * @param [in] name The name of the uniform
   * @param [out] out The information of the uniform
   * @return Whether the uniform exists or not
   */
  bool GetNamedUniform(const std::string& name, Dali::Graphics::UniformInfo& out) const;

  // Sampler

  /**
   * @brief Gets all the sampler uniforms
   *
   * @return A vector of the sampler uniforms
   */
  std::vector<Dali::Graphics::UniformInfo> GetSamplers() const;

  // Language

  /**
   * @brief Retrieves the language of the shader
   *
   * @return The language of the shader
   */
  Graphics::ShaderLanguage GetLanguage() const;

  // Other

  /**
   * @brief Set the program object whose reflection to be built
   *
   * @param [in] glProgram The program object
   */
  void SetGlProgram(uint32_t glProgram);

private:
  /**
   * @brief Build the reflection of vertex attributes
   */
  void BuildVertexAttributeReflection();

  /**
   * @brief Build the reflection of uniforms
   */
  void BuildUniformReflection();

  /**
   * @brief Build the reflection of uniform blocks
   */
  void BuildUniformBlockReflection();

protected:
  Reflection(Reflection&&) = default;
  Reflection& operator=(Reflection&&) = default;

private:
  Graphics::EglGraphicsController& mController; ///< The Graphics controller
  uint32_t                         mGlProgram;  ///< The GL program object

  struct AttributeInfo
  {
    uint32_t                                   location{};
    std::string                                name{};
    Dali::Graphics::VertexInputAttributeFormat format{};
  };

  std::vector<AttributeInfo>              mVertexInputAttributes; ///< List of vertex attributes
  Graphics::UniformBlockInfo              mDefaultUniformBlock{}; ///< The emulated UBO containing all the standalone uniforms
  std::vector<Graphics::UniformInfo>      mUniformOpaques{};      ///< List of opaque uniforms (i.e. samplers)
  std::vector<Graphics::UniformBlockInfo> mUniformBlocks{};       ///< List of uniform blocks
};

} // namespace GLES
} // namespace Graphics
} // namespace Dali

#endif // DALI_GRAPHICS_GLES_REFLECTION_H
