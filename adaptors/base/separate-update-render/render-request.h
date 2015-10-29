#ifndef __DALI_INTERNAL_RENDER_REQUEST_H__
#define __DALI_INTERNAL_RENDER_REQUEST_H__

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

// INTERNAL INCLUDES
#include <egl-interface.h>
#include <render-surface.h> // needed for Dali::RenderSurface

namespace Dali
{

class RenderSurface;
class DisplayConnection;

namespace Internal
{
namespace Adaptor
{

class RenderRequest
{
public:
  enum Request
  {
    REPLACE_SURFACE, // Request to replace surface
  };

  /**
   * Constructor.
   * @param[in] type The type of the request
   */
  RenderRequest( Request type );

  /**
   * @return the type of the request
   */
  Request GetType();

private:
  Request mRequestType;
};

class ReplaceSurfaceRequest : public RenderRequest
{
public:

  /**
   * Constructor
   */
  ReplaceSurfaceRequest();

  /**
   * Set the new surface
   * @param[in] newSurface The new surface to use
   */
  void SetSurface(RenderSurface* newSurface);

  /**
   * @return the new surface
   */
  RenderSurface* GetSurface();

  /**
   * Called when the request has been completed to set the result.
   */
  void ReplaceCompleted();

  /**
   * @return true if the replace has completed.
   */
  bool GetReplaceCompleted();

private:
  RenderSurface* mNewSurface;     ///< The new surface to use.
  unsigned int mReplaceCompleted; ///< Set to true when the replace has completed.
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_RENDER_REQUEST_H__
