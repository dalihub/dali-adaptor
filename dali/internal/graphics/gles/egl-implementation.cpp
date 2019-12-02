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
                                      Integration::StencilBufferAvailable stencilBufferRequired )
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
  mIsSurfacelessContextSupported( false ),
  mIsKhrCreateContextSupported( false ),
  mSwapBufferCountAfterResume( 0 )
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

    //@todo see if we can just EGL_DEFAULT_DISPLAY instead
    mEglDisplay = eglGetDisplay(mEglNativeDisplay);
    EGLint error = eglGetError();

    if( mEglDisplay == NULL && error != EGL_SUCCESS )
    {
      throw Dali::DaliException( "", "OpenGL ES is not supported");
    }

    EGLint majorVersion = 0;
    EGLint minorVersion = 0;
    if ( !eglInitialize( mEglDisplay, &majorVersion, &minorVersion ) )
    {
      return false;
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    mGlesInitialized = true;
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

  // We want to display this information all the time, so use the LogMessage directly
  Integration::Log::LogMessage(Integration::Log::DebugInfo, "EGL Information\n"
      "            Vendor:        %s\n"
      "            Version:       %s\n"
      "            Client APIs:   %s\n"
      "            Extensions:    %s\n",
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

  return true;
}

void EglImplementation::DestroyContext( EGLContext& eglContext )
{
  DALI_ASSERT_ALWAYS( mEglContext && "no EGL context" );

  eglDestroyContext( mEglDisplay, eglContext );
  eglContext = 0;
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

    eglSwapBuffers( mEglDisplay, eglSurface );

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

  // destroy the surface
  DestroySurface( eglSurface );

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
