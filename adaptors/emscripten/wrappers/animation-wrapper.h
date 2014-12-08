#ifndef __DALI_ANIMATION_WRAPPER_H__
#define __DALI_ANIMATION_WRAPPER_H__

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
#include <dali/public-api/dali-core.h>
#include "emscripten/emscripten.h"
#include "emscripten/bind.h"

// INTERNAL INCLUDES
#include "handle-wrapper.h"

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Keyframe animation helper functions for Emscripten.
 *
 * Allows a string alphaFunction
 *
 * @param[in] keyFrames The dali keyframes
 * @param[in] progress The keyframe progress
 * @param[in] value The property value
 * @param[in] alphaFunction The alpha function by name
 *
 */
void KeyFramesAddWithAlpha(Dali::KeyFrames& keyFrames, float progress, Property::Value& value,
                           const std::string& alphaFunction);


/**
 * Animation helper functions for Emscripten.
 *
 * Allows string property names without function overloading which is unavailable with Javascript
 *
 * @param[in] self The animation
 * @param[in] handle The handle to animate
 * @param[in] property The property name to animate
 * @param[in] alphaFunction The alpha function
 * @param[in] delay The animation delay
 * @param[in] duration The animation duration
 *
 */
void AnimateTo(Dali::Animation& self,
               Dali::Handle& handle,
               const std::string& property,
               const Dali::Property::Value& destinationValue,
               const Dali::AlphaFunction::BuiltinFunction& alphaFunction,
               const float delay,
               const float duration);

/**
 * AnimateBy helper
 *
 * Allows string property names without function overloading which is unavailable with Javascript
 *
 * @param[in] self The animation
 * @param[in] handle The handle to animate
 * @param[in] property The property name to animate
 * @param[in] alphaFunction The alpha function
 * @param[in] delay The animation delay
 * @param[in] duration The animation duration
 */
void AnimateBy(Dali::Animation& self,
               Dali::Handle& handle,
               const std::string& property,
               const Dali::Property::Value& destinationValue,
               const Dali::AlphaFunction::BuiltinFunction& alphaFunction,
               const float delay,
               const float duration);

/**
 * AnimateBetween helper
 *
 * Allows string property names without function overloading which is unavailable with Javascript
 *
 * @param[in] self The animation
 * @param[in] handle The handle to animate
 * @param[in] property The property name to animate
 * @param[in] alphaFunction The alpha function
 * @param[in] delay The animation delay
 * @param[in] duration The animation duration
 */
void AnimateBetween(Dali::Animation& self,
                    Dali::Handle& handle,
                    const std::string& property,
                    Dali::KeyFrames& keyFrames,
                    const Dali::AlphaFunction::BuiltinFunction& alphaFunction,
                    const float delay,
                    const float duration,
                    const Dali::Animation::Interpolation& interpolation);

/**
 * AnimatePath helper (see AnimateTo docs above)
 *
 * Allows string property names without function overloading which is unavailable with Javascript
 *
 * @param[in] self The animation
 * @param[in] target The handle to animate
 * @param[in] path The dali Path
 * @param[in] forward The path animation forward vector
 * @param[in] delay The animation delay
 * @param[in] duration The animation duration
 */
void AnimatePath(Dali::Animation& self,
                 Dali::Handle& target,
                 const Dali::Path& path,
                 const Dali::Vector3& forward,
                 const Dali::AlphaFunction::BuiltinFunction& alphaFunction,
                 const float delay,
                 const float duration);

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
