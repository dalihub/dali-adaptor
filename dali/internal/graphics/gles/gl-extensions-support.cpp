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

// CLASS HEADER
#include <dali/internal/graphics/gles/gl-extensions-support.h>

// EXTERNAL HEADER
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include <algorithm> // for std::find_if
#include <sstream>
#include <string>
#include <string_view>
#include <utility> // for std::pair

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
static constexpr const char* KHR_BLEND_EQUATION_ADVANCED        = "GL_KHR_blend_equation_advanced";
static constexpr const char* EXT_MULTISAMPLED_RENDER_TO_TEXTURE = "GL_EXT_multisampled_render_to_texture";

} // namespace

namespace GlExtensionCache
{
void GlExtensionSupportedCacheList::EnsureGlExtensionSupportedCheck()
{
  // Note that this function calls at most one time.
  // But the number of GL_EXTENSIONS itmes are so vairous.
  // We need to reduce extension checkup

  const char* const  extensionStr = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
  std::istringstream stream(extensionStr);
  std::string        currentExtension;

  // Create expected string-to-checkertype convertor.
  using StringTypePair = std::pair<std::string_view, GlExtensionCheckerType>;
  // clang-format off
  std::vector<StringTypePair> extensionStringToTypeList
  {
    {KHR_BLEND_EQUATION_ADVANCED,        GlExtensionCheckerType::BLEND_EQUATION_ADVANCED       },

#ifndef DALI_PROFILE_UBUNTU
    // @todo : Current ubuntu profile's multisamples FBO make crash when eglDestroyContext.
    // Invalidate multisampled render to texture feature hardly.
    {EXT_MULTISAMPLED_RENDER_TO_TEXTURE, GlExtensionCheckerType::MULTISAMPLED_RENDER_TO_TEXTURE},
#endif //DALI_PROFILE_UBUNTU

    ///< Append additional extension checker type here.
  };
  // clang-format on

  while(!extensionStringToTypeList.empty() && std::getline(stream, currentExtension, ' '))
  {
    auto findResult = std::find_if(extensionStringToTypeList.begin(),
                                   extensionStringToTypeList.end(),
                                   [&currentExtension](const StringTypePair& value) {
                                     return value.first == currentExtension;
                                   });
    if(findResult != extensionStringToTypeList.end())
    {
      auto type = findResult->second;

      // Mark as True.
      MarkSupported(type, true);

      // Remove from list. we don't need to check this extension.
      extensionStringToTypeList.erase(findResult);
    }
  }

  // Set supported as false if extension keyword not exist.
  SetAllUncachedAsNotSupported();
}
} // namespace GlExtensionCache
} // namespace Adaptor
} // namespace Internal
} // namespace Dali
