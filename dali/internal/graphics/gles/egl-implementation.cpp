/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/gles/egl-implementation.h>

// EXTERNAL INCLUDES
#include <sstream>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-vector.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/internal/graphics/gles/gl-implementation.h>
#include <dali/internal/graphics/gles/egl-debug.h>

// EGL constants use C style casts
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

namespace
{
  const uint32_t THRESHOLD_SWAPBUFFER_COUNT = 5;
  const uint32_t CHECK_EXTENSION_NUMBER = 2;
  const std::string EGL_KHR_SURFACELESS_CONTEXT = "EGL_KHR_surfaceless_context";
  const std::string EGL_KHR_CREATE_CONTEXT = "EGL_KHR_create_context";
}

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#define TEST_EGL_ERROR(lastCommand) \
{ \
  EGLint err = eglGetError(); \
  if (err != EGL_SUCCESS) \
  { \
    DALI_LOG_ERROR("EGL error after %s\n", lastCommand); \
    Egl::PrintError(err); \
    DALI_ASSERT_ALWAYS(0 && "EGL error"); \
  } \
}

EglImplementation::EglImplementation( int multiSamplingLevel,
                                      Integration::DepthBufferAvailable depthBufferRequired,
                                      Integration::StencilBufferAvailable stencilBufferRequired ,
                                      Integration::PartialUpdateAvailable partialUpdateRequired )
: mContextAttribs(),
  mEglNativeDisplay( 0 ),
  mEglNativeWindow( 0 ),
  mCurrentEglNativePixmap( 0 ),
  mEglDisplay( 0 ),
  mEglConfig( 0 ),
  mEglContext( 0 ),
  mCurrentEglSurface( 0 ),
  mCurrentEglContext( EGL_NO_CONTEXT ),
  mMultiSamplingLevel( multiSamplingLevel ),
  mGlesVersion( 30 ),
  mColorDepth( COLOR_DEPTH_24 ),
  mGlesInitialized( false ),
  mIsOwnSurface( true ),
  mIsWindow( true ),
  mDepthBufferRequired( depthBufferRequired == Integration::DepthBufferAvailable::TRUE ),
  mStencilBufferRequired( stencilBufferRequired == Integration::StencilBufferAvailable::TRUE ),
  mPartialUpdateRequired( partialUpdateRequired == Integration::PartialUpdateAvailable::TRUE ),
  mIsSurfacelessContextSupported( false ),
  mIsKhrCreateContextSupported( false ),
  mSwapBufferCountAfterResume( 0 ),
  mEglSetDamageRegionKHR( 0 ),
  mEglSwapBuffersWithDamageKHR( 0 ),
  mBufferAge( 0 ),
  mFullSwapNextFrame( true )
{
}

EglImplementation::~EglImplementation()
{
  TerminateGles();
}

bool EglImplementation::InitializeGles( EGLNativeDisplayType display, bool isOwnSurface )
{
  if ( !mGlesInitialized )
  {
    mEglNativeDisplay = display;

    // Try to get the display connection for the native display first
    mEglDisplay = eglGetDisplay( mEglNativeDisplay );

    if( mEglDisplay == EGL_NO_DISPLAY )
    {
      // If failed, try to get the default display connection
      mEglDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    }

    if( mEglDisplay == EGL_NO_DISPLAY )
    {
      // Still failed to get a display connection
      throw Dali::DaliException( "", "OpenGL ES is not supported");
    }

    EGLint majorVersion = 0;
    EGLint minorVersion = 0;
    if ( !eglInitialize( mEglDisplay, &majorVersion, &minorVersion ) )
    {
      return false;
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    mIsOwnSurface = isOwnSurface;
  }

  // Query EGL extensions to check whether surfaceless context is supported
  const char* const extensionStr = eglQueryString( mEglDisplay, EGL_EXTENSIONS );
  std::istringstream stream( extensionStr );
  std::string currentExtension;
  uint32_t extensionCheckCount = 0;
  while( std::getline( stream, currentExtension, ' ' ) && extensionCheckCount < CHECK_EXTENSION_NUMBER )
  {
    if( currentExtension == EGL_KHR_SURFACELESS_CONTEXT )
    {
      mIsSurfacelessContextSupported = true;
      extensionCheckCount++;
    }
    if( currentExtension == EGL_KHR_CREATE_CONTEXT )
    {
      mIsKhrCreateContextSupported = true;
      extensionCheckCount++;
    }
  }

  mGlesInitialized = true;

  // We want to display this information all the time, so use the LogMessage directly
  Integration::Log::LogMessage(Integration::Log::DebugInfo, "EGL Information\n"
      "            PartialUpdate  %d\n"
      "            Vendor:        %s\n"
      "            Version:       %s\n"
      "            Client APIs:   %s\n"
      "            Extensions:    %s\n",
      mPartialUpdateRequired,
      eglQueryString( mEglDisplay, EGL_VENDOR ),
      eglQueryString( mEglDisplay, EGL_VERSION ),
      eglQueryString( mEglDisplay, EGL_CLIENT_APIS ),
      extensionStr);

  return mGlesInitialized;
}

bool EglImplementation::CreateContext()
{
  // make sure a context isn't created twice
  DALI_ASSERT_ALWAYS( (mEglContext == 0) && "EGL context recreated" );

  mEglContext = eglCreateContext(mEglDisplay, mEglConfig, NULL, &(mContextAttribs[0]));
  TEST_EGL_ERROR("eglCreateContext render thread");

  DALI_ASSERT_ALWAYS( EGL_NO_CONTEXT != mEglContext && "EGL context not created" );

  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_VENDOR : %s ***\n", glGetString(GL_VENDOR));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_RENDERER : %s ***\n", glGetString(GL_RENDERER));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_VERSION : %s ***\n", glGetString(GL_VERSION));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_SHADING_LANGUAGE_VERSION : %s***\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** Supported Extensions ***\n%s\n\n", glGetString(GL_EXTENSIONS));

  mEglSetDamageRegionKHR = reinterpret_cast<PFNEGLSETDAMAGEREGIONKHRPROC>(eglGetProcAddress("eglSetDamageRegionKHR"));
  if (!mEglSetDamageRegionKHR)
  {
    DALI_LOG_ERROR("Coudn't find eglSetDamageRegionKHR!\n");
    mPartialUpdateRequired = false;
  }
  mEglSwapBuffersWithDamageKHR = reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(eglGetProcAddress("eglSwapBuffersWithDamageKHR"));
  if (!mEglSwapBuffersWithDamageKHR)
  {
    DALI_LOG_ERROR("Coudn't find eglSwapBuffersWithDamageKHR!\n");
    mPartialUpdateRequired = false;
  }
  return true;
}

bool EglImplementation::CreateWindowContext( EGLContext& eglContext )
{
  // make sure a context isn't created twice
  DALI_ASSERT_ALWAYS( (eglContext == 0) && "EGL context recreated" );

  eglContext = eglCreateContext(mEglDisplay, mEglConfig, mEglContext, &(mContextAttribs[0]));
  TEST_EGL_ERROR("eglCreateContext render thread");

  DALI_ASSERT_ALWAYS( EGL_NO_CONTEXT != eglContext && "EGL context not created" );

  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_VENDOR : %s ***\n", glGetString(GL_VENDOR));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_RENDERER : %s ***\n", glGetString(GL_RENDERER));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_VERSION : %s ***\n", glGetString(GL_VERSION));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_SHADING_LANGUAGE_VERSION : %s***\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** Supported Extensions ***\n%s\n\n", glGetString(GL_EXTENSIONS));

  mEglWindowContexts.push_back( eglContext );

  mEglSetDamageRegionKHR = reinterpret_cast<PFNEGLSETDAMAGEREGIONKHRPROC>(eglGetProcAddress("eglSetDamageRegionKHR"));
  if (!mEglSetDamageRegionKHR)
  {
    DALI_LOG_ERROR("Coudn't find eglSetDamageRegionKHR!\n");
    mPartialUpdateRequired = false;
  }
  mEglSwapBuffersWithDamageKHR = reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(eglGetProcAddress("eglSwapBuffersWithDamageKHR"));
  if (!mEglSwapBuffersWithDamageKHR)
  {
    DALI_LOG_ERROR("Coudn't find eglSwapBuffersWithDamageKHR!\n");
    mPartialUpdateRequired = false;
  }
  return true;
}

void EglImplementation::DestroyContext( EGLContext& eglContext )
{
  if( eglContext )
  {
    eglDestroyContext( mEglDisplay, eglContext );
    eglContext = 0;
  }
}

void EglImplementation::DestroySurface( EGLSurface& eglSurface )
{
  if(mIsOwnSurface && eglSurface)
  {
    // Make context null to prevent crash in driver side
    MakeContextNull();
    eglDestroySurface( mEglDisplay, eglSurface );
    eglSurface = 0;
  }
}

void EglImplementation::MakeContextCurrent( EGLSurface eglSurface, EGLContext eglContext )
{
  if (mCurrentEglContext == eglContext)
  {
    return;
  }

  mCurrentEglSurface = eglSurface;

  if(mIsOwnSurface)
  {
    eglMakeCurrent( mEglDisplay, eglSurface, eglSurface, eglContext );

    mCurrentEglContext = eglContext;
  }

  EGLint error = eglGetError();

  if ( error != EGL_SUCCESS )
  {
    Egl::PrintError(error);

    DALI_ASSERT_ALWAYS(false && "MakeContextCurrent failed!");
  }
}

void EglImplementation::MakeCurrent( EGLNativePixmapType pixmap, EGLSurface eglSurface )
{
  if (mCurrentEglContext == mEglContext)
  {
    return;
  }

  mCurrentEglNativePixmap = pixmap;
  mCurrentEglSurface = eglSurface;

  if(mIsOwnSurface)
  {
    eglMakeCurrent( mEglDisplay, eglSurface, eglSurface, mEglContext );

    mCurrentEglContext = mEglContext;
  }

  EGLint error = eglGetError();

  if ( error != EGL_SUCCESS )
  {
    Egl::PrintError(error);

    DALI_ASSERT_ALWAYS(false && "MakeCurrent failed!");
  }
}

void EglImplementation::MakeContextNull()
{
  // clear the current context
  eglMakeCurrent( mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
  mCurrentEglContext = EGL_NO_CONTEXT;
}

void EglImplementation::TerminateGles()
{
  if ( mGlesInitialized )
  {
    // Make context null to prevent crash in driver side
    MakeContextNull();

    for ( auto eglSurface : mEglWindowSurfaces )
    {
      if(mIsOwnSurface && eglSurface)
      {
        eglDestroySurface(mEglDisplay, eglSurface);
      }
    }
    eglDestroyContext(mEglDisplay, mEglContext);
    for ( auto eglContext : mEglWindowContexts )
    {
      eglDestroyContext(mEglDisplay, eglContext);
    }

    eglTerminate(mEglDisplay);

    mEglDisplay = NULL;
    mEglConfig  = NULL;
    mEglContext = NULL;
    mCurrentEglSurface = NULL;
    mCurrentEglContext = EGL_NO_CONTEXT;

    mGlesInitialized = false;
  }
}

bool EglImplementation::IsGlesInitialized() const
{
  return mGlesInitialized;
}

void EglImplementation::SwapBuffers( EGLSurface& eglSurface )
{
  if ( eglSurface != EGL_NO_SURFACE ) // skip if using surfaceless context
  {
#ifndef DALI_PROFILE_UBUNTU
    if( mSwapBufferCountAfterResume < THRESHOLD_SWAPBUFFER_COUNT )
    {
      DALI_LOG_RELEASE_INFO( "EglImplementation::SwapBuffers started.\n" );
    }
#endif //DALI_PROFILE_UBUNTU

    // DALI_LOG_ERROR("EglImplementation::SwapBuffers()\n");
    eglSwapBuffers( mEglDisplay, eglSurface );
    mFullSwapNextFrame = false;

#ifndef DALI_PROFILE_UBUNTU
    if( mSwapBufferCountAfterResume < THRESHOLD_SWAPBUFFER_COUNT )
    {
      DALI_LOG_RELEASE_INFO( "EglImplementation::SwapBuffers finished.\n" );
      mSwapBufferCountAfterResume++;
    }
#endif //DALI_PROFILE_UBUNTU
  }
}

EGLint EglImplementation::GetBufferAge(EGLSurface& eglSurface) const
{
  EGLint age = 0;
  eglQuerySurface(mEglDisplay, eglSurface, EGL_BUFFER_AGE_EXT, &age);
  if (age < 0)
  {
    DALI_LOG_ERROR("eglQuerySurface(%d)\n", eglGetError());
    age = 0;
  }

  // 0 - invalid buffer
  // 1, 2, 3
  if (age > 3)
  {
    DALI_LOG_ERROR("EglImplementation::GetBufferAge() buffer age %d > 3\n", age);
    age = 0; // shoudn't be more than 3 back buffers, if there is just reset, I don't want to add extra history level
  }

  return age;
}

bool EglImplementation::DamageAreasSet() const
{
  return (mDamagedAreas.size() ? true : false);
}

void EglImplementation::SetDamageAreas( std::vector<Dali::Rect<int>>& damagedAreas )
{
  mFullSwapNextFrame = true;
  mDamagedAreas = damagedAreas;
}

void EglImplementation::SetFullSwapNextFrame()
{
  mFullSwapNextFrame = true;
}

void mergeRects(Rect<int>& mergingRect, const std::vector<Rect<int>>& rects)
{
  uint32_t i = 0;
  if (mergingRect.IsEmpty())
  {
    for (;i < rects.size(); i++)
    {
      if (!rects[i].IsEmpty())
      {
        mergingRect = rects[i];
        break;
      }
    }
  }

  for (;i < rects.size(); i++)
  {
    mergingRect.Merge(rects[i]);
  }
}

void insertRects(std::list<std::vector<Rect<int>>>& damagedRectsList, const std::vector<Rect<int>>& damagedRects)
{
  damagedRectsList.push_front(damagedRects);
  if (damagedRectsList.size() > 4) // past triple buffers + current
  {
    damagedRectsList.pop_back();
  }
}

void EglImplementation::SetDamage( EGLSurface& eglSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect )
{
  if (!mPartialUpdateRequired)
  {
    return;
  }

  if (eglSurface != EGL_NO_SURFACE) // skip if using surfaceless context
  {
    EGLint width = 0;
    EGLint height = 0;
    eglQuerySurface(mEglDisplay, eglSurface, EGL_WIDTH, &width);
    eglQuerySurface(mEglDisplay, eglSurface, EGL_HEIGHT, &height);
    Rect<int> surfaceRect(0, 0, width, height);

    mSurfaceRect = surfaceRect;

    if (mFullSwapNextFrame)
    {
      mBufferAge = 0;
      insertRects(mBufferDamagedRects, std::vector<Rect<int>>(1, surfaceRect));
      clippingRect = Rect<int>();
      return;
    }

    EGLint bufferAge = GetBufferAge(eglSurface);
    if (mDamagedAreas.size())
    {
      mBufferAge = bufferAge;
      if (bufferAge == 0)
      {
        // Buffer age is reset
        clippingRect = Rect<int>();
        return;
      }

      mergeRects(clippingRect, mDamagedAreas);
    }
    else
    {
      // Buffer age 0 means the back buffer in invalid and requires full swap
      if (!damagedRects.size() || bufferAge != mBufferAge || bufferAge == 0)
      {
        // No damage or buffer is out of order or buffer age is reset
        mBufferAge = bufferAge;
        insertRects(mBufferDamagedRects, std::vector<Rect<int>>(1, surfaceRect));
        clippingRect = Rect<int>();
        return;
      }

      // We push current frame damaged rects here, zero index for current frame
      mBufferAge = bufferAge;
      insertRects(mBufferDamagedRects, damagedRects);

      // Merge damaged rects into clipping rect
      auto bufferDamagedRects = mBufferDamagedRects.begin();
      while (bufferAge-- >= 0 && bufferDamagedRects != mBufferDamagedRects.end())
      {
        const std::vector<Rect<int>>& rects = *bufferDamagedRects++;
        mergeRects(clippingRect, rects);
      }
    }

    if (!clippingRect.Intersect(surfaceRect) || clippingRect.Area() > surfaceRect.Area() * 0.8)
    {
      // clipping area too big or doesn't intersect surface rect
      clippingRect = Rect<int>();
      return;
    }

    // DALI_LOG_ERROR("eglSetDamageRegionKHR(%d, %d, %d, %d)\n", clippingRect.x, clippingRect.y, clippingRect.width, clippingRect.height);
    EGLBoolean result = mEglSetDamageRegionKHR(mEglDisplay, eglSurface, reinterpret_cast<int*>(&clippingRect), 1);
    if (result == EGL_FALSE)
    {
      DALI_LOG_ERROR("eglSetDamageRegionKHR(%d)\n", eglGetError());
    }
  }
}

void EglImplementation::SwapBuffers(EGLSurface& eglSurface, const std::vector<Rect<int>>& damagedRects)
{
  if (eglSurface != EGL_NO_SURFACE ) // skip if using surfaceless context
  {
    if (!mPartialUpdateRequired || mFullSwapNextFrame || mBufferAge == 0 || !damagedRects.size())
    {
      SwapBuffers(eglSurface);
      return;
    }

#ifndef DALI_PROFILE_UBUNTU
    if( mSwapBufferCountAfterResume < THRESHOLD_SWAPBUFFER_COUNT )
    {
      DALI_LOG_RELEASE_INFO( "EglImplementation::SwapBuffers started.\n" );
    }
#endif //DALI_PROFILE_UBUNTU

    if (mDamagedAreas.size())
    {
      // DALI_LOG_ERROR("EglImplementation::SwapBuffers(%d)\n", mDamagedAreas.size());
      EGLBoolean result = mEglSwapBuffersWithDamageKHR(mEglDisplay, eglSurface, reinterpret_cast<int*>(mDamagedAreas.data()), mDamagedAreas.size());
      if (result == EGL_FALSE)
      {
        DALI_LOG_ERROR("eglSwapBuffersWithDamageKHR(%d)\n", eglGetError());
      }

#ifndef DALI_PROFILE_UBUNTU
      if( mSwapBufferCountAfterResume < THRESHOLD_SWAPBUFFER_COUNT )
      {
        DALI_LOG_RELEASE_INFO( "EglImplementation::SwapBuffers finished.\n" );
        mSwapBufferCountAfterResume++;
      }
#endif //DALI_PROFILE_UBUNTU
      return;
    }

    // current frame damaged rects were pushed by EglImplementation::SetDamage() to 0 index.
    EGLint bufferAge = mBufferAge;
    mCombinedDamagedRects.clear();

    // Combine damaged rects from previous frames (beginning from bufferAge index) with the current frame (0 index)
    auto bufferDamagedRects = mBufferDamagedRects.begin();
    while (bufferAge-- >= 0 && bufferDamagedRects != mBufferDamagedRects.end())
    {
      const std::vector<Rect<int>>& rects = *bufferDamagedRects++;
      mCombinedDamagedRects.insert(mCombinedDamagedRects.end(), rects.begin(), rects.end());
    }

    // Merge intersecting rects, form an array of non intersecting rects to help driver a bit
    // Could be optional and can be removed, needs to be checked with and without on platform
    const int n = mCombinedDamagedRects.size();
    for (int i = 0; i < n-1; i++)
    {
      if (mCombinedDamagedRects[i].IsEmpty())
      {
        continue;
      }

      for (int j = i+1; j < n; j++)
      {
        if (mCombinedDamagedRects[j].IsEmpty())
        {
          continue;
        }

        if (mCombinedDamagedRects[i].Intersects(mCombinedDamagedRects[j]))
        {
          mCombinedDamagedRects[i].Merge(mCombinedDamagedRects[j]);
          mCombinedDamagedRects[j].width = 0;
          mCombinedDamagedRects[j].height = 0;
        }
      }
    }

    int j = 0;
    for (int i = 0; i < n; i++)
    {
      if (!mCombinedDamagedRects[i].IsEmpty())
      {
        mCombinedDamagedRects[j++] = mCombinedDamagedRects[i];
      }
    }

    if (j != 0)
    {
      mCombinedDamagedRects.resize(j);
    }

    if (!mCombinedDamagedRects.size() || (mCombinedDamagedRects[0].Area() > mSurfaceRect.Area() * 0.8))
    {
      SwapBuffers(eglSurface);
      return;
    }

    // DALI_LOG_ERROR("EglImplementation::SwapBuffers(%d)\n", mCombinedDamagedRects.size());
    EGLBoolean result = mEglSwapBuffersWithDamageKHR(mEglDisplay, eglSurface, reinterpret_cast<int*>(mCombinedDamagedRects.data()), mCombinedDamagedRects.size());
    if (result == EGL_FALSE)
    {
      DALI_LOG_ERROR("eglSwapBuffersWithDamageKHR(%d)\n", eglGetError());
    }

#ifndef DALI_PROFILE_UBUNTU
    if( mSwapBufferCountAfterResume < THRESHOLD_SWAPBUFFER_COUNT )
    {
      DALI_LOG_RELEASE_INFO( "EglImplementation::SwapBuffers finished.\n" );
      mSwapBufferCountAfterResume++;
    }
#endif //DALI_PROFILE_UBUNTU
  }
}

void EglImplementation::CopyBuffers( EGLSurface& eglSurface )
{
  eglCopyBuffers( mEglDisplay, eglSurface, mCurrentEglNativePixmap );
}

void EglImplementation::WaitGL()
{
  eglWaitGL();
}

bool EglImplementation::ChooseConfig( bool isWindowType, ColorDepth depth )
{
  if(mEglConfig && isWindowType == mIsWindow && mColorDepth == depth)
  {
    return true;
  }

  mColorDepth = depth;
  mIsWindow = isWindowType;

  EGLint numConfigs;
  Vector<EGLint> configAttribs;
  configAttribs.Reserve(31);

  if(isWindowType)
  {
    configAttribs.PushBack( EGL_SURFACE_TYPE );
    configAttribs.PushBack( EGL_WINDOW_BIT );
  }
  else
  {
    configAttribs.PushBack( EGL_SURFACE_TYPE );
    configAttribs.PushBack( EGL_PIXMAP_BIT );
  }

  configAttribs.PushBack( EGL_RENDERABLE_TYPE );

  if( mGlesVersion >= 30 )
  {
    configAttribs.PushBack( EGL_OPENGL_ES3_BIT_KHR );
  }
  else
  {
    configAttribs.PushBack( EGL_OPENGL_ES2_BIT );
  }

// TODO: enable this flag when it becomes supported
//  configAttribs.PushBack( EGL_CONTEXT_FLAGS_KHR );
//  configAttribs.PushBack( EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR );

  configAttribs.PushBack( EGL_RED_SIZE );
  configAttribs.PushBack( 8 );
  configAttribs.PushBack( EGL_GREEN_SIZE );
  configAttribs.PushBack( 8 );
  configAttribs.PushBack( EGL_BLUE_SIZE );
  configAttribs.PushBack( 8 );

//  For underlay video playback, we also need to set the alpha value of the 24/32bit window.
  configAttribs.PushBack( EGL_ALPHA_SIZE );
  configAttribs.PushBack( 8 );

  configAttribs.PushBack( EGL_DEPTH_SIZE );
  configAttribs.PushBack( mDepthBufferRequired ? 24 : 0 );
  configAttribs.PushBack( EGL_STENCIL_SIZE );
  configAttribs.PushBack( mStencilBufferRequired ? 8 : 0 );

#ifndef DALI_PROFILE_UBUNTU
  if( mMultiSamplingLevel != EGL_DONT_CARE )
  {
    configAttribs.PushBack( EGL_SAMPLES );
    configAttribs.PushBack( mMultiSamplingLevel );
    configAttribs.PushBack( EGL_SAMPLE_BUFFERS );
    configAttribs.PushBack( 1 );
  }
#endif // DALI_PROFILE_UBUNTU
  configAttribs.PushBack( EGL_NONE );

  // Ensure number of configs is set to 1 as on some drivers,
  // eglChooseConfig succeeds but does not actually create a proper configuration.
  if ( ( eglChooseConfig( mEglDisplay, &(configAttribs[0]), &mEglConfig, 1, &numConfigs ) != EGL_TRUE ) ||
       ( numConfigs != 1 ) )
  {
    if( mGlesVersion >= 30 )
    {
      mEglConfig = NULL;
      DALI_LOG_ERROR("Fail to use OpenGL es 3.0. Retrying to use OpenGL es 2.0.");
      return false;
    }

    if ( numConfigs != 1 )
    {
      DALI_LOG_ERROR("No configurations found.\n");

      TEST_EGL_ERROR("eglChooseConfig");
    }

    EGLint error = eglGetError();
    switch (error)
    {
      case EGL_BAD_DISPLAY:
      {
        DALI_LOG_ERROR("Display is not an EGL display connection\n");
        break;
      }
      case EGL_BAD_ATTRIBUTE:
      {
        DALI_LOG_ERROR("The parameter configAttribs contains an invalid frame buffer configuration attribute or an attribute value that is unrecognized or out of range\n");
        break;
      }
      case EGL_NOT_INITIALIZED:
      {
        DALI_LOG_ERROR("Display has not been initialized\n");
        break;
      }
      case EGL_BAD_PARAMETER:
      {
        DALI_LOG_ERROR("The parameter numConfig is NULL\n");
        break;
      }
      default:
      {
        DALI_LOG_ERROR("Unknown error.\n");
      }
    }
    DALI_ASSERT_ALWAYS(false && "eglChooseConfig failed!");
    return false;
  }
  Integration::Log::LogMessage(Integration::Log::DebugInfo, "Using OpenGL es %d.%d.\n", mGlesVersion / 10, mGlesVersion % 10 );

  mContextAttribs.Clear();
  if( mIsKhrCreateContextSupported )
  {
    mContextAttribs.Reserve(5);
    mContextAttribs.PushBack( EGL_CONTEXT_MAJOR_VERSION_KHR );
    mContextAttribs.PushBack( mGlesVersion / 10 );
    mContextAttribs.PushBack( EGL_CONTEXT_MINOR_VERSION_KHR );
    mContextAttribs.PushBack( mGlesVersion % 10 );
  }
  else
  {
    mContextAttribs.Reserve(3);
    mContextAttribs.PushBack( EGL_CONTEXT_CLIENT_VERSION );
    mContextAttribs.PushBack( mGlesVersion / 10 );
  }
  mContextAttribs.PushBack( EGL_NONE );

  return true;
}

EGLSurface EglImplementation::CreateSurfaceWindow( EGLNativeWindowType window, ColorDepth depth )
{
  mEglNativeWindow = window;
  mColorDepth = depth;
  mIsWindow = true;

  // egl choose config
  ChooseConfig(mIsWindow, mColorDepth);

  mCurrentEglSurface = eglCreateWindowSurface( mEglDisplay, mEglConfig, mEglNativeWindow, NULL );
  TEST_EGL_ERROR("eglCreateWindowSurface");

  DALI_ASSERT_ALWAYS( mCurrentEglSurface && "Create window surface failed" );

  return mCurrentEglSurface;
}

EGLSurface EglImplementation::CreateSurfacePixmap( EGLNativePixmapType pixmap, ColorDepth depth )
{
  mCurrentEglNativePixmap = pixmap;
  mColorDepth = depth;
  mIsWindow = false;

  // egl choose config
  ChooseConfig(mIsWindow, mColorDepth);

  mCurrentEglSurface = eglCreatePixmapSurface( mEglDisplay, mEglConfig, mCurrentEglNativePixmap, NULL );
  TEST_EGL_ERROR("eglCreatePixmapSurface");

  DALI_ASSERT_ALWAYS( mCurrentEglSurface && "Create pixmap surface failed" );

  return mCurrentEglSurface;
}

bool EglImplementation::ReplaceSurfaceWindow( EGLNativeWindowType window, EGLSurface& eglSurface, EGLContext& eglContext )
{
  bool contextLost = false;

  // display connection has not changed, then we can just create a new surface
  //  the surface is bound to the context, so set the context to null
  MakeContextNull();

  if( eglSurface )
  {
    // destroy the surface
    DestroySurface( eglSurface );
  }

  // create the EGL surface
  EGLSurface newEglSurface = CreateSurfaceWindow( window, mColorDepth );

  // set the context to be current with the new surface
  MakeContextCurrent( newEglSurface, eglContext );

  return contextLost;
}

bool EglImplementation::ReplaceSurfacePixmap( EGLNativePixmapType pixmap, EGLSurface& eglSurface )
{
  bool contextLost = false;

  // display connection has not changed, then we can just create a new surface
  // create the EGL surface
  eglSurface = CreateSurfacePixmap( pixmap, mColorDepth );

  // set the eglSurface to be current
  MakeCurrent( pixmap, eglSurface );

  return contextLost;
}

void EglImplementation::SetGlesVersion( const int32_t glesVersion )
{
  mGlesVersion = glesVersion;
}

void EglImplementation::SetFirstFrameAfterResume()
{
  mSwapBufferCountAfterResume = 0;
}

EGLDisplay EglImplementation::GetDisplay() const
{
  return mEglDisplay;
}

EGLContext EglImplementation::GetContext() const
{
  return mEglContext;
}

int32_t EglImplementation::GetGlesVersion() const
{
  return mGlesVersion;
}

bool EglImplementation::IsSurfacelessContextSupported() const
{
  return mIsSurfacelessContextSupported;
}

void EglImplementation::WaitClient()
{
  // Wait for EGL to finish executing all rendering calls for the current context
  if ( eglWaitClient() != EGL_TRUE )
  {
    TEST_EGL_ERROR("eglWaitClient");
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
