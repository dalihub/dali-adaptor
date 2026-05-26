#ifndef DALI_INTERNAL_CLIPBOARD_FACTORY_H
#define DALI_INTERNAL_CLIPBOARD_FACTORY_H

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

// EXTERNAL INCLUDES
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/clipboard.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * @brief Factory interface for creating platform-specific Clipboard implementations.
 *
 * This mirrors other adaptor factories (SystemFactory, WindowFactory, ...):
 * higher layers ask for a factory and call CreateClipboard() without knowing
 * which backend (ecore_wl, tcore_wl, etc.) is used underneath.
 */
class ClipboardFactory
{
public:
  ClipboardFactory()          = default;
  virtual ~ClipboardFactory() = default;

  /**
   * @brief Create or retrieve a Clipboard handle for the current platform.
   */
  virtual Dali::Clipboard CreateClipboard() = 0;
};

/**
 * @brief Returns a platform-specific Clipboard factory.
 *
 * Implemented per backend (ecore_wl, tcore_wl, x11, generic, ...).
 */
extern std::unique_ptr<ClipboardFactory> GetClipboardFactory();

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_CLIPBOARD_FACTORY_H

