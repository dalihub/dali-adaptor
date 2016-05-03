#ifndef __DALI_INTERNAL_ECORE_INDICATOR_H__
#define __DALI_INTERNAL_ECORE_INDICATOR_H__

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

// EXTERNAL INCLUDES
#include <dali/public-api/animation/animation.h>
#include <dali/public-api/actors/image-actor.h>
#include <dali/public-api/events/pan-gesture.h>
#include <dali/public-api/events/pan-gesture-detector.h>
#include <dali/devel-api/rendering/renderer.h>

// INTERNAL INCLUDES
#include <base/interfaces/indicator-interface.h>
#include <indicator-buffer.h>
#include <ecore-server-connection.h>
#include <shared-file.h>
#include <timer.h>
#include <window.h>

namespace Dali
{
namespace Integration
{
class Core;
}

namespace Internal
{
namespace Adaptor
{
class Adaptor;

typedef unsigned int PixmapId;

/**
 * The Indicator class connects to the indicator server, and gets and draws the indicator
 * for the given orientation.
 */
class Indicator : public ConnectionTracker, public ServerConnection::Observer, public IndicatorInterface
{
public:

  enum State
  {
    DISCONNECTED,
    CONNECTED
  };


protected:
  /**
   * Class to encapsulate lock file
   */
  class LockFile
  {
  public:
    /**
     * Constructor. open lock file
     */
    LockFile(const std::string filename);

    /**
     * Close lock file
     */
    ~LockFile();

    /**
     * Grab an exclusive lock on this file
     * @return true if the lock succeeded, false if it failed
     */
    bool Lock();

    /**
     * Remove the lock
     */
    void Unlock();

    /**
     * Test if there is an error with the lock file, and clears
     * the error flag;
     * @return true if an error was thrown
     */
    bool RetrieveAndClearErrorStatus();

  private:
    std::string mFilename;
    int         mFileDescriptor;
    bool        mErrorThrown;
  };

  /**
   * Class to ensure lock/unlock through object destruction
   */
  class ScopedLock
  {
  public:
    /**
     * Constructor - creates a lock on the lockfile
     * @param[in] lockFile The lockfile to use
     */
    ScopedLock( LockFile* lockFile );

    /**
     * Destructor - removes the lock (if any) on the lockfile
     */
    ~ScopedLock();

    /**
     * Method to test if the locking succeeded
     * @return TRUE if locked
     */
    bool IsLocked();

  private:
    LockFile* mLockFile; ///< The lock file to use
    bool      mLocked;   ///< Whether the lock succeeded
  };


public:  // Dali::Internal::Adaptor::IndicicatorInterface
  /**
   * @copydoc Dali::Internal::IndicatorInterface::IndicatorInterface
   */
  Indicator( Adaptor* adaptor,
             Dali::Window::WindowOrientation orientation,
             IndicatorInterface::Observer* observer );

  /**
   * @copydoc Dali::Internal::IndicatorInterface::~IndicatorInterface
   */
  virtual ~Indicator();


  virtual void SetAdaptor(Adaptor* adaptor);

  /**
   * @copydoc Dali::Internal::IndicatorInterface::GetActor
   */
  virtual Dali::Actor GetActor();

  /**
   * @copydoc Dali::Internal::IndicatorInterface::Open
   */
  virtual void Open( Dali::Window::WindowOrientation orientation );

  /**
   * @copydoc Dali::Internal::IndicatorInterface::Close
   */
  virtual void Close();

  /**
   * @copydoc Dali::Internal::IndicatorInterface::SetOpacityMode
   */
  virtual void SetOpacityMode( Dali::Window::IndicatorBgOpacity mode );

  /**
   * @copydoc Dali::Internal::IndicatorInterface::SetVisible
   */
  virtual void SetVisible( Dali::Window::IndicatorVisibleMode visibleMode, bool forceUpdate = false );

  /**
   * @copydoc Dali::Internal::IndicatorInterface::IsConnected
   */
  virtual bool IsConnected();

  /**
   * @copydoc Dali::Internal::IndicatorInterface::SendMessage
   */
  virtual bool SendMessage( int messageDomain, int messageId, const void *data, int size );

private:
  /**
   * Initialize the indicator actors
   */
  void Initialize();

  /**
   * Constructs the renderers used for the background
   */
  Dali::Geometry CreateBackgroundGeometry();

  /**
   * Touch event callback.
   * It should pass the valid touch event to indicator server
   *
   * @param[in] indicator  The indicator actor that was touched
   * @param[in] touchEvent The touch event
   */
  bool OnTouched(Dali::Actor indicator, const TouchEvent& touchEvent);

  /**
   * Pan gesture callback.
   * It finds flick down gesture to show hidden indicator image
   *
   * @param[in] actor  The actor for gesture
   * @param[in] gesture The gesture event
   */
  void OnPan( Dali::Actor actor, const Dali::PanGesture& gesture );

  /**
   * Touch event callback on stage.
   * If stage is touched, hide showing indicator image
   *
   * @param[in] touchEvent The touch event
   */
  void OnStageTouched(const Dali::TouchEvent& touchEvent);

  /**
   * Connect to the indicator service
   */
  bool Connect();

  /**
   * Start the reconnection timer. This will run every second until we reconnect to
   * the indicator service.
   */
  void StartReconnectionTimer();

  /**
   * If connection failed, attempt to re-connect every second
   */
  bool OnReconnectTimer();

  /**
   * Disconnect from the indicator service
   */
  void Disconnect();

  /**
   * Handle Resize event
   * @param[in] width The new width
   * @param[in] height The new height
   */
  void Resize( int width, int height );

  /**
   * Set the lock file info.
   * @param[in] epcEvent Current ecore event.
   */
  void SetLockFileInfo( Ecore_Ipc_Event_Server_Data *epcEvent );

  /**
   * Set the shared indicator image info
   * @param[in] epcEvent The event containing the image data
   */
  void SetSharedImageInfo( Ecore_Ipc_Event_Server_Data *epcEvent );

  /**
   * Load the shared indicator image
   * @param[in] epcEvent The event containing the image data
   */
  void LoadSharedImage( Ecore_Ipc_Event_Server_Data *epcEvent );

  /**
   * Load the pixmap indicator image
   * @param[in] epcEvent The event containing the image data
   */
  void LoadPixmapImage( Ecore_Ipc_Event_Server_Data *epcEvent );

  /**
   * Inform dali that the indicator data has been updated.
   * @param[in] bufferNumber The shared file number
   */
  void UpdateImageData( int bufferNumber );

  /**
   * Lock the temporary file, Copy the shared image into IndicatorBuffer
   * and then unlock the temporary file.
   * Caller should ensure we are not writing image to gl texture.
   * @param[in] bufferNumber The shared file number
   */
  bool CopyToBuffer( int bufferNumber );

  /**
   * Create a new image for the indicator, and set up signal handling for it.
   * @param[in] bufferNumber The shared file number
   */
  void CreateNewImage( int bufferNumber );

  /**
   * Create a new pixmap image for the indicator, and set up signal handling for it.
   */
  void CreateNewPixmapImage();

  /**
   * Indicator type has changed.
   * Inform observer
   * @param[in] type The new indicator type
   */
  void OnIndicatorTypeChanged( Type type );

  /**
   * Check whether the indicator could be visible or invisible
   * @return true if indicator should be shown
   */
  bool CheckVisibleState();

  /**
   * Show/Hide indicator actor with effect
   * @param[in] duration how long need to show the indicator,
   *                     if it equal to 0, hide the indicator
   *                     if it less than 0, show always
   */
  void ShowIndicator( float duration );

  /**
   * Showing timer callback
   */
  bool OnShowTimer();

  /**
   * Showing animation finished callback
   * @param[in] animation
   */
  void OnAnimationFinished( Dali::Animation& animation );

private: // Implementation of ServerConnection::Observer
  /**
   * @copydoc Dali::Internal::Adaptor::ServerConnection::Observer::DataReceived()
   */
  virtual void DataReceived( void* event );

  /**
   * @copydoc Dali::Internal::Adaptor::ServerConnection::Observer::DataReceived()
   */
  virtual void ConnectionClosed();

private:

  /**
   * Clear shared file info
   */
  void ClearSharedFileInfo();

private:

  struct SharedFileInfo
  {
    SharedFileInfo()
      : mLock( NULL ),
        mSharedFile( NULL ),
        mImageWidth( 0 ),
        mImageHeight( 0 ),
        mLockFileName(),
        mSharedFileName(),
        mSharedFileID( 0 ),
        mSharedFileNumber( 0 )
    {
    }

    LockFile*                        mLock;              ///< File lock for the shared file
    SharedFile*                      mSharedFile;        ///< Shared file

    int                              mImageWidth;        ///< Shared image width
    int                              mImageHeight;       ///< Shared image height

    std::string                      mLockFileName;      ///< Lock file name
    std::string                      mSharedFileName;    ///< Shared file name
    int                              mSharedFileID;      ///< Shared file ID
    int                              mSharedFileNumber;  ///< Shared file number
  };

  static const int SHARED_FILE_NUMBER = 2;               ///< Shared file number

  Dali::Geometry                   mTranslucentGeometry; ///< Geometry used for rendering the translucent background
  Dali::Geometry                   mSolidGeometry;       ///< Geometry used for rendering the opaque background
  Dali::Shader                     mBackgroundShader;    ///< Shader used for rendering the background

  IndicatorBufferPtr               mIndicatorBuffer;     ///< class which handles indicator rendering
  PixmapId                         mPixmap;              ///< Pixmap including indicator content
  Dali::Image                      mImage;               ///< Image created from mIndicatorBuffer
  Dali::ImageActor                 mIndicatorImageActor; ///< Actor created from mImage

  Dali::Actor                      mIndicatorImageContainerActor; ///< Actor container for image and background
  Dali::Actor                      mBackgroundActor;     ///< Actor for background
  Dali::Actor                      mIndicatorActor;      ///< Handle to topmost indicator actor
  Dali::Actor                      mEventActor;          ///< Handle to event
  Dali::PanGestureDetector         mPanDetector;         ///< Pan detector to find flick gesture for hidden indicator
  float                            mGestureDeltaY;       ///< Checking how much panning moved
  bool                             mGestureDetected;     ///< Whether find the flick gesture

  Dali::Timer                      mReconnectTimer;      ///< Reconnection timer
  SlotDelegate< Indicator >        mConnection;

  Dali::Window::IndicatorBgOpacity mOpacityMode;         ///< Opacity enum for background
  Indicator::State                 mState;               ///< The connection state

  Adaptor*                         mAdaptor;
  ServerConnection*                mServerConnection;
  IndicatorInterface::Observer*    mObserver;            ///< Upload observer

  Dali::Window::WindowOrientation  mOrientation;
  int                              mImageWidth;
  int                              mImageHeight;
  Dali::Window::IndicatorVisibleMode mVisible;           ///< Whether the indicator is visible

  Dali::Timer                      mShowTimer;           ///< Timer to show indicator
  bool                             mIsShowing;           ///< Whether the indicator is showing on the screen
  Dali::Animation                  mIndicatorAnimation;  ///< Animation to show/hide indicator image

  bool                             mIsAnimationPlaying;  ///< Whether the animation is playing

  int                              mCurrentSharedFile;   ///< Current shared file number
  SharedFileInfo                   mSharedFileInfo[SHARED_FILE_NUMBER];    ///< Table to store shared file info
};

} // Adaptor
} // Internal
} // Dali

#endif
