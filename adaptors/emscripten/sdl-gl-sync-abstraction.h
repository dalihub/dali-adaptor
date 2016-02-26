#ifndef __DALI_INTEGRATION_SDL_GL_SYNC_ABSTRACTION_H__
#define __DALI_INTEGRATION_SDL_GL_SYNC_ABSTRACTION_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <dali/integration-api/gl-sync-abstraction.h>

namespace Dali
{

/**
 *
 * This is a stubb GLSyncAbstraction class to pass to core for the Emscripten/browser environment
 */
class DALI_IMPORT_API SdlGlSyncAbstraction : public Dali::Integration::GlSyncAbstraction
{
public:

  virtual ~SdlGlSyncAbstraction();

  /**
   * @copydoc Dali::Integration::GlSyncAbstraction::CreateSyncObject()
   */
  virtual Integration::GlSyncAbstraction::SyncObject* CreateSyncObject();

  /**
   * @copydoc Dali::Integration::GlSyncAbstraction::DestroySyncObject(Integration::GlSyncAbstraction::SyncObject* syncObject)
   */
  virtual void DestroySyncObject(Integration::GlSyncAbstraction::SyncObject* syncObject);
};

} // namespace Dali

#endif // __DALI_INTEGRATION_SDL_GL_SYNC_ABSTRACTION_H__
