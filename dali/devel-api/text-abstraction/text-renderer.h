#ifndef DALI_TOOLKIT_TEXT_ABSTRACTION_TEXT_RENDERER_H
#define DALI_TOOLKIT_TEXT_ABSTRACTION_TEXT_RENDERER_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/object/base-handle.h>

// INTERNAL INCLUDES
#include <dali/devel-api/text-abstraction/glyph-info.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

namespace Dali
{

namespace TextAbstraction
{

namespace Internal
{

  // Forward declaration
  class TextRenderer;

} // namespace Internal

/**
 * @brief Renders the given @e glyphs at the given @e positions into a pixel buffer.
 *
 * @note This class renders glyphs not characters.
 *       Font selection, RTL reordering, shaping and layout
 *       has to be done before calling the @e Render()
 *       method of this class.
 */
class DALI_ADAPTOR_API TextRenderer : public BaseHandle
{
public:
  /**
   * @brief Parameters to render the text.
   */
  struct Parameters
  {
    /**
     * @brief Enum with the possible pixel formats of the output pixel buffer.
     */
    enum PixelFormat
    {
      A8,      ///< Alpha channel, 8-bit color depth.
      RGBA8888 ///< Red, Green, Blue and Alpha channels, 8-bit color depth per channel.
    };

    /**
     * @brief Whether the circular layout is clockwise.
     */
    enum CircularLayout
    {
      CLOCKWISE,        ///< The text is laid clockwise on a circular path.
      COUNTER_CLOCKWISE ///< The text is laid counter clockwise on a circular path.
    };

    /**
     * @brief Parameters for the text renderer function.
     */
    Parameters( Vector<GlyphInfo>& glyphs,
                Vector<Vector2>& positions,
                Vector<Vector4>& colors,
                Vector<ColorIndex>& colorIndices,
                Vector<ColorBlendingMode>& blendingMode,
                Vector<bool>& isEmoji )
    : glyphs( glyphs ),
      positions( positions ),
      colors( colors ),
      colorIndices( colorIndices ),
      blendingMode( blendingMode ),
      isEmoji( isEmoji ),
      width{ 0u },
      height{ 0u },
      radius{ 0u },
      circularWidth{ 0u },
      circularHeight{ 0u },
      centerX{ 0 },
      centerY{ 0 },
      beginAngle{ 0.f },
      pixelFormat{ A8 },
      circularLayout{ CLOCKWISE }
    {}

    Vector<GlyphInfo>& glyphs;               ///< The glyphs to be rendered.
    Vector<Vector2>& positions;              ///< The position for each glyph.
    Vector<Vector4>& colors;                 ///< Colors of the glyphs.
    Vector<ColorIndex>& colorIndices;        ///< Indices to the vector of colors for each glyphs.
    Vector<ColorBlendingMode>& blendingMode; ///< How each glyph is going to be blended with the color of the text.
    Vector<bool>& isEmoji;                   ///< Whether each glyph is an emoji.
    unsigned int width;                      ///< The width of the pixel buffer. @note Some implementations may change the width for performance reasons.
    unsigned int height;                     ///< The height of the pixel buffer.
    unsigned int radius;                     ///< The radius in pixels of the circular text.
    unsigned int circularWidth;              ///< The width of the text laid out on an horizontal straight line.
    unsigned int circularHeight;             ///< The height of the text laid out on an horizontal straight line.
    int centerX;                             ///< The 'x' coordinate of the center. For circular layout.
    int centerY;                             ///< The 'y' coordinate of the center. For circular layout.
    float beginAngle;                        ///< The angle in radians where the circular text begins.
    PixelFormat pixelFormat;                 ///< The pixel format of the pixel buffer.
    CircularLayout circularLayout;           ///< The direction of the text's layout.
  };

public:

  /**
   * @brief Create an uninitialized TextRenderer handle.
   *
   */
  TextRenderer();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~TextRenderer();

  /**
   * @brief Retrieve a handle to the TextRenderer instance.
   *
   * @return A handle to the TextRenderer.
   */
  static TextRenderer Get();

  /**
   * @brief Renders the given @e glyphs into a pixel buffer.
   *
   * @param[in] parameters Struct with the glyphs, positions and the size of the pixel buffer.
   *
   * @return The pixel buffer with the text rendered on it.
   */
  Devel::PixelBuffer Render(const Parameters& parameters);

public: // Not intended for application developers.

  /// @cond internal
  /**
   * @brief This constructor is used by TextRenderer::Get().
   *
   * @param[in] implementation A pointer to the internal text renderer object.
   */
  explicit DALI_INTERNAL TextRenderer(Internal::TextRenderer* implementation);
  /// @endcond
};

} // namespace TextAbstraction

} // namespace Dali

#endif // DALI_TOOLKIT_TEXT_ABSTRACTION_TEXT_RENDERER_H
