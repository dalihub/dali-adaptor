#ifndef __DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H__
#define __DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <public-api/font-client.h>
#include <dali/public-api/object/base-object.h>



namespace Dali
{

namespace Internal
{

namespace TextAbstraction
{

/**
 * Implementation of the FontClient
 */

class FontClient :  public Dali::BaseObject
{
public:

  /**
   * Constructor
   */
  FontClient();

  /**
   * Destructor
   */
  ~FontClient();

  /**
   * @copydoc Dali::FontClient::Get()
   */
  static Dali::TextAbstraction::FontClient Get();

private:

  // Undefined copy constructor.
  FontClient( const FontClient& );

  // Undefined assignment constructor.
  FontClient& operator=( FontClient& );

  void* mPlugin; ///< TODO replace this with font client

}; // class FontClient


} // namespace TextAbstraction

} // namespace Internal

inline static Internal::TextAbstraction::FontClient& GetImplementation(Dali::TextAbstraction::FontClient& reordering)
{
  DALI_ASSERT_ALWAYS( reordering && "reordering handle is empty" );
  BaseObject& handle = reordering.GetBaseObject();
  return static_cast<Internal::TextAbstraction::FontClient&>(handle);
}

inline static const  Internal::TextAbstraction::FontClient& GetImplementation(const Dali::TextAbstraction::FontClient& reordering)
{
  DALI_ASSERT_ALWAYS( reordering && "reordering handle is empty" );
  const BaseObject& handle = reordering.GetBaseObject();
  return static_cast<const Internal::TextAbstraction::FontClient&>(handle);
}

} // namespace Dali

#endif // __DALI_INTERNAL_TEXT_ABSTRACTION_FONT_CLIENT_IMPL_H__
