#ifndef DALI_WEB_ENGINE_HIT_TEST_H
#define DALI_WEB_ENGINE_HIT_TEST_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/common/bitwise-enum.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/object/property-map.h>
#include <string>

namespace Dali
{
/**
 * @brief A class WebEngineHitTest for hit test of web engine.
 */
class WebEngineHitTest
{
public:
  /**
   * @brief Enumeration for mode of hit test.
   */
  enum class HitTestMode
  {
    DEFAULT    = 1 << 1,                           ///< link data.
    NODE_DATA  = 1 << 2,                           ///< extra node data(tag name, node value, attribute infomation, etc).
    IMAGE_DATA = 1 << 3,                           ///< extra image data(image data, image data length, image file name exteionsion, etc).
    ALL        = DEFAULT | NODE_DATA | IMAGE_DATA, ///< all data.
  };

  /**
   * @brief Enumeration for context of hit test result.
   */
  enum class ResultContext
  {
    DOCUMENT  = 1 << 1, ///< anywhere in the document.
    LINK      = 1 << 2, ///< a hyperlink element.
    IMAGE     = 1 << 3, ///< an image element.
    MEDIA     = 1 << 4, ///< a video or audio element.
    SELECTION = 1 << 5, ///< the area is selected.
    EDITABLE  = 1 << 6, ///< the area is editable
    TEXT      = 1 << 7, ///< the area is text
  };

  /**
   * @brief Constructor.
   */
  WebEngineHitTest() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineHitTest() = default;

  /**
   * @brief Get the context of the hit test.
   *
   * @return a bitmask of the hit test context.
   */
  virtual ResultContext GetResultContext() const = 0;

  /**
   * @brief Get the link uri string of the hit test.
   *
   * @return the URI of the link element in the coordinates of the hit test
   */
  virtual std::string GetLinkUri() const = 0;

  /**
   * @brief Get the link title of the hit test.
   *
   * @return the title of the link element in the coordinates of the hit test
   */
  virtual std::string GetLinkTitle() const = 0;

  /**
   * @brief Get the link label of the hit test.
   *
   * @return the label of the link element in the coordinates of the hit test
   */
  virtual std::string GetLinkLabel() const = 0;

  /**
   * @brief Get the image uri of the hit test.
   *
   * @return the URI of the image element in the coordinates of the hit test
   */
  virtual std::string GetImageUri() const = 0;

  /**
   * @brief Get the media uri of the hit test.
   *
   * @return the URI of the media element in the coordinates of the hit test
   */
  virtual std::string GetMediaUri() const = 0;

  /**
   * @brief Get the tag name of hit element of the hit test.
   *
   * @return the tag name of the hit element in the coordinates of the hit test
   */
  virtual std::string GetTagName() const = 0;

  /**
   * @brief Get the node value of hit element of the hit test.
   *
   * @return the node value of the hit element in the coordinates of the hit test
   */
  virtual std::string GetNodeValue() const = 0;

  /**
   * @brief Get the attribute data of hit element of the hit test.
   *
   * @return the attribute data of the hit element in the coordinates of the hit test
   */
  virtual Dali::Property::Map GetAttributes() const = 0;

  /**
   * @brief Get the image file name extension of hit element of the hit test.
   *
   * @return the image fiile name extension of the hit element in the coordinates of the hit test
   */
  virtual std::string GetImageFileNameExtension() const = 0;

  /**
   * @brief Get the image buffer of hit element of the hit test.
   *
   * @return the image buffer of the hit element in the coordinates of the hit test
   */
  virtual Dali::PixelData GetImageBuffer() = 0;
};

// specialization has to be done in the same namespace
template<>
struct EnableBitMaskOperators<WebEngineHitTest::HitTestMode>
{
  static const bool ENABLE = true;
};

template<>
struct EnableBitMaskOperators<WebEngineHitTest::ResultContext>
{
  static const bool ENABLE = true;
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_HIT_TEST_H
