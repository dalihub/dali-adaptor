#ifndef DALI_X11_TYPES_H
#define DALI_X11_TYPES_H

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
#include <X11/Xlib.h>

namespace Dali
{

typedef ::Pixmap XPixmap;
typedef ::Window XWindow;
typedef ::Display XDisplay;
typedef ::Screen XScreen;

} // namespace Dali

#endif /* DALI_X11_TYPES_H */
