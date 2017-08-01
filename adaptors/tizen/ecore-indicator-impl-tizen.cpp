/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include "ecore-indicator-impl.h"

// EXTERNAL INCLUDES
#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Wayland.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <dali/public-api/images/native-image.h>
#include <dali/public-api/events/touch-data.h>
#include <dali/public-api/common/stage.h>
#include <dali/public-api/images/buffer-image.h>
#include <dali/devel-api/images/texture-set-image.h>
#include <dali/integration-api/debug.h>

#include <tbm_surface.h>
#include <wayland-extension/tizen-remote-surface-client-protocol.h>
#include <wayland-client.h>
#include <wayland-tbm-client.h>

// INTERNAL INCLUDES
#include <adaptor-impl.h>
#include <accessibility-adaptor-impl.h>
#include <native-image-source.h>

using Dali::Vector4;

#if defined(DEBUG_ENABLED)
#define STATE_DEBUG_STRING(state) (state==DISCONNECTED?"DISCONNECTED":state==CONNECTED?"CONNECTED":"UNKNOWN")
#endif

namespace
{

const float SLIDING_ANIMATION_DURATION( 0.2f ); // 200 milli seconds
const float AUTO_INDICATOR_STAY_DURATION(3.0f); // 3 seconds
const float SHOWING_DISTANCE_HEIGHT_RATE(0.34f); // 20 pixels

enum
{
  KEEP_SHOWING = -1,
  HIDE_NOW = 0
};

const int NUM_GRADIENT_INTERVALS(5); // Number of gradient intervals
const float GRADIENT_ALPHA[NUM_GRADIENT_INTERVALS+1] = { 0.6f, 0.38f, 0.20f, 0.08f, 0.0f, 0.0f };

#define MAKE_SHADER(A)#A

const char* BACKGROUND_VERTEX_SHADER = MAKE_SHADER(
  attribute mediump vec2 aPosition;
  attribute mediump float aAlpha;
  varying mediump float vAlpha;
  uniform mediump mat4 uMvpMatrix;
  uniform mediump vec3 uSize;

  void main()
  {
    mediump vec4 vertexPosition = vec4( aPosition * uSize.xy, 0.0, 1.0 );
    vertexPosition = uMvpMatrix * vertexPosition;

    vAlpha = aAlpha;
    gl_Position = vertexPosition;
  }
);

const char* BACKGROUND_FRAGMENT_SHADER = MAKE_SHADER(
  uniform lowp vec4 uColor;
  varying mediump float vAlpha;

  void main()
  {
    gl_FragColor = uColor;
    gl_FragColor.a *= vAlpha;
  }
);

const char* FOREGROUND_VERTEX_SHADER = DALI_COMPOSE_SHADER(
  attribute mediump vec2 aPosition;\n
  varying mediump vec2 vTexCoord;\n
  uniform mediump mat4 uMvpMatrix;\n
  uniform mediump vec3 uSize;\n
  uniform mediump vec4 sTextureRect;\n
  \n
  void main()\n
  {\n
    gl_Position = uMvpMatrix * vec4(aPosition * uSize.xy, 0.0, 1.0);\n
    vTexCoord = aPosition + vec2(0.5);\n
  }\n
);

const char* FOREGROUND_FRAGMENT_SHADER = DALI_COMPOSE_SHADER(
  varying mediump vec2 vTexCoord;\n
  uniform sampler2D sTexture;\n
  \n
  void main()\n
  {\n
    gl_FragColor = texture2D( sTexture, vTexCoord );\n // the foreground does not apply actor color
  }\n
);

const char* FOREGROUND_TBM_FRAGMENT_SHADER = DALI_COMPOSE_SHADER(
  varying mediump vec2 vTexCoord;\n
  uniform samplerExternalOES sTexture;\n
  \n
  void main()\n
  {\n
    gl_FragColor = texture2D( sTexture, vTexCoord );\n // the foreground does not apply actor color
  }\n
);

Dali::Geometry CreateQuadGeometry()
{
  Dali::Property::Map quadVertexFormat;
  quadVertexFormat["aPosition"] = Dali::Property::VECTOR2;
  Dali::PropertyBuffer vertexData = Dali::PropertyBuffer::New( quadVertexFormat );

  const float halfQuadSize = .5f;
  struct QuadVertex { Dali::Vector2 position; };
  QuadVertex quadVertexData[4] = {
      { Dali::Vector2(-halfQuadSize, -halfQuadSize) },
      { Dali::Vector2(-halfQuadSize, halfQuadSize) },
      { Dali::Vector2( halfQuadSize, -halfQuadSize) },
      { Dali::Vector2( halfQuadSize, halfQuadSize) } };
  vertexData.SetData(quadVertexData, 4);

  Dali::Geometry quad = Dali::Geometry::New();
  quad.AddVertexBuffer( vertexData );
  quad.SetType( Dali::Geometry::TRIANGLE_STRIP );
  return quad;
}

const float OPAQUE_THRESHOLD(0.99f);
const float TRANSPARENT_THRESHOLD(0.05f);

// indicator service name
const char* INDICATOR_SERVICE_NAME("elm_indicator");

// Copied from ecore_evas_extn_engine.h

#define NBUF 2

enum // opcodes
{
   OP_RESIZE,
   OP_SHOW,
   OP_HIDE,
   OP_FOCUS,
   OP_UNFOCUS,
   OP_UPDATE,
   OP_UPDATE_DONE,
   OP_SHM_REF0,
   OP_SHM_REF1,
   OP_SHM_REF2,
   OP_PROFILE_CHANGE_REQUEST,
   OP_PROFILE_CHANGE_DONE,
   OP_EV_MOUSE_IN,
   OP_EV_MOUSE_OUT,
   OP_EV_MOUSE_UP,
   OP_EV_MOUSE_DOWN,
   OP_EV_MOUSE_MOVE,
   OP_EV_MOUSE_WHEEL,
   OP_EV_MULTI_UP,
   OP_EV_MULTI_DOWN,
   OP_EV_MULTI_MOVE,
   OP_EV_KEY_UP,
   OP_EV_KEY_DOWN,
   OP_EV_HOLD,
   OP_MSG_PARENT,
   OP_MSG,
   OP_GL_REF,
};

// Copied from elm_conform.c

const int MSG_DOMAIN_CONTROL_INDICATOR( 0x10001 );
const int MSG_ID_INDICATOR_REPEAT_EVENT( 0x10002 );
const int MSG_ID_INDICATOR_ROTATION( 0x10003 );
const int MSG_ID_INDICATOR_OPACITY( 0X1004 );
const int MSG_ID_INDICATOR_TYPE( 0X1005 );
const int MSG_ID_INDICATOR_START_ANIMATION( 0X10006 );

struct IpcDataUpdate
{
   int x, w, y, h;
};

struct IpcDataResize
{
  int w, h;
};

struct IpcIndicatorDataAnimation
{
  unsigned int xwin;
  double       duration;
};

struct IpcDataEvMouseUp
{
  int               b;
  Evas_Button_Flags flags;
  int               mask;
  unsigned int      timestamp;
  Evas_Event_Flags  event_flags;

  IpcDataEvMouseUp(unsigned long timestamp)
  : b(1),
    flags(EVAS_BUTTON_NONE),
    mask(0),
    timestamp(static_cast<unsigned int>(timestamp)),
    event_flags(EVAS_EVENT_FLAG_NONE)
  {
  }
};

struct IpcDataEvMouseDown
{
  int                b;
  Evas_Button_Flags  flags;
  int                mask;
  unsigned int       timestamp;
  Evas_Event_Flags   event_flags;

  IpcDataEvMouseDown(unsigned long timestamp)
  : b(1),
    flags(EVAS_BUTTON_NONE),
    mask(0),
    timestamp(static_cast<unsigned int>(timestamp)),
    event_flags(EVAS_EVENT_FLAG_NONE)
  {
  }
};

struct IpcDataEvMouseMove
{
  int                x, y;
  Evas_Button_Flags  flags;
  int                mask;
  unsigned int       timestamp;
  Evas_Event_Flags   event_flags;

  IpcDataEvMouseMove(const Dali::TouchData& touchData, unsigned long timestamp)
  : x(static_cast<Evas_Coord>(touchData.GetLocalPosition( 0 ).x)),
    y(static_cast<Evas_Coord>(touchData.GetLocalPosition( 0 ).y)),
    flags(EVAS_BUTTON_NONE),
    mask(0),
    timestamp(static_cast<unsigned int>(timestamp)),
    event_flags(EVAS_EVENT_FLAG_NONE)
  {
  }
};

struct IpcDataEvMouseOut
{
  unsigned int     timestamp;
  int              mask;
  Evas_Event_Flags event_flags;

  IpcDataEvMouseOut(unsigned long timestamp)
  : timestamp(static_cast<unsigned int>(timestamp)),
    mask(0),
    event_flags(EVAS_EVENT_FLAG_NONE)
  {
  }
};

struct wl_buffer* preBuffer;

static void OnUpdateIndicatorImage( void* data, struct tizen_remote_surface* remoteSurface, struct wl_buffer* buffer, uint32_t time )
{
  Dali::Internal::Adaptor::Indicator* indicator = static_cast< Dali::Internal::Adaptor::Indicator* >( data );

  if( indicator )
  {
    tbm_surface_h tbmSurface = static_cast< tbm_surface_h >( wl_buffer_get_user_data( buffer ) );

    indicator->UpdateIndicatorImage( tbmSurface );
  }

  if( preBuffer != NULL && tizen_remote_surface_get_version( remoteSurface ) >= TIZEN_REMOTE_SURFACE_RELEASE_SINCE_VERSION )
  {
    tizen_remote_surface_release( remoteSurface, preBuffer );
  }

  preBuffer = buffer;
}

static void OnMissingIndicatorImage( void* data, struct tizen_remote_surface* surface )
{
  DALI_LOG_ERROR( "Missing indicator remote surface\n" );
}

static void OnChangedIndicatorImage( void* data, struct tizen_remote_surface* remoteSurface, uint32_t type, struct wl_buffer* buffer,
                                     int32_t fd, uint32_t fileSize, uint32_t time, struct wl_array* keys )
{
  Dali::Internal::Adaptor::Indicator* indicator = static_cast< Dali::Internal::Adaptor::Indicator* >( data );

  if( indicator && type == TIZEN_REMOTE_SURFACE_BUFFER_TYPE_TBM )
  {
    tbm_surface_h tbmSurface = static_cast< tbm_surface_h >( wl_buffer_get_user_data( buffer ) );

    indicator->UpdateIndicatorImage( tbmSurface );
  }

  if( preBuffer != NULL && tizen_remote_surface_get_version( remoteSurface ) >= TIZEN_REMOTE_SURFACE_RELEASE_SINCE_VERSION )
  {
    tizen_remote_surface_release( remoteSurface, preBuffer );
  }

  preBuffer = buffer;

  close( fd );
}

static const struct tizen_remote_surface_listener remoteSurfaceCallback =
{
  OnUpdateIndicatorImage,
  OnMissingIndicatorImage,
  OnChangedIndicatorImage,
};

} // anonymous namespace


namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gIndicatorLogFilter = Debug::Filter::New(Debug::Concise, false, "LOG_INDICATOR");
#endif

// Impl to hide EFL implementation.

struct Indicator::Impl
{
  enum // operation mode
  {
    INDICATOR_HIDE,
    INDICATOR_STAY_WITH_DURATION
  };

  /**
   * Constructor
   */
  Impl(Indicator* indicator)
  : mIndicator(indicator),
    mEcoreEventHandler(NULL)
  {
#if defined(DALI_PROFILE_MOBILE)
    mEcoreEventHandler = ecore_event_handler_add(ECORE_WL_EVENT_INDICATOR_FLICK,  EcoreEventIndicator, this);
#endif // WAYLAND && DALI_PROFILE_MOBILE
  }

  /**
   * Destructor
   */
  ~Impl()
  {
    if ( mEcoreEventHandler )
    {
      ecore_event_handler_del(mEcoreEventHandler);
    }
  }

  static void SetIndicatorVisibility( void* data, int operation )
  {
    Indicator::Impl* indicatorImpl((Indicator::Impl*)data);

    if ( indicatorImpl == NULL || indicatorImpl->mIndicator == NULL)
    {
      return;
    }
    if ( operation == INDICATOR_STAY_WITH_DURATION )
    {
      // if indicator is not showing, INDICATOR_FLICK_DONE is given
      if( indicatorImpl->mIndicator->mVisible == Dali::Window::AUTO &&
          !indicatorImpl->mIndicator->mIsShowing )
      {
        indicatorImpl->mIndicator->ShowIndicator( AUTO_INDICATOR_STAY_DURATION );
      }
    }
    else if( operation == INDICATOR_HIDE )
    {
      if( indicatorImpl->mIndicator->mVisible == Dali::Window::AUTO &&
          indicatorImpl->mIndicator->mIsShowing )
      {
        indicatorImpl->mIndicator->ShowIndicator( HIDE_NOW );
      }
    }
  }
#if defined(DALI_PROFILE_MOBILE)
  /**
   * Called when the Ecore indicator event is received.
   */
  static Eina_Bool EcoreEventIndicator( void* data, int type, void* event )
  {
    SetIndicatorVisibility( data, INDICATOR_STAY_WITH_DURATION );
    return ECORE_CALLBACK_PASS_ON;
  }
#endif // WAYLAND && DALI_PROFILE_MOBILE

  // Data
  Indicator*           mIndicator;
  Ecore_Event_Handler* mEcoreEventHandler;
};

Indicator::LockFile::LockFile(const std::string filename)
: mFilename(filename),
  mErrorThrown(false)
{
  mFileDescriptor = open(filename.c_str(), O_RDWR);
  if( mFileDescriptor == -1 )
  {
    mFileDescriptor = 0;
    mErrorThrown = true;
    DALI_LOG_ERROR( "### Cannot open %s for indicator lock ###\n", mFilename.c_str() );
  }
}

Indicator::LockFile::~LockFile()
{
  // Closing file descriptor also unlocks file.
  close( mFileDescriptor );
}

bool Indicator::LockFile::Lock()
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  bool locked = false;
  if( mFileDescriptor > 0 )
  {
    struct flock filelock;

    filelock.l_type = F_RDLCK;
    filelock.l_whence = SEEK_SET;
    filelock.l_start = 0;
    filelock.l_len = 0;
    if( fcntl( mFileDescriptor, F_SETLKW, &filelock ) == -1 )
    {
      mErrorThrown = true;
      DALI_LOG_ERROR( "### Failed to lock with fd : %s ###\n", mFilename.c_str() );
    }
    else
    {
      locked = true;
    }
  }
  else
  {
    mErrorThrown = true;
    DALI_LOG_ERROR( "### Invalid fd ###\n" );
  }

  return locked;
}

void Indicator::LockFile::Unlock()
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  struct flock filelock;

  filelock.l_type = F_UNLCK;
  filelock.l_whence = SEEK_SET;
  filelock.l_start = 0;
  filelock.l_len = 0;
  if (fcntl(mFileDescriptor, F_SETLKW, &filelock) == -1)
  {
    mErrorThrown = true;
    DALI_LOG_ERROR( "### Failed to lock with fd : %s ###\n", mFilename.c_str() );
  }
}

bool Indicator::LockFile::RetrieveAndClearErrorStatus()
{
  bool error = mErrorThrown;
  mErrorThrown = false;
  return error;
}

Indicator::ScopedLock::ScopedLock(LockFile* lockFile)
: mLockFile(lockFile),
  mLocked(false)
{
  if(mLockFile)
  {
    mLocked = mLockFile->Lock();
  }
}

Indicator::ScopedLock::~ScopedLock()
{
  if( mLockFile )
  {
    mLockFile->Unlock();
  }
}

bool Indicator::ScopedLock::IsLocked()
{
  return mLocked;
}

Indicator::Indicator( Adaptor* adaptor, Dali::Window::WindowOrientation orientation, IndicatorInterface::Observer* observer )
: mPixmap( 0 ),
  mGestureDeltaY( 0.0f ),
  mGestureDetected( false ),
  mConnection( this ),
  mOpacityMode( Dali::Window::OPAQUE ),
  mState( DISCONNECTED ),
  mAdaptor(adaptor),
  mServerConnection( NULL ),
  mObserver( observer ),
  mOrientation( orientation ),
  mImageWidth( 0 ),
  mImageHeight( 0 ),
  mVisible( Dali::Window::INVISIBLE ),
  mIsShowing( true ),
  mIsAnimationPlaying( false ),
  mCurrentSharedFile( 0 ),
  mSharedBufferType( BUFFER_TYPE_SHM ),
  mImpl( NULL ),
  mBackgroundVisible( false ),
  mTopMargin( 0 )
{
  mIndicatorContentActor = Dali::Actor::New();
  mIndicatorContentActor.SetParentOrigin( ParentOrigin::TOP_CENTER );
  mIndicatorContentActor.SetAnchorPoint( AnchorPoint::TOP_CENTER );

  // Indicator image handles the touch event including "leave"
  mIndicatorContentActor.SetLeaveRequired( true );
  mIndicatorContentActor.TouchSignal().Connect( this, &Indicator::OnTouched );
  mIndicatorContentActor.SetColor( Color::BLACK );

  mIndicatorActor = Dali::Actor::New();
  mIndicatorActor.Add( mIndicatorContentActor );

  // Event handler to find out flick down gesture
  mEventActor = Dali::Actor::New();
  mEventActor.SetParentOrigin( ParentOrigin::TOP_CENTER );
  mEventActor.SetAnchorPoint( AnchorPoint::TOP_CENTER );
  mIndicatorActor.Add( mEventActor );

  // Attach pan gesture to find flick down during hiding.
  // It can prevent the problem that scrollview gets pan gesture even indicator area is touched,
  // since it consumes the pan gesture in advance.
  mPanDetector = Dali::PanGestureDetector::New();
  mPanDetector.DetectedSignal().Connect( this, &Indicator::OnPan );
  mPanDetector.Attach( mEventActor );

  Open( orientation );

  // register indicator to accessibility adaptor
  Dali::AccessibilityAdaptor accessibilityAdaptor = AccessibilityAdaptor::Get();
  if(accessibilityAdaptor)
  {
    AccessibilityAdaptor::GetImplementation( accessibilityAdaptor ).SetIndicator( this );
  }
  // hide the indicator by default
  mIndicatorActor.SetVisible( false );

  // create impl to handle ecore event
  mImpl = new Impl(this);
}

Indicator::~Indicator()
{
  if(mImpl)
  {
    delete mImpl;
    mImpl = NULL;
  }

  if(mEventActor)
  {
    mEventActor.TouchSignal().Disconnect( this, &Indicator::OnTouched );
  }
  Disconnect();
}

void Indicator::SetAdaptor(Adaptor* adaptor)
{
  mAdaptor = adaptor;
  mIndicatorBuffer->SetAdaptor( adaptor );
}

Dali::Actor Indicator::GetActor()
{
  return mIndicatorActor;
}

void Indicator::Open( Dali::Window::WindowOrientation orientation )
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  // Calls from Window should be set up to ensure we are in a
  // disconnected state before opening a second time.
  DALI_ASSERT_DEBUG( mState == DISCONNECTED );

  mOrientation = orientation;

  Connect();

  // Change background visibility depending on orientation
  if( mOrientation == Dali::Window::LANDSCAPE || mOrientation == Dali::Window::LANDSCAPE_INVERSE  )
  {
    if( mBackgroundRenderer )
    {
      mIndicatorContentActor.RemoveRenderer( mBackgroundRenderer );
      mBackgroundVisible = false;
    }
  }
  else
  {
    SetOpacityMode( mOpacityMode );
  }
}

void Indicator::Close()
{
  DALI_LOG_TRACE_METHOD_FMT( gIndicatorLogFilter, "State: %s", STATE_DEBUG_STRING(mState) );

  if( mState == CONNECTED )
  {
    Disconnect();
    if( mObserver != NULL )
    {
      mObserver->IndicatorClosed( this );
    }
  }

  Dali::Texture emptyTexture;
  SetForegroundImage( emptyTexture );
}

void Indicator::SetOpacityMode( Dali::Window::IndicatorBgOpacity mode )
{
  mOpacityMode = mode;

  Dali::Geometry geometry = CreateBackgroundGeometry();
  if( geometry )
  {
    if( mBackgroundRenderer )
    {
      if( mBackgroundRenderer.GetGeometry() != geometry )
      {
        mBackgroundRenderer.SetGeometry( geometry );
      }
    }
    else
    {
      if( !mBackgroundShader )
      {
        mBackgroundShader = Dali::Shader::New( BACKGROUND_VERTEX_SHADER, BACKGROUND_FRAGMENT_SHADER, Dali::Shader::Hint::OUTPUT_IS_TRANSPARENT );
      }

      mBackgroundRenderer = Dali::Renderer::New( geometry, mBackgroundShader );
    }

    if( !mBackgroundVisible )
    {
      mIndicatorContentActor.AddRenderer( mBackgroundRenderer );
      mBackgroundVisible = true;
    }
  }
  else if( mBackgroundRenderer )
  {
    mIndicatorContentActor.RemoveRenderer( mBackgroundRenderer );
    mBackgroundVisible = false;
  }
  UpdateTopMargin();
}

void Indicator::SetVisible( Dali::Window::IndicatorVisibleMode visibleMode, bool forceUpdate )
{
  if ( visibleMode != mVisible || forceUpdate )
  {
    // If we were previously hidden, then we should update the image data before we display the indicator
    if ( mVisible == Dali::Window::INVISIBLE )
    {
      UpdateImageData( mCurrentSharedFile );
    }

    if ( visibleMode == Dali::Window::INVISIBLE )
    {
      if (mServerConnection)
      {
        mServerConnection->SendEvent( OP_HIDE, NULL, 0 );
      }
    }
    else
    {
      mIndicatorActor.SetVisible( true );

      if( mServerConnection )
      {
         mServerConnection->SendEvent( OP_SHOW, NULL, 0 );
      }
    }

    mVisible = visibleMode;
    UpdateTopMargin();

    if( mForegroundRenderer &&
        ( mForegroundRenderer.GetTextures().GetTexture( 0u ) ||
          Dali::TextureGetImage( mForegroundRenderer.GetTextures(), 0u ) )
      )
    {
      if( CheckVisibleState() && mVisible == Dali::Window::AUTO )
      {
        // hide indicator
        ShowIndicator( AUTO_INDICATOR_STAY_DURATION /* stay n sec */ );
      }
      else if( CheckVisibleState() && mVisible == Dali::Window::VISIBLE )
      {
        // show indicator
        ShowIndicator( KEEP_SHOWING );
      }
      else
      {
        // hide indicator
        ShowIndicator( HIDE_NOW );
      }
    }
    else
    {
      mIsShowing = false;
    }
  }
}

bool Indicator::IsConnected()
{
  return ( mState == CONNECTED );
}

bool Indicator::SendMessage( int messageDomain, int messageId, const void *data, int size )
{
 if(IsConnected())
 {
   return mServerConnection->SendEvent( OP_MSG, messageDomain, messageId, data, size );
 }
 else
 {
   return false;
 }
}

bool Indicator::OnTouched(Dali::Actor indicator, const Dali::TouchData& touchData)
{
  if( mServerConnection )
  {
    // Send touch event to indicator server when indicator is showing
    if( CheckVisibleState() || mIsShowing )
    {
      switch( touchData.GetState( 0 ) )
      {
        case Dali::PointState::DOWN:
        {
          IpcDataEvMouseMove ipcMove( touchData, touchData.GetTime() );
          IpcDataEvMouseDown ipcDown( touchData.GetTime() );
          mServerConnection->SendEvent( OP_EV_MOUSE_MOVE, &ipcMove, sizeof(ipcMove) );
          mServerConnection->SendEvent( OP_EV_MOUSE_DOWN, &ipcDown, sizeof(ipcDown) );

          if( mVisible == Dali::Window::AUTO )
          {
            // Stop hiding indicator
            ShowIndicator( KEEP_SHOWING );
          }
        }
        break;

        case Dali::PointState::MOTION:
        {
          IpcDataEvMouseMove ipcMove( touchData, touchData.GetTime() );
          mServerConnection->SendEvent( OP_EV_MOUSE_MOVE, &ipcMove, sizeof(ipcMove) );
        }
        break;

        case Dali::PointState::UP:
        case Dali::PointState::INTERRUPTED:
        {
          IpcDataEvMouseUp ipcUp( touchData.GetTime() );
          mServerConnection->SendEvent( OP_EV_MOUSE_UP, &ipcUp, sizeof(ipcUp) );

          if( mVisible == Dali::Window::AUTO )
          {
            // Hide indicator
            ShowIndicator( 0.5f /* hide after 0.5 sec */ );
          }
        }
        break;

        case Dali::PointState::LEAVE:
        {
          IpcDataEvMouseMove ipcMove( touchData, touchData.GetTime() );
          mServerConnection->SendEvent( OP_EV_MOUSE_MOVE, &ipcMove, sizeof(ipcMove) );
          IpcDataEvMouseUp ipcOut( touchData.GetTime() );
          mServerConnection->SendEvent( OP_EV_MOUSE_OUT, &ipcOut, sizeof(ipcOut) );
        }
        break;

        default:
          break;
      }

      return true;
    }
  }

  return false;
}

bool Indicator::Connect()
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  DALI_ASSERT_DEBUG( mState == DISCONNECTED );

  bool connected = false;

  mServerConnection = new ServerConnection( INDICATOR_SERVICE_NAME, 0, false, this );
  if( mServerConnection )
  {
    connected = mServerConnection->IsConnected();
    if( ! connected )
    {
      delete mServerConnection;
      mServerConnection = NULL;
    }
  }

  if( !connected )
  {
    StartReconnectionTimer();
  }
  else
  {
    mState = CONNECTED;
  }

  return connected;
}

void Indicator::StartReconnectionTimer()
{
  if( ! mReconnectTimer )
  {
    mReconnectTimer = Dali::Timer::New(1000);
    mConnection.DisconnectAll();
    mReconnectTimer.TickSignal().Connect( mConnection, &Indicator::OnReconnectTimer );
  }
  mReconnectTimer.Start();
}

bool Indicator::OnReconnectTimer()
{
  bool retry = false;

  if( mState == DISCONNECTED )
  {
    if( !Connect() )
    {
      retry = true;
    }
  }

  return retry;
}

void Indicator::Disconnect()
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  mState = DISCONNECTED;

  delete mServerConnection;
  mServerConnection = NULL;

  ClearSharedFileInfo();
}

void Indicator::Resize( int width, int height )
{
  if( width < 1 )
  {
    width = 1;
  }
  if( height < 1 )
  {
    height = 1;
  }

  if( mImageWidth != width || mImageHeight != height )
  {
    mImageWidth = width;
    mImageHeight = height;

    mIndicatorContentActor.SetSize( mImageWidth, mImageHeight );
    mIndicatorActor.SetSize( mImageWidth, mImageHeight );
    mEventActor.SetSize(mImageWidth, mImageHeight);
    UpdateTopMargin();
  }
}

void Indicator::SetLockFileInfo( Ecore_Ipc_Event_Server_Data *epcEvent )
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  // epcEvent->ref == w
  // epcEvent->ref_to == h
  // epcEvent->response == buffer num
  // epcEvent->data = lockfile + nul byte

  if( (epcEvent->ref > 0) && (epcEvent->ref_to > 0) && (epcEvent->data) &&
      (((unsigned char *)epcEvent->data)[epcEvent->size - 1] == 0) )
  {
    int n = epcEvent->response;

    if( n >= 0 && n < SHARED_FILE_NUMBER )
    {
      mCurrentSharedFile = n;

      mSharedFileInfo[n].mImageWidth  = epcEvent->ref;
      mSharedFileInfo[n].mImageHeight = epcEvent->ref_to;

      mSharedFileInfo[n].mLockFileName.clear();

      mSharedFileInfo[n].mLockFileName = static_cast< char* >( epcEvent->data );

      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "SetLockFileInfo: buffer num = %d, w = %d, h = %d, lock = %s\n",
                     n, mSharedFileInfo[n].mImageWidth, mSharedFileInfo[n].mImageHeight, mSharedFileInfo[n].mLockFileName.c_str() );
    }
  }
}

void Indicator::SetSharedImageInfo( Ecore_Ipc_Event_Server_Data *epcEvent )
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  // epcEvent->ref == shm id
  // epcEvent->ref_to == shm num
  // epcEvent->response == buffer num
  // epcEvent->data = shm ref string + nul byte

  if ( (epcEvent->data) &&
       (epcEvent->size > 0) &&
       (((unsigned char *)epcEvent->data)[epcEvent->size - 1] == 0) )
  {
    int n = epcEvent->response;

    if( n >= 0 && n < SHARED_FILE_NUMBER )
    {
      mCurrentSharedFile = n;

      mSharedFileInfo[n].mSharedFileName.clear();

      mSharedFileInfo[n].mSharedFileName = static_cast< char* >( epcEvent->data );

      mSharedFileInfo[n].mSharedFileID = epcEvent->ref;
      mSharedFileInfo[n].mSharedFileNumber = epcEvent->ref_to;

      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "SetSharedImageInfo: buffer num %d, shared file = %s, id = %d, num = %d\n",
                     n, mSharedFileInfo[n].mSharedFileName.c_str(), mSharedFileInfo[n].mSharedFileID, mSharedFileInfo[n].mSharedFileNumber );
    }
  }
}

void Indicator::LoadSharedImage( Ecore_Ipc_Event_Server_Data *epcEvent )
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  // epcEvent->ref == alpha
  // epcEvent->ref_to == sys
  // epcEvent->response == buffer num

  if ( mSharedBufferType != BUFFER_TYPE_SHM )
  {
    return ;
  }

  int n = epcEvent->response;

  if( n >= 0 && n < SHARED_FILE_NUMBER )
  {
    mCurrentSharedFile = n;

    delete mSharedFileInfo[n].mSharedFile;
    mSharedFileInfo[n].mSharedFile = NULL;

    delete mSharedFileInfo[n].mLock;
    mSharedFileInfo[n].mLock = NULL;

    std::stringstream sharedFileID;
    std::stringstream sharedFileNumber;

    sharedFileID << mSharedFileInfo[n].mSharedFileID;
    sharedFileNumber << mSharedFileInfo[n].mSharedFileNumber;

    std::string sharedFilename = "/" + mSharedFileInfo[n].mSharedFileName + "-" + sharedFileID.str() + "." + sharedFileNumber.str();

    DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "LoadSharedImage: file name = %s\n", sharedFilename.c_str() );

    mSharedFileInfo[n].mSharedFile = SharedFile::New( sharedFilename.c_str(), mSharedFileInfo[n].mImageWidth * mSharedFileInfo[n].mImageWidth * 4, true );
    if( mSharedFileInfo[n].mSharedFile != NULL )
    {
      mSharedFileInfo[n].mLock = new Indicator::LockFile( mSharedFileInfo[n].mLockFileName );
      if( mSharedFileInfo[n].mLock->RetrieveAndClearErrorStatus() )
      {
        DALI_LOG_ERROR( "### Indicator error: Cannot open lock file %s ###\n", mSharedFileInfo[n].mLockFileName.c_str() );

        return;
      }

      CreateNewImage( n );
      UpdateVisibility();
    }
  }
}

void Indicator::SetupNativeIndicatorImage( Ecore_Ipc_Event_Server_Data *epcEvent )
{
  // Bind tizen remote surface
  Eina_Inlist* globals;
  Ecore_Wl_Global* global;
  struct tizen_remote_surface_manager* remoteSurfaceManager = NULL;
  struct tizen_remote_surface* remoteSurface = NULL;
  struct wl_registry* registry;
  struct wayland_tbm_client* tbmClient;
  struct wl_tbm* wlTbm;

  if ( !remoteSurfaceManager )
  {
    registry = ecore_wl_registry_get();
    globals = ecore_wl_globals_get();

    if (!registry || !globals)
    {
      DALI_LOG_ERROR( "SetupNativeIndicatorImage registry or globals error\n" );
      return;
    }

    EINA_INLIST_FOREACH(globals, global)
    {
      if ( !strcmp( global->interface, "tizen_remote_surface_manager" ) )
      {
        remoteSurfaceManager = ( struct tizen_remote_surface_manager* )wl_registry_bind( registry, global->id, &tizen_remote_surface_manager_interface, ( ( global->version < 3 )? global->version: 3 ) );
      }
    }
  }
  if ( !remoteSurfaceManager )
  {
    DALI_LOG_ERROR( "SetupNativeIndicatorImage bind error\n" );
    return;
  }

  tbmClient = ( struct wayland_tbm_client* )wayland_tbm_client_init( ecore_wl_display_get() );
  if( !tbmClient )
  {
    DALI_LOG_ERROR( "SetupNativeIndicatorImage client init error\n" );
    return;
  }

  wlTbm = ( struct wl_tbm* )wayland_tbm_client_get_wl_tbm( tbmClient );
  if( !wlTbm )
  {
    DALI_LOG_ERROR( "SetupNativeIndicatorImage wl tbm error\n" );
    return;
  }

  uint32_t resourceId = epcEvent->ref;
  remoteSurface = tizen_remote_surface_manager_create_surface( remoteSurfaceManager, resourceId, wlTbm );

  if( !remoteSurface )
  {
    DALI_LOG_ERROR( "SetupNativeIndicatorImage create surface error\n" );
    return;
  }

  tizen_remote_surface_add_listener( remoteSurface, &remoteSurfaceCallback, this );
  tizen_remote_surface_redirect( remoteSurface );
  tizen_remote_surface_transfer_visibility( remoteSurface, TIZEN_REMOTE_SURFACE_VISIBILITY_TYPE_VISIBLE);
}

void Indicator::UpdateIndicatorImage( Any source )
{
  if( !mNativeImageSource )
  {
    mNativeImageSource = Dali::NativeImageSource::New( source );
    Dali::NativeImage nativeImage = Dali::NativeImage::New( *mNativeImageSource );

    SetForegroundNativeImage( nativeImage );
    mIndicatorContentActor.SetSize( mImageWidth, mImageHeight );
    mIndicatorActor.SetSize( mImageWidth, mImageHeight );
    mEventActor.SetSize( mImageWidth, mImageHeight );

    UpdateVisibility();
    return;
  }

  mNativeImageSource->SetSource( source );
  Dali::Stage::GetCurrent().KeepRendering( 0.0f );
}

void Indicator::UpdateTopMargin()
{
  int newMargin = (mVisible == Dali::Window::VISIBLE && mOpacityMode == Dali::Window::OPAQUE) ? mImageHeight : 0;
  if (mTopMargin != newMargin)
  {
    mTopMargin = newMargin;
    mAdaptor->IndicatorSizeChanged( mTopMargin );
  }
}

void Indicator::UpdateVisibility()
{
  if( CheckVisibleState() )
  {
    // set default indicator type (enable the quick panel)
    OnIndicatorTypeChanged( INDICATOR_TYPE_1 );
  }
  else
  {
    // set default indicator type (disable the quick panel)
    OnIndicatorTypeChanged( INDICATOR_TYPE_2 );
  }

  if( !mIsShowing )
  {
    mIndicatorContentActor.SetPosition( 0.0f, -mImageHeight, 0.0f );
  }

  SetVisible(mVisible, true);
}

void Indicator::UpdateImageData( int bufferNumber )
{
  DALI_LOG_TRACE_METHOD_FMT( gIndicatorLogFilter, "State: %s  mVisible: %s", STATE_DEBUG_STRING(mState), mVisible?"T":"F" );

  if( mState == CONNECTED && mVisible )
  {
    if(mPixmap == 0)
    {
      // in case of shm indicator (not pixmap), not sure we can skip it when mIsShowing is false
      CopyToBuffer( bufferNumber );
    }
    else
    {
      if(mIsShowing)
      {
        mAdaptor->RequestUpdateOnce();
      }
    }
  }
}

bool Indicator::CopyToBuffer( int bufferNumber )
{
  bool success = false;

  if( mSharedFileInfo[bufferNumber].mLock )
  {
    Indicator::ScopedLock scopedLock(mSharedFileInfo[bufferNumber].mLock);
    if( mSharedFileInfo[bufferNumber].mLock->RetrieveAndClearErrorStatus() )
    {
      // Do nothing here.
    }
    else if( scopedLock.IsLocked() )
    {
      unsigned char *src = mSharedFileInfo[bufferNumber].mSharedFile->GetAddress();
      size_t size = mSharedFileInfo[bufferNumber].mImageWidth * mSharedFileInfo[bufferNumber].mImageHeight * 4;

      if( mIndicatorBuffer->UpdatePixels( src, size ) )
      {
        mAdaptor->RequestUpdateOnce();
        success = true;
      }
    }
  }

  return success;
}

void Indicator::LoadPixmapImage( Ecore_Ipc_Event_Server_Data *epcEvent )
{
}

void Indicator::CreateNewPixmapImage()
{
}

void Indicator::CreateNewImage( int bufferNumber )
{
  DALI_LOG_TRACE_METHOD_FMT( gIndicatorLogFilter, "W:%d H:%d", mSharedFileInfo[bufferNumber].mImageWidth, mSharedFileInfo[bufferNumber].mImageHeight );
  mIndicatorBuffer = new IndicatorBuffer( mAdaptor, mSharedFileInfo[bufferNumber].mImageWidth, mSharedFileInfo[bufferNumber].mImageHeight, Pixel::BGRA8888 );
  bool success = false;

  if( CopyToBuffer( bufferNumber ) ) // Only create images if we have valid image buffer
  {
    Dali::Texture texture = Dali::Texture::New( mIndicatorBuffer->GetNativeImage() );
    if( texture )
    {
      SetForegroundImage( texture );
      success = true;
    }
  }

  if( !success )
  {
    DALI_LOG_WARNING("### Cannot create indicator image - disconnecting ###\n");
    Disconnect();
    if( mObserver != NULL )
    {
      mObserver->IndicatorClosed( this );
    }
    // Don't do connection in this callback - strange things happen!
    StartReconnectionTimer();
  }
}

Dali::Geometry Indicator::CreateBackgroundGeometry()
{
  switch( mOpacityMode )
  {
    case Dali::Window::TRANSLUCENT:
      if( !mTranslucentGeometry )
      {
        // Construct 5 interval mesh
        // 0  +---+  1
        //    | \ |
        // 2  +---+  3
        //    | \ |
        // 4  +---+  5
        //    | \ |
        // 6  +---+  7
        //    | \ |
        // 8  +---+  9
        //    | \ |
        // 10 +---+  11

        // Create vertices
        struct BackgroundVertex
        {
          Vector2 mPosition;
          float   mAlpha;
        };

        unsigned int numVertices = 2 * ( NUM_GRADIENT_INTERVALS + 1 );
        BackgroundVertex vertices[ numVertices ];

        float d = -0.5f;
        float delta = 1.0f / NUM_GRADIENT_INTERVALS;
        BackgroundVertex* currentVertex = vertices;
        for( int y = 0; y < NUM_GRADIENT_INTERVALS + 1; ++y, d += delta )
        {
          currentVertex->mPosition = Vector2( -0.5f, d );
          currentVertex->mAlpha = GRADIENT_ALPHA[ y ];
          currentVertex++;

          currentVertex->mPosition = Vector2( 0.5f, d );
          currentVertex->mAlpha = GRADIENT_ALPHA[ y ];
          currentVertex++;
        }

        // Create indices
        unsigned int numIndices = 2 * 3 * NUM_GRADIENT_INTERVALS;
        unsigned short indices[ numIndices ];

        unsigned short* currentIndex = indices;
        for( int y = 0; y < NUM_GRADIENT_INTERVALS; ++y )
        {
          *currentIndex++ = (2 * y);
          *currentIndex++ = (2 * y) + 3;
          *currentIndex++ = (2 * y) + 1;

          *currentIndex++ = (2 * y);
          *currentIndex++ = (2 * y) + 2;
          *currentIndex++ = (2 * y) + 3;
        }

        Dali::Property::Map vertexFormat;
        vertexFormat[ "aPosition" ] = Dali::Property::VECTOR2;
        vertexFormat[ "aAlpha" ] = Dali::Property::FLOAT;
        Dali::PropertyBuffer vertexPropertyBuffer = Dali::PropertyBuffer::New( vertexFormat );
        vertexPropertyBuffer.SetData( vertices, numVertices );

        // Create the geometry object
        mTranslucentGeometry = Dali::Geometry::New();
        mTranslucentGeometry.AddVertexBuffer( vertexPropertyBuffer );
        mTranslucentGeometry.SetIndexBuffer( &indices[0], numIndices );
      }

      return mTranslucentGeometry;
    case Dali::Window::OPAQUE:

      if( !mSolidGeometry )
      {
        // Create vertices
        struct BackgroundVertex
        {
          Vector2 mPosition;
          float   mAlpha;
        };

        BackgroundVertex vertices[ 4 ] = { { Vector2( -0.5f, -0.5f ), 1.0f }, { Vector2( 0.5f, -0.5f ), 1.0f },
                                           { Vector2( -0.5f,  0.5f ), 1.0f }, { Vector2( 0.5f,  0.5f ), 1.0f } };

        // Create indices
        unsigned short indices[ 6 ] = { 0, 3, 1, 0, 2, 3 };

        Dali::Property::Map vertexFormat;
        vertexFormat[ "aPosition" ] = Dali::Property::VECTOR2;
        vertexFormat[ "aAlpha" ] = Dali::Property::FLOAT;
        Dali::PropertyBuffer vertexPropertyBuffer = Dali::PropertyBuffer::New( vertexFormat );
        vertexPropertyBuffer.SetData( vertices, 4 );


        // Create the geometry object
        mSolidGeometry = Dali::Geometry::New();
        mSolidGeometry.AddVertexBuffer( vertexPropertyBuffer );
        mSolidGeometry.SetIndexBuffer( &indices[0], 6 );
      }

      return mSolidGeometry;
    case Dali::Window::TRANSPARENT:
      break;
  }

  return Dali::Geometry();
}

void Indicator::SetForegroundImage( Dali::Texture texture )
{
  if( !mForegroundRenderer && texture )
  {
    // Create Shader
    Dali::Shader shader = Dali::Shader::New( FOREGROUND_VERTEX_SHADER, FOREGROUND_FRAGMENT_SHADER );

    // Create renderer from geometry and material
    Dali::Geometry quad = CreateQuadGeometry();
    mForegroundRenderer = Dali::Renderer::New( quad, shader );
    // Make sure the foreground stays in front of the background
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::DEPTH_INDEX, 1.f );

    // Set blend function
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::BLEND_FACTOR_SRC_RGB,    Dali::BlendFactor::ONE );
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::BLEND_FACTOR_DEST_RGB,   Dali::BlendFactor::ONE_MINUS_SRC_ALPHA );
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::BLEND_FACTOR_SRC_ALPHA,  Dali::BlendFactor::ONE );
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::BLEND_FACTOR_DEST_ALPHA, Dali::BlendFactor::ONE );

    // Create a texture-set and add to renderer

    Dali::TextureSet textureSet = Dali::TextureSet::New();
    textureSet.SetTexture( 0u, texture );
    mForegroundRenderer.SetTextures( textureSet );

    mIndicatorContentActor.AddRenderer( mForegroundRenderer );
  }
  else if( mForegroundRenderer )
  {
    Dali::TextureSet textureSet = mForegroundRenderer.GetTextures();
    textureSet.SetTexture( 0u, texture );
  }

  if( mImageWidth == 0 && mImageHeight == 0  && texture)
  {
    Resize( texture.GetWidth(), texture.GetHeight() );
  }
}

void Indicator::SetForegroundNativeImage( Dali::Image image )
{
  if( !mForegroundRenderer && image )
  {

    std::string fragmentShader = "#extension GL_OES_EGL_image_external:require\n";
    fragmentShader += "\n";
    fragmentShader += FOREGROUND_TBM_FRAGMENT_SHADER;

    // Create Shader
    Dali::Shader shader = Dali::Shader::New( FOREGROUND_VERTEX_SHADER, fragmentShader );

    // Create renderer from geometry and material
    Dali::Geometry quad = CreateQuadGeometry();
    mForegroundRenderer = Dali::Renderer::New( quad, shader );
    // Make sure the foreground stays in front of the background
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::DEPTH_INDEX, 1.f );

    // Set blend function
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::BLEND_FACTOR_SRC_RGB,    Dali::BlendFactor::ONE );
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::BLEND_FACTOR_DEST_RGB,   Dali::BlendFactor::ONE_MINUS_SRC_ALPHA );
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::BLEND_FACTOR_SRC_ALPHA,  Dali::BlendFactor::ONE );
    mForegroundRenderer.SetProperty( Dali::Renderer::Property::BLEND_FACTOR_DEST_ALPHA, Dali::BlendFactor::ONE );

    // Create a texture-set and add to renderer

    Dali::TextureSet textureSet = Dali::TextureSet::New();
    Dali::TextureSetImage( textureSet, 0u, image );

    mForegroundRenderer.SetTextures( textureSet );

    mIndicatorContentActor.AddRenderer( mForegroundRenderer );
  }
  else if( mForegroundRenderer )
  {
    Dali::TextureSet textureSet = mForegroundRenderer.GetTextures();
    Dali::TextureSetImage( textureSet, 0u, image );
  }

  if( mImageWidth == 0 && mImageHeight == 0  && image )
  {
    Resize( image.GetWidth(), image.GetHeight() );
  }
}

void Indicator::OnIndicatorTypeChanged( Type indicatorType )
{
  if( mObserver != NULL )
  {
    mObserver->IndicatorTypeChanged( indicatorType );
  }
}

void Indicator::DataReceived( void* event )
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );
  Ecore_Ipc_Event_Server_Data *epcEvent = static_cast<Ecore_Ipc_Event_Server_Data *>( event );

  switch( epcEvent->minor )
  {
    case OP_UPDATE:
    {
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_UPDATE\n" );
      if( mIsShowing )
      {
        mAdaptor->RequestUpdateOnce();
      }
      break;
    }
    case OP_UPDATE_DONE:
    {
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_UPDATE_DONE [%d]\n", epcEvent->response );
      // epcEvent->response == display buffer #
      UpdateImageData( epcEvent->response );
      break;
    }
    case OP_SHM_REF0:
    {
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_SHM_REF0\n" );
      SetSharedImageInfo( epcEvent );
      break;
    }
    case OP_SHM_REF1:
    {
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_SHM_REF1\n" );
      SetLockFileInfo( epcEvent );
      break;
    }
    case OP_SHM_REF2:
    {
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_SHM_REF2\n" );
      LoadSharedImage( epcEvent );
      break;
    }
    case OP_GL_REF:
    {
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_GL_REF\n" );
      SetupNativeIndicatorImage( epcEvent );
      break;
    }
    case OP_RESIZE:
    {
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_RESIZE\n" );

      if( (epcEvent->data) && (epcEvent->size >= (int)sizeof(IpcDataResize)) )
      {
        IpcDataResize *newSize = static_cast<IpcDataResize*>( epcEvent->data );
        Resize( newSize->w, newSize->h );
      }
      break;
    }
    case OP_MSG_PARENT:
    {
      int msgDomain = epcEvent->ref;
      int msgId = epcEvent->ref_to;

      void *msgData = NULL;
      int msgDataSize = 0;
      msgData = epcEvent->data;
      msgDataSize = epcEvent->size;

      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_MSG_PARENT. msgDomain = %d\n", msgDomain );

      if( msgDomain == MSG_DOMAIN_CONTROL_INDICATOR )
      {
        switch( msgId )
        {
          case MSG_ID_INDICATOR_TYPE:
          {
            DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_MSG_PARENT, INDICATOR_TYPE\n" );
            Type* indicatorType = static_cast<Type*>( epcEvent->data );
            OnIndicatorTypeChanged( *indicatorType );
            break;
          }

          case MSG_ID_INDICATOR_START_ANIMATION:
          {
            DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: MSG_ID_INDICATOR_START_ANIMATION\n" );

            if (msgDataSize != (int)sizeof(IpcIndicatorDataAnimation))
            {
              DALI_LOG_ERROR("Message data is incorrect\n");
              break;
            }

            IpcIndicatorDataAnimation *animData = static_cast<IpcIndicatorDataAnimation*>(msgData);

            if(!CheckVisibleState())
            {
              ShowIndicator( animData->duration /* n sec */ );
            }
            break;
          }
        }
      }
      break;
    }
  }
}

void Indicator::ConnectionClosed()
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  // Will get this callback if the server connection failed to start up.
  delete mServerConnection;
  mServerConnection = NULL;
  mState = DISCONNECTED;

  // Attempt to re-connect
  Connect();
}

bool Indicator::CheckVisibleState()
{
  if( mOrientation == Dali::Window::LANDSCAPE
    || mOrientation == Dali::Window::LANDSCAPE_INVERSE
    || (mVisible == Dali::Window::INVISIBLE)
    || (mVisible == Dali::Window::AUTO && !mIsShowing) )
  {
    return false;
  }

  return true;
}

void Indicator::ClearSharedFileInfo()
{
  for( int i = 0; i < SHARED_FILE_NUMBER; i++ )
  {
    delete mSharedFileInfo[i].mLock;
    mSharedFileInfo[i].mLock = NULL;

    delete mSharedFileInfo[i].mSharedFile;
    mSharedFileInfo[i].mSharedFile = NULL;

    mSharedFileInfo[i].mLockFileName.clear();
    mSharedFileInfo[i].mSharedFileName.clear();
  }
}

/**
 * duration can be this
 *
 * enum
 * {
 *  KEEP_SHOWING = -1,
 *  HIDE_NOW = 0
 * };
 */
void Indicator::ShowIndicator(float duration)
{
  if( !mIndicatorAnimation )
  {
    mIndicatorAnimation = Dali::Animation::New(SLIDING_ANIMATION_DURATION);
    mIndicatorAnimation.FinishedSignal().Connect(this, &Indicator::OnAnimationFinished);
  }

  if(mIsShowing && !EqualsZero(duration))
  {
    // If need to show during showing, do nothing.
    // In 2nd phase (below) will update timer
  }
  else if(!mIsShowing && mIsAnimationPlaying && EqualsZero(duration))
  {
    // If need to hide during hiding or hidden already, do nothing
  }
  else
  {
    mIndicatorAnimation.Clear();

    if( EqualsZero(duration) )
    {
      mIndicatorAnimation.AnimateTo( Property( mIndicatorContentActor, Dali::Actor::Property::POSITION ), Vector3(0, -mImageHeight, 0), Dali::AlphaFunction::EASE_OUT );

      mIsShowing = false;

      OnIndicatorTypeChanged( INDICATOR_TYPE_2 ); // un-toucable
    }
    else
    {
      mIndicatorAnimation.AnimateTo( Property( mIndicatorContentActor, Dali::Actor::Property::POSITION ), Vector3(0, 0, 0), Dali::AlphaFunction::EASE_OUT );

      mIsShowing = true;

      OnIndicatorTypeChanged( INDICATOR_TYPE_1 ); // touchable
    }

    mIndicatorAnimation.Play();
    mIsAnimationPlaying = true;
  }

  if(duration > 0)
  {
    if(!mShowTimer)
    {
      mShowTimer = Dali::Timer::New(1000 * duration);
      mShowTimer.TickSignal().Connect(this, &Indicator::OnShowTimer);
    }
    mShowTimer.SetInterval(1000* duration);
    mShowTimer.Start();

    if( mVisible == Dali::Window::AUTO )
    {
      // check the stage touch
      Dali::Stage::GetCurrent().TouchSignal().Connect( this, &Indicator::OnStageTouched );
    }
  }
  else
  {
    if(mShowTimer && mShowTimer.IsRunning())
    {
      mShowTimer.Stop();
    }

    if( mVisible == Dali::Window::AUTO )
    {
      // check the stage touch
      Dali::Stage::GetCurrent().TouchSignal().Disconnect( this, &Indicator::OnStageTouched );
    }
  }
}

bool Indicator::OnShowTimer()
{
  // after time up, hide indicator
  ShowIndicator( HIDE_NOW );

  return false;
}

void Indicator::OnAnimationFinished(Dali::Animation& animation)
{
  mIsAnimationPlaying = false;
  // once animation is finished and indicator is hidden, take it off stage
  if( mObserver != NULL )
  {
    mObserver->IndicatorVisibilityChanged( mIsShowing ); // is showing?
  }
}

void Indicator::OnPan( Dali::Actor actor, const Dali::PanGesture& gesture )
{
  // Nothing to do, but we still want to consume pan
}

void Indicator::OnStageTouched(const Dali::TouchData& touchData)
{
  // when stage is touched while indicator is showing temporary, hide it
  if( mIsShowing && ( CheckVisibleState() == false || mVisible == Dali::Window::AUTO ) )
  {
    switch( touchData.GetState( 0 ) )
    {
      case Dali::PointState::DOWN:
      {
        // if touch point is inside the indicator, indicator is not hidden
        if( mImageHeight < int( touchData.GetScreenPosition( 0 ).y ) )
        {
          ShowIndicator( HIDE_NOW );
        }
        break;
      }

      default:
      break;
    }
  }
}

} // Adaptor
} // Internal
} // Dali
