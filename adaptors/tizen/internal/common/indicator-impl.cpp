/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "indicator-impl.h"

// EXTERNAL INCLUDES
#include <Ecore.h>
#include <Evas.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <dali/public-api/images/bitmap-image.h>
#include <dali/public-api/adaptor-framework/common/pixmap-image.h>
#include <dali/public-api/actors/image-actor.h>
#include <dali/public-api/events/touch-event.h>
#include <dali/public-api/events/touch-point.h>
#include <dali/public-api/common/stage.h>
#include <dali/public-api/actors/blending.h>
#include <dali/public-api/shader-effects/shader-effect.h>

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <internal/common/adaptor-impl.h>
#include <internal/common/accessibility-manager-impl.h>

using Dali::Vector4;

#if defined(DEBUG_ENABLED)
#define STATE_DEBUG_STRING(state) (state==DISCONNECTED?"DISCONNECTED":state==CONNECTED?"CONNECTED":"UNKNOWN")
#endif

namespace
{

const float SLIDING_ANIMATION_DURATION( 0.2f ); // 200 milli seconds
const float AUTO_INDICATOR_STAY_DURATION(3.0f); // 3 seconds
const float SHOWING_DISTANCE_HEIGHT_RATE(0.17f); // 10 pixels

enum
{
  KEEP_SHOWING = -1,
  HIDE_NOW = 0
};

const int NUM_GRADIENT_INTERVALS(5); // Number of gradient intervals
const Dali::Vector4 GRADIENT_COLORS[NUM_GRADIENT_INTERVALS+1] =
{
  Vector4(0.0f, 0.0f, 0.0f, 0.6f),
  Vector4(0.0f, 0.0f, 0.0f, 0.38f),
  Vector4(0.0f, 0.0f, 0.0f, 0.20f),
  Vector4(0.0f, 0.0f, 0.0f, 0.08f),
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),
};

const float OPAQUE_THRESHOLD(0.99f);
const float TRANSPARENT_THRESHOLD(0.05f);

// Indicator orientation
const char* ELM_INDICATOR_PORTRAIT("elm_indicator_portrait");
const char* ELM_INDICATOR_LANDSCAPE("elm_indicator_landscape");
const char* ELM_INDICATOR_PORTRAIT_FIXED_COLOR_STYLE("elm_indicator_portrait_fixed");
const char* ELM_INDICATOR_LANDSCAPE_FIXED_COLOR_STYLE("elm_indicator_landscape_fixed");

const char* MESH_VERTEX_SHADER =
"attribute lowp vec3     aColor;\n"
"varying   mediump vec4  vColor;\n"
"void main()\n"
"{\n"
"  gl_Position = uMvpMatrix * vec4(aPosition, 1.0);\n"
"  vColor = vec4(aColor.r, aColor.g, aColor.b, aTexCoord.x);\n"
"}\n";

const char* MESH_FRAGMENT_SHADER =
"varying mediump vec4  vColor;\n"
"void main()\n"
"{\n"
"  gl_FragColor = vColor*uColor;\n"
"}\n";

// Copied from elm_win.h

/**
 * Defines the type modes of indicator that can be shown
 * If the indicator can support several type of indicator,
 * you can use this enum value to deal with different type of indicator
 */
typedef enum
{
   ELM_WIN_INDICATOR_TYPE_UNKNOWN, /**< Unknown indicator type mode */
   ELM_WIN_INDICATOR_TYPE_1, /**< Type 0 the the indicator */
   ELM_WIN_INDICATOR_TYPE_2, /**< Type 1 the indicator */
} Elm_Win_Indicator_Type_Mode;

// Copied from ecore_evas_extn.c

enum // opcodes
{
  OP_RESIZE,
  OP_SHOW,
  OP_HIDE,
  OP_FOCUS,
  OP_UNFOCUS,
  OP_UPDATE,
  OP_UPDATE_DONE,
  OP_LOCK_FILE,
  OP_SHM_REF,
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
  OP_PIXMAP_REF
};

// Copied from elm_conform.c

const int MSG_DOMAIN_CONTROL_INDICATOR(0x10001);
const int MSG_ID_INDICATOR_REPEAT_EVENT(0x10002);
const int MSG_ID_INDICATOR_ROTATION(0x10003);
const int MSG_ID_INDICATOR_OPACITY(0X1004);
const int MSG_ID_INDICATOR_TYPE(0X1005);
const int MSG_ID_INDICATOR_START_ANIMATION(0X10006);

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

  IpcDataEvMouseMove(const Dali::TouchPoint& touchPoint, unsigned long timestamp)
  : x(static_cast<Evas_Coord>(touchPoint.local.x)),
    y(static_cast<Evas_Coord>(touchPoint.local.y)),
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

void SetMeshDataColors(Dali::AnimatableMesh mesh, const Vector4 (&colors)[NUM_GRADIENT_INTERVALS+1])
{
  for( size_t i=0; i<NUM_GRADIENT_INTERVALS+1; i++ )
  {
    int j=i*2;
    mesh[j].SetColor(colors[i]);
    mesh[j+1].SetColor(colors[i]);
    mesh[j].SetTextureCoords(Dali::Vector2(colors[i].a, colors[i].a));
    mesh[j+1].SetTextureCoords(Dali::Vector2(colors[i].a, colors[i].a));
  }
}

void SetMeshDataColors(Dali::AnimatableMesh mesh, const Vector4& color)
{
  for( size_t i=0, length=NUM_GRADIENT_INTERVALS+1 ; i<length; i++ )
  {
    int j=i*2;
    mesh[j].SetColor(color);
    mesh[j+1].SetColor(color);
    mesh[j].SetTextureCoords(Dali::Vector2(color.a, color.a));
    mesh[j+1].SetTextureCoords(Dali::Vector2(color.a, color.a));
  }
}

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


Indicator::LockFile::LockFile(const char* filename)
: mFilename(filename),
  mErrorThrown(false)
{
  mFileDescriptor = open(filename, O_RDWR);
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
    if( lockf( mFileDescriptor, F_LOCK, 0 ) == 0 ) // Note, operation may block.
    {
      locked = true;
    }
    else
    {
      if( errno == EBADF )
      {
        // file descriptor is no longer valid or not writable
        mFileDescriptor = 0;
        mErrorThrown = true;
        DALI_LOG_ERROR( "### Cannot lock indicator: bad file descriptor for %s ###\n", mFilename.c_str() );
      }
    }
  }

  return locked;
}

void Indicator::LockFile::Unlock()
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );
  if( lockf( mFileDescriptor, F_ULOCK, 0 ) != 0 )
  {
    if( errno == EBADF )
    {
      // file descriptor is no longer valid or not writable
      mFileDescriptor = 0;
      mErrorThrown = true;
      DALI_LOG_ERROR( "### Cannot unlock indicator: bad file descriptor for %s\n", mFilename.c_str() );
    }
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

Indicator::Indicator( Adaptor* adaptor, Dali::Window::WindowOrientation orientation, Dali::Window::IndicatorStyle style, Observer* observer )
: mPixmap( 0 ),
  mConnection( this ),
  mStyle( style ),
  mOpacityMode( Dali::Window::OPAQUE ),
  mState( DISCONNECTED ),
  mAdaptor(adaptor),
  mServerConnection( NULL ),
  mLock( NULL ),
  mSharedFile( NULL ),
  mObserver( observer ),
  mOrientation( orientation ),
  mRotation( 0 ),
  mImageWidth( 0 ),
  mImageHeight( 0 ),
  mVisible( Dali::Window::VISIBLE ),
  mIsShowing( true ),
  mIsAnimationPlaying( false ),
  mTouchedDown( false )
{
  mIndicatorImageActor = Dali::ImageActor::New();
  mIndicatorImageActor.SetBlendFunc( Dali::BlendingFactor::ONE, Dali::BlendingFactor::ONE_MINUS_SRC_ALPHA,
                                    Dali::BlendingFactor::ONE, Dali::BlendingFactor::ONE );

  mIndicatorImageActor.SetParentOrigin( ParentOrigin::TOP_CENTER );
  mIndicatorImageActor.SetAnchorPoint( AnchorPoint::TOP_CENTER );

  SetBackground();
  mBackgroundActor.SetParentOrigin( ParentOrigin::TOP_CENTER );
  mBackgroundActor.SetAnchorPoint( AnchorPoint::TOP_CENTER );
  mBackgroundActor.SetZ(-0.01f);

  // add background to image actor to move it with indicator image
  mIndicatorImageActor.Add( mBackgroundActor );

  mIndicatorActor = Dali::Actor::New();
  mIndicatorActor.Add( mIndicatorImageActor );

  if( mOrientation == Dali::Window::LANDSCAPE || mOrientation == Dali::Window::LANDSCAPE_INVERSE )
  {
    mBackgroundActor.SetVisible( false );
  }

  // event handler
  mEventActor = Dali::Actor::New();
  mEventActor.SetParentOrigin( ParentOrigin::TOP_CENTER );
  mEventActor.SetAnchorPoint( AnchorPoint::TOP_CENTER );
  mEventActor.SetPosition(0.0f, 0.0f, 1.0f);
  mEventActor.TouchedSignal().Connect( this, &Indicator::OnTouched );
  mEventActor.SetLeaveRequired( true );
  mIndicatorActor.Add( mEventActor );

  Open( orientation );

  // register indicator to accessibility manager
  Dali::AccessibilityManager accessibilityManager = AccessibilityManager::Get();
  if(accessibilityManager)
  {
    AccessibilityManager::GetImplementation( accessibilityManager ).SetIndicator( this );
  }
}

Indicator::~Indicator()
{
  if(mEventActor)
  {
    mEventActor.TouchedSignal().Disconnect( this, &Indicator::OnTouched );
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

  Connect( orientation );

  // Change background visibility depending on orientation
  if(mOrientation == Dali::Window::PORTRAIT || mOrientation == Dali::Window::PORTRAIT_INVERSE)
  {
    mBackgroundActor.SetVisible(true);
  }
  else
  {
    mBackgroundActor.SetVisible(false);
  }
}

void Indicator::Close()
{
  DALI_LOG_TRACE_METHOD_FMT( gIndicatorLogFilter, "State: %s\n", STATE_DEBUG_STRING(mState) );

  if( mState == CONNECTED )
  {
    Disconnect();
    if( mObserver != NULL )
    {
      mObserver->IndicatorClosed( this );
    }
  }

  Dali::Image emptyImage;
  mIndicatorImageActor.SetImage(emptyImage);
}

void Indicator::SetOpacityMode( Dali::Window::IndicatorBgOpacity mode )
{
  mOpacityMode = mode;
  SetBackground();
}

void Indicator::SetVisible( Dali::Window::IndicatorVisibleMode visibleMode, bool forceUpdate )
{
  if ( visibleMode != mVisible || forceUpdate )
  {
    // If we were previously hidden, then we should update the image data before we display the indicator
    if ( mVisible == Dali::Window::INVISIBLE )
    {
      UpdateImageData();
    }

    mVisible = visibleMode;

    if( mIndicatorImageActor.GetImage() )
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

bool Indicator::OnTouched(Dali::Actor indicator, const Dali::TouchEvent& touchEvent)
{
  if(mServerConnection)
  {
    const TouchPoint& touchPoint = touchEvent.GetPoint( 0 );

    // Send touch event to indicator server when indicator is showing
    if( CheckVisibleState() || mIsShowing )
    {
      switch( touchPoint.state )
      {
        case Dali::TouchPoint::Down:
        {
          IpcDataEvMouseMove ipcMove( touchPoint, touchEvent.time );
          IpcDataEvMouseDown ipcDown( touchEvent.time );
          mServerConnection->SendEvent( OP_EV_MOUSE_MOVE, &ipcMove, sizeof(ipcMove) );
          mServerConnection->SendEvent( OP_EV_MOUSE_DOWN, &ipcDown, sizeof(ipcDown) );

          if( mVisible == Dali::Window::AUTO )
          {
            // Stop hiding indicator
            ShowIndicator( KEEP_SHOWING );
            mEventActor.SetSize( Dali::Stage::GetCurrent().GetSize() );
          }
        }
        break;

        case Dali::TouchPoint::Motion:
        {
          IpcDataEvMouseMove ipcMove( touchPoint, touchEvent.time );
          mServerConnection->SendEvent( OP_EV_MOUSE_MOVE, &ipcMove, sizeof(ipcMove) );
        }
        break;

        case Dali::TouchPoint::Up:
        {
          IpcDataEvMouseUp ipcUp( touchEvent.time );
          mServerConnection->SendEvent( OP_EV_MOUSE_UP, &ipcUp, sizeof(ipcUp) );

          if( mVisible == Dali::Window::AUTO )
          {
            // Hide indicator
            ShowIndicator( 0.5f /* hide after 0.5 sec */ );
            // TODO: not necessary if dali supports the event for both indicator and behind button
            mEventActor.SetSize( mImageWidth, mImageHeight / 2 );
          }
        }
        break;

        case Dali::TouchPoint::Leave:
        {
          IpcDataEvMouseMove ipcMove( touchPoint, touchEvent.time );
          mServerConnection->SendEvent( OP_EV_MOUSE_MOVE, &ipcMove, sizeof(ipcMove) );
          IpcDataEvMouseUp ipcOut( touchEvent.time );
          mServerConnection->SendEvent( OP_EV_MOUSE_OUT, &ipcOut, sizeof(ipcOut) );
        }
        break;

        default:
          break;
      }
    }
    // show indicator when it is invisible
    else if( !mIsShowing && ( CheckVisibleState() == false && mVisible == Dali::Window::AUTO ) )
    {
      switch( touchPoint.state )
      {
        case Dali::TouchPoint::Down:
        {
          mTouchedDown = true;
          mTouchDownPosition = touchPoint.local;

        }
        break;

        case Dali::TouchPoint::Motion:
        case Dali::TouchPoint::Up:
        case Dali::TouchPoint::Leave:
        {
          if( mTouchedDown )
          {
            float moveDistance = sqrt( (mTouchDownPosition.x - touchPoint.local.x) * (mTouchDownPosition.x - touchPoint.local.x)
                               + (mTouchDownPosition.y - touchPoint.local.y)*(mTouchDownPosition.y - touchPoint.local.y) );

            if( moveDistance > 2 * (mImageHeight * SHOWING_DISTANCE_HEIGHT_RATE) /*threshold for distance*/
              && touchPoint.local.y - mTouchDownPosition.y > mImageHeight * SHOWING_DISTANCE_HEIGHT_RATE /*threshold for y*/ )
            {
              ShowIndicator( AUTO_INDICATOR_STAY_DURATION );
              mTouchedDown = false;
            }
          }
        }
        break;

        default:
          break;
      }
    }
  }

  return true;
}

/**
 * Return the current orientation in degrees
 * @return value of 0, 90, 180 or 270
 */
int Indicator::OrientationToDegrees( Dali::Window::WindowOrientation orientation )
{
  int degree = 0;

  switch( orientation )
  {
    case Dali::Window::PORTRAIT:
      degree = 0;
      break;
    case Dali::Window::PORTRAIT_INVERSE:
      degree = 180;
      break;
    case Dali::Window::LANDSCAPE:
      degree = 90;
      break;
    case Dali::Window::LANDSCAPE_INVERSE:
      degree = 270;
      break;
  }
  return degree;
}

bool Indicator::Connect( Dali::Window::WindowOrientation orientation )
{
  DALI_ASSERT_DEBUG( mState == DISCONNECTED );

  bool connected = false;
  mOrientation = orientation;
  mRotation = OrientationToDegrees(mOrientation);

  switch( orientation )
  {
    case Dali::Window::PORTRAIT:
      if(mStyle == Dali::Window::FIXED_COLOR)
      {
        connected = Connect( ELM_INDICATOR_PORTRAIT_FIXED_COLOR_STYLE );
      }
      else
      {
        connected = Connect( ELM_INDICATOR_PORTRAIT );
      }
      break;
    case Dali::Window::PORTRAIT_INVERSE:
      if(mStyle == Dali::Window::FIXED_COLOR)
      {
        connected = Connect( ELM_INDICATOR_PORTRAIT_FIXED_COLOR_STYLE );
      }
      else
      {
        connected = Connect( ELM_INDICATOR_PORTRAIT );
      }
      break;
    case Dali::Window::LANDSCAPE:
      if(mStyle == Dali::Window::FIXED_COLOR)
      {
        connected = Connect( ELM_INDICATOR_LANDSCAPE_FIXED_COLOR_STYLE );
      }
      else
      {
        connected = Connect( ELM_INDICATOR_LANDSCAPE );
      }
      break;
    case Dali::Window::LANDSCAPE_INVERSE:
      if(mStyle == Dali::Window::FIXED_COLOR)
      {
        connected = Connect( ELM_INDICATOR_LANDSCAPE_FIXED_COLOR_STYLE );
      }
      else
      {
        connected = Connect( ELM_INDICATOR_LANDSCAPE );
      }
      break;
  }

  return connected;
}

bool Indicator::Connect( const char *serviceName )
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  bool connected = false;

  mServerConnection = new ServerConnection( serviceName, 0, false, this );
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
    if( ! Connect( mOrientation ) )
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

  delete mLock;
  mLock = NULL;

  delete mSharedFile;
  mSharedFile = NULL;

  delete mServerConnection;
  mServerConnection = NULL;
}

void Indicator::NewLockFile( Ecore_Ipc_Event_Server_Data *epcEvent )
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  delete mLock;
  mLock = NULL;

  if ( (epcEvent->data) &&
       (epcEvent->size > 0) &&
       (((unsigned char *)epcEvent->data)[epcEvent->size - 1] == 0) )
  {
    const char* lockFile = static_cast< const char* >( epcEvent->data );
    mLock = new Indicator::LockFile( lockFile );
    if( mLock->RetrieveAndClearErrorStatus() )
    {
      DALI_LOG_ERROR( "### Indicator error: Cannot open lock file %s ###\n", lockFile );
    }
  }
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

    // We don't currently handle the pixel buffer size being changed. Create a new image instead
    if( mSharedFile )
    {
      CreateNewImage();
    }
  }
}

void Indicator::LoadPixmapImage( Ecore_Ipc_Event_Server_Data *epcEvent )
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  // epcEvent->ref == w
  // epcEvent->ref_to == h
  // epcEvent->response == alpha
  // epcEvent->data = pixmap id
  if( ( epcEvent->data ) &&
      (epcEvent->size >= (int)sizeof(PixmapId)) )
  {
    if( mSharedFile != NULL )
    {
      delete mSharedFile;
      mSharedFile = NULL;
    }

    if( (epcEvent->ref > 0) && (epcEvent->ref_to > 0) )
    {
      mImageWidth  = epcEvent->ref;
      mImageHeight = epcEvent->ref_to;

      mPixmap = *(static_cast<PixmapId*>(epcEvent->data));
      CreateNewPixmapImage();

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

      SetVisible(mVisible, true);
    }
  }
}

void Indicator::LoadSharedImage( Ecore_Ipc_Event_Server_Data *epcEvent )
{
  DALI_LOG_TRACE_METHOD( gIndicatorLogFilter );

  // epcEvent->ref == w
  // epcEvent->ref_to == h
  // epcEvent->response == alpha
  // epcEvent->data = shm ref string + nul byte
  if( ( epcEvent->data ) &&
      ( ( unsigned char * ) epcEvent->data)[ epcEvent->size - 1 ] == 0 )
  {
    if( mSharedFile != NULL )
    {
      delete mSharedFile;
      mSharedFile = NULL;
    }

    if( (epcEvent->ref > 0) && (epcEvent->ref_to > 0) )
    {
      mImageWidth  = epcEvent->ref;
      mImageHeight = epcEvent->ref_to;

      char* sharedFilename = static_cast<char*>(epcEvent->data);

      mSharedFile = SharedFile::New( sharedFilename, mImageWidth * mImageWidth * 4, true );
      if( mSharedFile != NULL )
      {
        CreateNewImage();
        mEventActor.SetSize(mImageWidth, mImageHeight);

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

        SetVisible(mVisible, true);
      }
    }
  }
}

void Indicator::UpdateImageData()
{
  DALI_LOG_TRACE_METHOD_FMT( gIndicatorLogFilter, "State: %s  mVisible: %s\n", STATE_DEBUG_STRING(mState), mVisible?"T":"F" );

  if( mState == CONNECTED && mVisible )
  {
    if(mPixmap == 0)
    {
      CopyToBuffer();
    }
    else
    {
      mAdaptor->RequestUpdateOnce();
    }
  }
}

bool Indicator::CopyToBuffer()
{
  bool success = false;

  if( mLock )
  {
    Indicator::ScopedLock scopedLock(mLock);
    if( mLock->RetrieveAndClearErrorStatus() )
    {
      // Do nothing here.
    }
    else if( scopedLock.IsLocked() )
    {
      unsigned char *src = mSharedFile->GetAddress();
      size_t size = mImageWidth * mImageHeight * 4;

      if( mIndicatorBuffer->UpdatePixels( src, size ) )
      {
        mAdaptor->RequestUpdateOnce();
        success = true;
      }
    }
  }

  return success;
}

void Indicator::SetBackground()
{
  if( ! mBackgroundActor )
  {
    ConstructBackgroundMesh();
  }

  switch( mOpacityMode )
  {
    case Dali::Window::TRANSLUCENT:
    {
      SetMeshDataColors( mBackgroundMesh, GRADIENT_COLORS );
    }
    break;

    case Dali::Window::TRANSPARENT:
    {
      SetMeshDataColors( mBackgroundMesh, Color::TRANSPARENT );
    }
    break;

    case Dali::Window::OPAQUE:
    default :
    {
      SetMeshDataColors( mBackgroundMesh, Color::BLACK );
    }
    break;
  }
}

void Indicator::CreateNewPixmapImage()
{
  DALI_LOG_TRACE_METHOD_FMT( gIndicatorLogFilter, "W:%d H:%d\n", mImageWidth, mImageHeight );
  Dali::PixmapImagePtr pixmapImage = Dali::PixmapImage::New(mPixmap, Dali::Adaptor::Get());

  if( pixmapImage )
  {
    mIndicatorImageActor.SetImage( Dali::Image::New(*pixmapImage) );
    mIndicatorImageActor.SetSize( mImageWidth, mImageHeight );
    mIndicatorActor.SetSize( mImageWidth, mImageHeight );
    mEventActor.SetSize(mImageWidth, mImageHeight);

    SetBackground();
    if( mBackgroundActor )
    {
      mBackgroundActor.SetSize( mImageWidth, mImageHeight );
    }
  }
  else
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

void Indicator::CreateNewImage()
{
  DALI_LOG_TRACE_METHOD_FMT( gIndicatorLogFilter, "W:%d H:%d\n", mImageWidth, mImageHeight );
  mIndicatorBuffer = new IndicatorBuffer( mAdaptor, mImageWidth, mImageHeight, Pixel::BGRA8888 );
  Dali::Image image = Dali::Image::New( mIndicatorBuffer->GetNativeImage() );

  if( CopyToBuffer() ) // Only create images if we have valid image buffer
  {
    mIndicatorImageActor.SetImage( image );
    mIndicatorImageActor.SetSize( mImageWidth, mImageHeight );
    mIndicatorActor.SetSize( mImageWidth, mImageHeight );
    mEventActor.SetSize(mImageWidth, mImageHeight);

    SetBackground();
    if( mBackgroundActor )
    {
      mBackgroundActor.SetSize( mImageWidth, mImageHeight );
    }
  }
  else
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
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_UPDATE\n" );
      if(mPixmap != 0)
      {
        mAdaptor->RequestUpdateOnce();
      }
      break;

    case OP_UPDATE_DONE:
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_UPDATE_DONE\n" );
      UpdateImageData();
      break;

    case OP_LOCK_FILE:
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_LOCK_FILE\n" );
      NewLockFile( epcEvent );
      break;

    case OP_SHM_REF:
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_SHM_REF\n" );
      LoadSharedImage( epcEvent );
      break;

    case OP_PIXMAP_REF:
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_PIXMAP_REF\n" );
      LoadPixmapImage( epcEvent );
      break;

    case OP_RESIZE:
      DALI_LOG_INFO( gIndicatorLogFilter, Debug::General, "Indicator client received: OP_RESIZE\n" );

      if( (epcEvent->data) && (epcEvent->size >= (int)sizeof(IpcDataResize)) )
      {
        IpcDataResize *newSize = static_cast<IpcDataResize*>( epcEvent->data );
        Resize( newSize->w, newSize->h );
      }
      break;

    case OP_MSG_PARENT:
    {
      int msgDomain = epcEvent->ref;
      int msgId = epcEvent->ref_to;

      void *msgData = NULL;
      int msgDataSize = 0;
      msgData = epcEvent->data;
      msgDataSize = epcEvent->size;

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
            if (msgDataSize != (int)sizeof(IpcIndicatorDataAnimation))
            {
              DALI_LOG_ERROR("Message data is incorrect");
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
  Connect(mOrientation);
}

bool Indicator::CheckVisibleState()
{
  if( mOrientation == Dali::Window::LANDSCAPE
    || mOrientation == Dali::Window::LANDSCAPE_INVERSE
    || (mVisible != Dali::Window::VISIBLE) )
  {
    return false;
  }

  return true;
}

void Indicator::ConstructBackgroundMesh()
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
  Dali::AnimatableMesh::Faces faces;
  faces.reserve(NUM_GRADIENT_INTERVALS * 6); // 2 tris per interval
  for(int i=0; i<NUM_GRADIENT_INTERVALS; i++)
  {
    int j=i*2;
    faces.push_back(j); faces.push_back(j+3); faces.push_back(j+1);
    faces.push_back(j); faces.push_back(j+2); faces.push_back(j+3);
  }

  mBackgroundMesh = Dali::AnimatableMesh::New((NUM_GRADIENT_INTERVALS+1)*2, faces);
  float interval=1.0f / (float)NUM_GRADIENT_INTERVALS;
  for(int i=0;i<NUM_GRADIENT_INTERVALS+1;i++)
  {
    int j=i*2;
    mBackgroundMesh[j  ].SetPosition(Vector3(-0.5f, -0.5f+(interval*(float)i), 0.0f));
    mBackgroundMesh[j+1].SetPosition(Vector3( 0.5f, -0.5f+(interval*(float)i), 0.0f));
  }

  mBackgroundActor = Dali::MeshActor::New(mBackgroundMesh);
  mBackgroundActor.SetAffectedByLighting(false);
  Dali::ShaderEffect shaderEffect = Dali::ShaderEffect::New( MESH_VERTEX_SHADER, MESH_FRAGMENT_SHADER,
                                                             GEOMETRY_TYPE_MESH, // Using vertex color
                                                             Dali::ShaderEffect::HINT_BLENDING );
  mBackgroundActor.SetShaderEffect(shaderEffect);
}

void Indicator::ShowIndicator(float duration)
{
  if( !mIndicatorAnimation )
  {
    mIndicatorAnimation = Dali::Animation::New(SLIDING_ANIMATION_DURATION);
    mIndicatorAnimation.FinishedSignal().Connect(this, &Indicator::OnAnimationFinished);
  }

  if(mIsShowing && duration != 0)
  {
    // do not need to show it again
  }
  else if(!mIsShowing && mIsAnimationPlaying && duration == 0)
  {
    // do not need to hide it again
  }
  else
  {
    if(duration == 0)
    {
      mIndicatorAnimation.MoveTo(mIndicatorImageActor, Vector3(0, -mImageHeight, 0), Dali::AlphaFunctions::EaseOut);

      mIsShowing = false;

      OnIndicatorTypeChanged( INDICATOR_TYPE_2 ); // un-toucable
    }
    else
    {
      mIndicatorAnimation.MoveTo(mIndicatorImageActor, Vector3(0, 0, 0), Dali::AlphaFunctions::EaseOut);

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
        Dali::Stage::GetCurrent().TouchedSignal().Connect( this, &Indicator::OnStageTouched );
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
        Dali::Stage::GetCurrent().TouchedSignal().Disconnect( this, &Indicator::OnStageTouched );
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

  if( mIsShowing == false )
  {
    // TODO: not necessary if dali supports the event for both indicator and behind button
    mEventActor.SetSize(mImageWidth, mImageHeight /2);
  }
  else
  {
    mEventActor.SetSize(mImageWidth, mImageHeight);
  }
}

void Indicator::OnStageTouched(const Dali::TouchEvent& touchEvent)
{
  const TouchPoint& touchPoint = touchEvent.GetPoint( 0 );

  // when stage is touched while indicator is showing temporary, hide it
  if( mIsShowing && ( CheckVisibleState() == false || mVisible == Dali::Window::AUTO ) )
  {
    switch( touchPoint.state )
    {
      case Dali::TouchPoint::Down:
      {
        ShowIndicator( HIDE_NOW );
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
