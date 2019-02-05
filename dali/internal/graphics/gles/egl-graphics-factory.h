#ifndef DALI_INTERNAL_GRAPHICS_FACTORY_H
#define DALI_INTERNAL_GRAPHICS_FACTORY_H

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

// CLASS HEADER
#include <dali/internal/graphics/common/graphics-factory-interface.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class GraphicsFactory : public GraphicsFactoryInterface
{
public:

  /**
   * Constructor
   */
  GraphicsFactory(EnvironmentOptions& environmentOptions);

  /**
   * Destructor
   */
  virtual ~GraphicsFactory();

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsFactoryInterface::Create()
   */
  Graphics:GraphicsInterface& Create(PositionSize posSize) override;

  /**
   * @copydoc Dali::Internal::Adaptor::GraphicsFactoryInterface::Destroy()
   */
  void Destroy();

private:
  EnvironmentOptions& mEnvironmentOptions;
};

} // Adaptor

} // Internal

} // Dali

#endif // DALI_INTERNAL_GRAPHICS_FACTORY_H
