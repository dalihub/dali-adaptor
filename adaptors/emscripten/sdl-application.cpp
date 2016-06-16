// EXTERNAL INCLUDES
#include "sdl-application.h"

#include <SDL.h>
#include <SDL_image.h>
#include <X11/Xlib.h>
#include <EGL/egl.h>
#include <SDL_syswm.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stropts.h>
#include <unistd.h>

#include <dali/public-api/dali-core.h>
#include <dali/integration-api/events/touch-event-integ.h>

// INTERNAL INCLUDES
#include "integration-api/egl-interface.h"
#include "base/separate-update-render/frame-time.h"
#include <gl/egl-implementation.h>

#include "platform-abstractions/emscripten/emscripten-callbacks.h"

// emscripten are using SDL version 1.3 that isnt available for download
// but version 2.0 is close to version 1.3 but version 1.2 support removed.
// This is one way it isnt.
#if SDL_MAJOR_VERSION == 2
# define VIDEO_FLAGS SDL_WINDOW_OPENGL
#else
# define VIDEO_FLAGS SDL_OPENGL
#endif

namespace
{

void LogFunction(Dali::Integration::Log::DebugPriority priority, std::string& message)
{
  printf("%s", message.c_str());
  EM_ASM( console.log( message.c_str() ) );
}

}

namespace Dali
{

typedef ::Pixmap XPixmap;
typedef ::Window XWindow;
typedef ::Display XDisplay;
typedef ::Screen XScreen;

const unsigned int SdlApplication::DEFAULT_SURFACE_WIDTH = 600;
const unsigned int SdlApplication::DEFAULT_SURFACE_HEIGHT= 480;


SdlApplication::SdlApplication( size_t surfaceWidth,
                                size_t surfaceHeight,
                                float  horizontalDpi,
                                float  verticalDpi )
  : mCore( NULL ),
    mSurfaceWidth( surfaceWidth ),
    mSurfaceHeight( surfaceHeight ),
    mFrame( 0u ),
    mSeconds(0),
    mMicroSeconds(0)
{

  EGLNativeDisplayType  display        = (EGLNativeDisplayType)XOpenDisplay(NULL);
  bool                  isOwnSurface   = true;
  mEglImplementation.InitializeGles( display, isOwnSurface );

  SdlCreateWindow(surfaceWidth, surfaceHeight, "Dali");

  bool                  isWindowType      = true;
  Dali::ColorDepth      depth             = Dali::COLOR_DEPTH_32;
  mEglImplementation.ChooseConfig( isWindowType, depth );

  EGLNativeWindowType window = NULL;

  mEglImplementation.CreateSurfaceWindow( window, depth );

  mEglImplementation.CreateContext();

  mEglImplementation.MakeContextCurrent();

  //
  // SDL/EGL setup, now create core
  //
  mCore = Dali::Integration::Core::New(
    mRenderController,
    mPlatformAbstraction,
    mGlAbstraction,
    mGlSyncAbstraction,
    mGestureManager,
    ResourcePolicy::DataRetention::DALI_RETAINS_ALL_DATA);

  mCore->ContextCreated();
  mCore->SurfaceResized( mSurfaceWidth, mSurfaceHeight );
  mCore->SetDpi( horizontalDpi, verticalDpi );

  Dali::Integration::Log::InstallLogFunction( LogFunction );

  mCore->SceneCreated();
}

SdlApplication::~SdlApplication()
{
  Dali::Integration::Log::UninstallLogFunction();
  delete mCore;
  SDL_Quit();
}

void SdlApplication::SdlCreateWindow(size_t surfaceWidth,
                                     size_t surfaceHeight,
                                     const std::string &title)
{
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
  {
    DALI_LOG_WARNING("SDL_Init Err: %s\n", SDL_GetError() );
    DALI_ASSERT_ALWAYS(!"Couldn't initialize SDL");
  }

  // load support for the JPG and PNG image formats
  int flags    = IMG_INIT_JPG | IMG_INIT_PNG;
  int initted  = IMG_Init(flags);
  if( (initted & flags) != flags)
  {
    DALI_LOG_WARNING("IMG_Init Err:%s\n", IMG_GetError());
    DALI_ASSERT_ALWAYS("!IMG_Init: Failed to init required jpg and png support!\n");
  }

  float gamma      = 0.0;
  int bpp          = 0; // 0 means current display bpp

  Uint32 video_flags = VIDEO_FLAGS;

  SDL_Surface *surface = SDL_SetVideoMode( static_cast<int>(surfaceWidth),
                                           static_cast<int>(surfaceHeight),
                                           bpp,
                                           video_flags );

  if ( surface == NULL )
  {
    DALI_LOG_WARNING("Couldn't set GL mode: %s\n", SDL_GetError());
    DALI_ASSERT_ALWAYS("Couldn't set GL mode");
    SDL_Quit();
    exit(1);
  }

  /* Set the window manager title bar */
  SDL_WM_SetCaption( title.c_str(), "daliweb" );

  /* Set the gamma for the window */
  if ( gamma != 0.0 )
  {
    SDL_SetGamma(gamma, gamma, gamma);
  }

}

void SdlApplication::DoUpdate( void )
{
  // pump events
  mCore->ProcessEvents();

  // Update Time values
  static Internal::Adaptor::FrameTime frameTime;
  static bool init = false;
  if( !init )
  {
    frameTime.SetMinimumFrameTimeInterval( 16667 );
    init = true;
  }

  static unsigned int frameNo = 0;
  frameNo++;
  frameTime.SetSyncTime(frameNo);

  float lastFrameDelta( 0.0f );
  unsigned int lastSyncTime( 0 );
  unsigned int nextSyncTime( 0 );
  frameTime.PredictNextSyncTime( lastFrameDelta, lastSyncTime, nextSyncTime );

  Integration::UpdateStatus status;

  mCore->Update( lastFrameDelta, lastSyncTime, nextSyncTime, status );

  Dali::Internal::Emscripten::stats.lastFrameDeltaSeconds = lastFrameDelta;
  Dali::Internal::Emscripten::stats.lastSyncTimeMilliseconds = lastSyncTime;
  Dali::Internal::Emscripten::stats.nextSyncTimeMilliseconds = nextSyncTime;

  Dali::Internal::Emscripten::stats.keepUpdating = status.keepUpdating;
  Dali::Internal::Emscripten::stats.needsNotification = status.needsNotification;
  Dali::Internal::Emscripten::stats.secondsFromLastFrame = status.secondsFromLastFrame;

}


void SdlApplication::DoRender()
{
  // render
  mCore->Render( mRenderStatus );

  mFrame++;

  Dali::Internal::Emscripten::stats.frameCount = mFrame;

  mEglImplementation.SwapBuffers();

}

void SdlApplication::SendTouchEvent(double x, double y, int mouseState)
{
  PointState::Type state = PointState::UP;
  if( 0 == mouseState )
  {
    state = PointState::DOWN;
  }
  else if( 1 == mouseState )
  {
    state = PointState::UP;
  }
  else if( 2 == mouseState )
  {
    state = PointState::MOTION;
  }

  Dali::Integration::TouchEvent e;
  Dali::Integration::Point point;
  point.SetState( state );
  point.SetScreenPosition( Vector2( static_cast<float>(x), static_cast<float>(y) ) );
  e.AddPoint( point );

  mCore->QueueEvent(e);
}

void SdlApplication::SetSurfaceWidth( unsigned int width, unsigned height )
{
  mSurfaceWidth = width;
  mSurfaceHeight = height;

  mCore->SurfaceResized( mSurfaceWidth, mSurfaceHeight );
}

}
