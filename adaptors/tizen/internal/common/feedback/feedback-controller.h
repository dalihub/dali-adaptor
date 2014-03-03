#ifndef __DALI_INTERNAL_FEEDBACK_CONTROLLER_H__
#define __DALI_INTERNAL_FEEDBACK_CONTROLLER_H__

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

// EXTERNAL INCLUDES
#include <map>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/adaptor-framework/common/style-monitor.h>

// INTERNAL INCLUDES
#include <public-api/adaptor-framework/common/feedback-plugin.h>
#include <internal/common/feedback/feedback-ids.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

struct FeedbackStyleInfo;
struct SignalFeedbackInfo;

class FeedbackPluginProxy;

/**
 * Plays feedback effects for Dali-Toolkit UI Controls.
 */
class FeedbackController : public ConnectionTracker
{
public:

  /**
   * Constructor.
   */
  FeedbackController( FeedbackPluginProxy& plugin );

  /**
   * The destructor
   */
  ~FeedbackController();

  /**
   * Called to start playing feedback effects.
   */
  void Start();

  /**
   * Called to stop playing feedback effects.
   */
  void Stop();

  /**
   * Callback function to play a feedback effect when a signal is emitted for an object
   * Plays feedback effect.
   * @param [in] type The Object type
   * @param [in] signalName The name of the signal
   */
  void PlayFeedback(const std::string& type, const std::string& signalName);

private:

  /**
   * UI string data format
   */
  enum UIFormat
  {
    JSON, ///< String is JSON
  };

  /**
   * Callback function for Dali::ObjectRegistry::SignalObjectCreated signal
   * @param [in] object Handle to the newly created object
   */
  void ObjectCreatedCallback( BaseHandle object );

  /**
   * Helper to retrieve styleInfo from mStyleInfoLut
   * @param type A string described a type of object
   * @return The style information for the given object
   */
  const FeedbackStyleInfo& GetStyleInfo( const std::string& type) const;

  /**
   * Callback function for Dali::ObjectRegistry::SignalObjectCreated signal
   * @param [in] object Handle to the newly created object
   */
  void StyleChangedCallback(Dali::StyleMonitor styleMonitor, StyleChange styleChange);

  /**
   * Callback function for Dali::Toolkit::PushButton::SignalPressed signal
   * Plays feedback effect.
   * @param [in] effect The feedback effect to play
   */
  bool LoadTheme(const std::string& data);

  /**
   * Loads a string representation the theme.
   * @param [in] data A string represenation of the theme.
   * @param [in] format The string representation format ie JSON.
   */
  void LoadFromString( const std::string& data );

  /**
   * Helper to store signal information.
   * @param [in] styleInfo The information will be stored here.
   * @param [in] signalInfo The information to add.
   */
  void AddSignalInfo( FeedbackStyleInfo& styleInfo, SignalFeedbackInfo signalInfo );

  /**
   * Helper to retrieve the path to a sound file.
   * @param[in] key A string which identifies the path.
   * @return The path.
   */
  std::string GetSoundPath( const std::string& key ) const;

  /**
   * Helper to retrieve the path to a haptic file.
   * @param[in] key A string which identifies the path.
   * @return The path.
   */
  std::string GetHapticPath( const std::string& key ) const;

  /**
   * Map a pattern string to feedback pattern ID.
   * @param [in] pattern The pattern string.
   * @return A feedback pattern ID.
   */
  FeedbackPattern GetFeedbackPattern( const std::string& pattern );

  /**
   * Plays a feedback effect
   * @param [in] type The feedback type haptic or sound
   * @param [in] effect The feedback effect to play
   */
  void PlayEffect(FeedbackType type, FeedbackPattern effect);

  /**
   * Plays a haptic or sound effect file
   * @param [in] type The feedback type haptic or sound
   * @param [in] file The path to the file containing the effect
   */
  void PlayFile(FeedbackType type, const std::string& file);

private:

  FeedbackPluginProxy& mPlugin;

  std::map<const std::string, FeedbackPattern>   mFeedbackPatternLut; ///< Used to convert feedback pattern strings into enumerated values
  std::map<const std::string, const std::string> mSoundFilesLut;      ///< Converts key strings into sound file paths
  std::map<const std::string, const std::string> mHapticFilesLut;     ///< Converts key strings into haptic file paths
  std::map<const std::string, FeedbackStyleInfo> mStyleInfoLut;       ///< Converts key strings into style information

  SlotDelegate< FeedbackController > mConnections; ///< Maintains the connections to the Object registry.
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_FEEDBACK_CONTROLLER_H__
