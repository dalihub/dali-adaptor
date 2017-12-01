#ifndef DALI_WIDGET_H
#define DALI_WIDGET_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-handle.h>

namespace Dali
{

  /**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Internal
{

namespace Adaptor
{
class Widget;
}

}

class Window;

/**
 * @brief Widget class is the base class for custom widget.
 *
 * To make own Widget, user should inherit this class and its impl class.
 *
 */
class DALI_IMPORT_API Widget : public BaseHandle
{
public:

  /**
   * @brief Struct for termination type of widget instance.
   */
  struct Termination
  {
    /**
     * @brief Enumeration for termination type of widget instance.
     */
    enum Type
    {
      PERMANENT, //< User deleted this widget from the viewer
      TEMPORARY, //< Widget is deleted because of other reasons (e.g. widget process is terminated temporarily by the system)
    };
  };

public:

  /**
   * @brief This is the constructor for Widget.
   * @return A handle to the Widget
   */
  static Widget New();

  /**
   * @brief The default constructor.
   *
   */
  Widget();

  /**
   * @brief Copy Constructor.
   * @param[in] Widget Handle to an object
   */
  Widget( const Widget& widget );

  /**
   * @brief Assignment operator.
   * @param[in] Widget Handle to an object
   * @return A reference to this
   */
  Widget& operator=( const Widget& widget );

  /**
   * @brief Destructor
   */
  ~Widget();

public: // Not intended for application developers
  explicit Widget(Internal::Adaptor::Widget* widget);
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_WIDGET_H
