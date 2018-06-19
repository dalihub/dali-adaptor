/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/gles20/egl-implementation.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-vector.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/internal/graphics/gles20/gl-implementation.h>
#include <dali/internal/graphics/gles20/egl-debug.h>

// EGL constants use C style casts
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

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
  mMultiSamplingLevel( multiSamplingLevel ),
  mColorDepth( COLOR_DEPTH_24 ),
  mGlesInitialized( false ),
  mIsOwnSurface( true ),
  mContextCurrent( false ),
  mIsWindow( true ),
  mDepthBufferRequired( depthBufferRequired == Integration::DepthBufferAvailable::TRUE ),
  mStencilBufferRequired( stencilBufferRequired == Integration::StencilBufferAvailable::TRUE )
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

    mContextAttribs.Clear();

#if DALI_GLES_VERSION >= 30

    mContextAttribs.Reserve(5);
    mContextAttribs.PushBack( EGL_CONTEXT_MAJOR_VERSION_KHR );
    mContextAttribs.PushBack( DALI_GLES_VERSION / 10 );
    mContextAttribs.PushBack( EGL_CONTEXT_MINOR_VERSION_KHR );
    mContextAttribs.PushBack( DALI_GLES_VERSION % 10 );

#else // DALI_GLES_VERSION >= 30

    mContextAttribs.Reserve(3);
    mContextAttribs.PushBack( EGL_CONTEXT_CLIENT_VERSION );
    mContextAttribs.PushBack( 2 );

#endif // DALI_GLES_VERSION >= 30

    mContextAttribs.PushBack( EGL_NONE );

    mGlesInitialized = true;
    mIsOwnSurface = isOwnSurface;
  }

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

void EglImplementation::DestroyContext()
{
  DALI_ASSERT_ALWAYS( mEglContext && "no EGL context" );

  eglDestroyContext( mEglDisplay, mEglContext );
  mEglContext = 0;
}

void EglImplementation::DestroySurface()
{
  if(mIsOwnSurface && mCurrentEglSurface)
  {
    // Make context null to prevent crash in driver side
    MakeContextNull();
    eglDestroySurface( mEglDisplay, mCurrentEglSurface );
    mCurrentEglSurface = 0;
  }
}

void EglImplementation::MakeContextCurrent()
{
  mContextCurrent = true;

  if(mIsOwnSurface)
  {
    eglMakeCurrent( mEglDisplay, mCurrentEglSurface, mCurrentEglSurface, mEglContext );
  }

  EGLint error = eglGetError();

  if ( error != EGL_SUCCESS )
  {
    Egl::PrintError(error);

    DALI_ASSERT_ALWAYS(false && "MakeContextCurrent failed!");
  }

  // We want to display this information all the time, so use the LogMessage directly
  Integration::Log::LogMessage(Integration::Log::DebugInfo, "EGL Information\n"
      "            Vendor:        %s\n"
      "            Version:       %s\n"
      "            Client APIs:   %s\n"
      "            Extensions:    %s\n",
      eglQueryString(mEglDisplay, EGL_VENDOR),
      eglQueryString(mEglDisplay, EGL_VERSION),
      eglQueryString(mEglDisplay, EGL_CLIENT_APIS),
      eglQueryString(mEglDisplay, EGL_EXTENSIONS));
}

void EglImplementation::MakeCurrent( EGLNativePixmapType pixmap, EGLSurface eglSurface )
{
  mCurrentEglNativePixmap = pixmap;
  mCurrentEglSurface = eglSurface;

  if(mIsOwnSurface)
  {
    eglMakeCurrent( mEglDisplay, mCurrentEglSurface, mCurrentEglSurface, mEglContext );
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
  mContextCurrent = false;
  // clear the current context
  eglMakeCurrent( mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
}

void EglImplementation::TerminateGles()
{
  if ( mGlesInitialized )
  {
    // Make context null to prevent crash in driver side
    MakeContextNull();

    if(mIsOwnSurface && mCurrentEglSurface)
    {
      eglDestroySurface(mEglDisplay, mCurrentEglSurface);
    }
    eglDestroyContext(mEglDisplay, mEglContext);

    eglTerminate(mEglDisplay);

    mEglDisplay = NULL;
    mEglConfig  = NULL;
    mEglContext = NULL;
    mCurrentEglSurface = NULL;

    mGlesInitialized = false;
  }
}

bool EglImplementation::IsGlesInitialized() const
{
  return mGlesInitialized;
}

void EglImplementation::SwapBuffers()
{
  eglSwapBuffers( mEglDisplay, mCurrentEglSurface );
}

void EglImplementation::CopyBuffers()
{
  eglCopyBuffers( mEglDisplay, mCurrentEglSurface, mCurrentEglNativePixmap );
}

void EglImplementation::WaitGL()
{
  eglWaitGL();
}

void EglImplementation::ChooseConfig( bool isWindowType, ColorDepth depth )
{
  if(mEglConfig && isWindowType == mIsWindow && mColorDepth == depth)
  {
    return;
  }

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

#if DALI_GLES_VERSION >= 30

#ifdef _ARCH_ARM_
  configAttribs.PushBack( EGL_OPENGL_ES3_BIT_KHR );
#else
  // There is a bug in the desktop emulator
  // Requesting for ES3 causes eglCreateContext even though it allows to ask
  // for a configuration that supports GLES 3.0
  configAttribs.PushBack( EGL_OPENGL_ES2_BIT );
#endif // _ARCH_ARM_

#else // DALI_GLES_VERSION >= 30

  Integration::Log::LogMessage( Integration::Log::DebugInfo, "Using OpenGL ES 2 \n" );
  configAttribs.PushBack( EGL_OPENGL_ES2_BIT );

#endif //DALI_GLES_VERSION >= 30

#if DALI_GLES_VERSION >= 30
// TODO: enable this flag when it becomes supported
//  configAttribs.PushBack( EGL_CONTEXT_FLAGS_KHR );
//  configAttribs.PushBack( EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR );
#endif //DALI_GLES_VERSION >= 30

  configAttribs.PushBack( EGL_RED_SIZE );
  configAttribs.PushBack( 8 );
  configAttribs.PushBack( EGL_GREEN_SIZE );
  configAttribs.PushBack( 8 );
  configAttribs.PushBack( EGL_BLUE_SIZE );
  configAttribs.PushBack( 8 );

  configAttribs.PushBack( EGL_ALPHA_SIZE );
#ifdef _ARCH_ARM_
  // For underlay video playback, we also need to set the alpha value of the 24/32bit window.
  configAttribs.PushBack( 8 );
#else
  // There is a bug in the desktop emulator
  // setting EGL_ALPHA_SIZE to 8 results in eglChooseConfig failing
  configAttribs.PushBack( 0 );
#endif // _ARCH_ARM_

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

  if ( eglChooseConfig( mEglDisplay, &(configAttribs[0]), &mEglConfig, 1, &numConfigs ) != EGL_TRUE )
  {
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
  }

  if ( numConfigs != 1 )
  {
    DALI_LOG_ERROR("No configurations found.\n");

    TEST_EGL_ERROR("eglChooseConfig");
  }
}

void EglImplementation::CreateSurfaceWindow( EGLNativeWindowType window, ColorDepth depth )
{
  DALI_ASSERT_ALWAYS( ( mCurrentEglSurface == 0 ) && "EGL surface already exists" );

  mEglNativeWindow = window;
  mColorDepth = depth;
  mIsWindow = true;

  // egl choose config
  ChooseConfig(mIsWindow, mColorDepth);

  mCurrentEglSurface = eglCreateWindowSurface( mEglDisplay, mEglConfig, mEglNativeWindow, NULL );
  TEST_EGL_ERROR("eglCreateWindowSurface");

  DALI_ASSERT_ALWAYS( mCurrentEglSurface && "Create window surface failed" );
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

bool EglImplementation::ReplaceSurfaceWindow( EGLNativeWindowType window )
{
  bool contextLost = false;

  // display connection has not changed, then we can just create a new surface
  //  the surface is bound to the context, so set the context to null
  MakeContextNull();

  // destroy the surface
  DestroySurface();

  // create the EGL surface
  CreateSurfaceWindow( window, mColorDepth );

  // set the context to be current with the new surface
  MakeContextCurrent();

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

EGLDisplay EglImplementation::GetDisplay() const
{
  return mEglDisplay;
}

EGLDisplay EglImplementation::GetContext() const
{
  return mEglContext;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
