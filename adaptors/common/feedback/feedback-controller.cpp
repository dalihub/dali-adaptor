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
#include <feedback/feedback-controller.h>

// EXTERNAL INCLUDES
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/stage.h>
#include <dali/public-api/object/object-registry.h>

// INTERNAL INCLUDES
#include <feedback/feedback-ids.h>
#include <feedback/feedback-plugin-proxy.h>

using std::string;
using boost::property_tree::ptree;

namespace // unnamed namespace
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::General, false, "LOG_FEEDBACK_CONTROLLER");
#endif

const char* DEFAULT_FEEDBACK_THEME_PATH = DALI_FEEDBACK_THEME_DIR"default-feedback-theme.json";

string LoadFile(const string& filename)
{
  DALI_ASSERT_DEBUG( 0 != filename.length());

  string contents;

  std::filebuf buf;
  buf.open(filename.c_str(), std::ios::in);
  if( buf.is_open() )
  {
    std::istream stream(&buf);

    // determine data length
    stream.seekg(0, std::ios_base::end);
    unsigned int length = static_cast<unsigned int>( stream.tellg() );
    stream.seekg(0, std::ios_base::beg);

    // allocate a buffer
    contents.resize(length);
    // read data into buffer
    stream.read(&contents[0], length);

    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "ResourceLoader::LoadFile(%s) - loaded %d bytes\n", filename.c_str(), length );
  }
  else
  {
    DALI_LOG_ERROR("ResourceLoader::LoadFile(%s) - failed to load\n", filename.c_str());
  }

  return contents;
}

static string FindFilename( const ptree& child )
{
  boost::optional<string> filename = child.get_optional<string>( "filename" );
  DALI_ASSERT_ALWAYS( filename && "Filename definiton must have 'filename'" );

  struct stat buf;

  if( 0 == stat( (*filename).c_str(), &buf) )
  {
    return *filename;
  }

  // else not found
  return "";
}

} // unnamed namespace

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

struct SignalFeedbackInfo
{
  /**
   * Default constructor.
   */
  SignalFeedbackInfo()
  :mHasHapticFeedbackInfo(false),
   mHasSoundFeedbackInfo(false)
  {
  }

  string mSignalName;
  bool        mHasHapticFeedbackInfo;
  bool        mHasSoundFeedbackInfo;
  string mHapticFeedbackPattern;
  string mSoundFeedbackPattern;
  string mHapticFeedbackFile;
  string mSoundFeedbackFile;
};

typedef std::vector<SignalFeedbackInfo> SignalFeedbackInfoContainer;
typedef SignalFeedbackInfoContainer::const_iterator SignalFeedbackInfoConstIter;

struct FeedbackStyleInfo
{
  /**
   * Default constructor.
   */
  FeedbackStyleInfo()
  {
  }

  string mTypeName;
  std::vector<SignalFeedbackInfo> mSignalFeedbackInfoList;
};

static const FeedbackStyleInfo DEFAULT_FEEDBACK_STYLE_INFO;

FeedbackController::FeedbackController( FeedbackPluginProxy& plugin )
: mPlugin( plugin ),
  mConnections( this )
{
  string defaultTheme = LoadFile( DEFAULT_FEEDBACK_THEME_PATH );
  LoadTheme( defaultTheme );

  Dali::ObjectRegistry registry = Dali::Stage::GetCurrent().GetObjectRegistry();

  registry.ObjectCreatedSignal().Connect(   mConnections, &FeedbackController::ObjectCreatedCallback );

  Dali::StyleMonitor styleMonitor( Dali::StyleMonitor::Get() );
  DALI_ASSERT_DEBUG( styleMonitor && "StyleMonitor not available" );
  styleMonitor.StyleChangeSignal().Connect( mConnections, &FeedbackController::StyleChangedCallback );
}

FeedbackController::~FeedbackController()
{
}

struct PlayFeedbackFromSignal
{
  PlayFeedbackFromSignal( FeedbackController& controller, const string& typeName, const string& signalName )
  : mController( controller ),
    mTypeName( typeName ),
    mSignalName( signalName )
  {
  }

  void operator()()
  {
    mController.PlayFeedback( mTypeName, mSignalName );
  }

  FeedbackController& mController;
  string mTypeName;
  string mSignalName;
};

void FeedbackController::ObjectCreatedCallback( BaseHandle handle )
{
  if( handle )
  {
    string type = handle.GetTypeName();

    const FeedbackStyleInfo styleInfo = GetStyleInfo( type );

    for( SignalFeedbackInfoConstIter iter = styleInfo.mSignalFeedbackInfoList.begin(); iter != styleInfo.mSignalFeedbackInfoList.end(); ++iter )
    {
      const SignalFeedbackInfo& info = *iter;

      if( info.mHasHapticFeedbackInfo || info.mHasSoundFeedbackInfo )
      {
        if( !info.mHapticFeedbackPattern.empty() || !info.mHapticFeedbackFile.empty() ||
            !info.mSoundFeedbackPattern.empty()  || !info.mSoundFeedbackFile.empty() )
        {
          handle.ConnectSignal( this,
                                info.mSignalName,
                                PlayFeedbackFromSignal( *this, type, info.mSignalName ) );

          DALI_LOG_INFO( gLogFilter, Debug::Verbose, "FeedbackController::ObjectCreatedCallback found Haptic pattern %s for Object type: %s, Signal Type: %s\n",
                         info.mHapticFeedbackPattern.c_str(), type.c_str(), info.mSignalName.c_str() );
        }
        else
        {
          DALI_LOG_ERROR("FeedbackController::ObjectCreatedCallback() Warning Inconsistent data in theme file!\n");
        }
      }
    }
  }
}

const FeedbackStyleInfo& FeedbackController::GetStyleInfo( const string& type ) const
{
  std::map<const string, FeedbackStyleInfo>::const_iterator iter( mStyleInfoLut.find( type ) );
  if( iter != mStyleInfoLut.end() )
  {
    return iter->second;
  }
  else
  {
    return DEFAULT_FEEDBACK_STYLE_INFO;
  }
}

void FeedbackController::StyleChangedCallback( Dali::StyleMonitor styleMonitor, Dali::StyleChange::Type styleChange )
{
  if( StyleChange::THEME_CHANGE )
  {
    const string& userDefinedThemePath = styleMonitor.GetTheme();
    const string& userDefinedTheme = LoadFile( userDefinedThemePath );

    if( !LoadTheme( userDefinedTheme ) )
    {
      DALI_LOG_ERROR("FeedbackController::StyleChangedCallback() User defined theme failed to load! \n");

      //If there is any problem is using the user defined theme, then fall back to default theme
      if( !LoadTheme( DEFAULT_FEEDBACK_THEME_PATH ) )
      {
        //If the default theme fails, Then No luck!
        DALI_LOG_ERROR("FeedbackController::StyleChangedCallback() Default theme failed to load! \n");
      }
    }
  }
}

bool FeedbackController::LoadTheme( const string& data )
{
  bool result = false;

  try
  {
    LoadFromString( data );

    result = true;
  }
  catch(...)
  {
    //Problem in user set theme, So fallback to use default theme.
    DALI_LOG_ERROR( "FeedbackController::LoadTheme() Failed to load theme\n" );
  }

  return result;
}

void FeedbackController::LoadFromString( const string& data )
{
  std::stringstream jsonData( data );

  ptree node;

  try
  {
    // tree root is cleared each read_json
    boost::property_tree::json_parser::read_json( jsonData, node );
  }
  catch( boost::property_tree::json_parser::json_parser_error& error )
  {
    DALI_LOG_WARNING( "JSON Parse Error:'%s'\n", error.message().c_str() );
    DALI_LOG_WARNING( "JSON Parse File :'%s'\n", error.filename().c_str() );
    DALI_LOG_WARNING( "JSON Parse Line :'%d'\n", error.line() );
    throw;
  }
  catch(...)
  {
    throw;
  }

  // Clear previously loaded style

  mSoundFilesLut.clear();
  mHapticFilesLut.clear();
  mStyleInfoLut.clear();

  // Parse filenames

  if ( node.get_child_optional("sounds") )
  {
    const ptree& soundsNode = node.get_child( "sounds" );
    const ptree::const_iterator endSoundsIter = soundsNode.end();
    for( ptree::const_iterator iter = soundsNode.begin(); endSoundsIter != iter; ++iter )
    {
      const ptree::value_type& keyChild = *iter;

      string key( keyChild.first );

      boost::optional<string> name( keyChild.second.get_optional<string>("filename") );
      if( name )
      {
        string fileName( FindFilename( keyChild.second ) );

        mSoundFilesLut.insert( std::pair<const string, const string>(key, fileName) );
      }
      else
      {
        DALI_LOG_WARNING("Invalid sound file\n");
      }
    }
  }

  if ( node.get_child_optional("haptic") )
  {
    const ptree& hapticNode = node.get_child( "haptic" );
    const ptree::const_iterator endHapticIter = hapticNode.end();
    for( ptree::const_iterator iter = hapticNode.begin(); endHapticIter != iter; ++iter )
    {
      const ptree::value_type& keyChild = *iter;

      string key( keyChild.first );

      boost::optional<string> name( keyChild.second.get_optional<string>("filename") );
      if( name )
      {
        string fileName( FindFilename( keyChild.second ) );

        mHapticFilesLut.insert( std::pair<const string, const string>(key, fileName) );
      }
      else
      {
        DALI_LOG_WARNING("Invalid haptic file\n");
      }
    }
  }

  // Parse style

  const ptree& styleNode = node.get_child( "style" );
  const ptree::const_iterator endIter = styleNode.end();
  for( ptree::const_iterator iter = styleNode.begin(); endIter != iter; ++iter )
  {
    const ptree::value_type& keyChild = *iter;

    string key( keyChild.first );
    FeedbackStyleInfo themeInfo;
    themeInfo.mTypeName = key;

    const ptree& signalsNode = keyChild.second.get_child( "signals" );
    const ptree::const_iterator endIter = signalsNode.end();
    for( ptree::const_iterator iter = signalsNode.begin(); endIter != iter; ++iter )
    {
      const ptree::value_type& signal_child = *iter;

      SignalFeedbackInfo signalFeedbackInfo;

      boost::optional<string> type( signal_child.second.get_optional<string>( "type" ) );
      DALI_ASSERT_ALWAYS( type && "Signal must have a type" );

      signalFeedbackInfo.mSignalName = *type;

      boost::optional<string> hapticFeedbackPattern( signal_child.second.get_optional<string>( "haptic-feedback-pattern" ) );
      if( hapticFeedbackPattern )
      {
        signalFeedbackInfo.mHasHapticFeedbackInfo = true;
        signalFeedbackInfo.mHapticFeedbackPattern = *hapticFeedbackPattern;
      }

      boost::optional<string> hapticFeedbackFile( signal_child.second.get_optional<string>( "haptic-feedback-file" ) );
      if( hapticFeedbackFile )
      {
        signalFeedbackInfo.mHasHapticFeedbackInfo = true;
        signalFeedbackInfo.mHapticFeedbackFile = GetHapticPath( *hapticFeedbackFile );
      }

      boost::optional<string> soundFeedbackPattern( signal_child.second.get_optional<string>( "sound-feedback-pattern" ) );
      if( soundFeedbackPattern )
      {
        signalFeedbackInfo.mHasSoundFeedbackInfo = true;
        signalFeedbackInfo.mSoundFeedbackPattern = *soundFeedbackPattern;
      }

      boost::optional<string> soundFeedbackFile( signal_child.second.get_optional<string>( "sound-feedback-file" ) );
      if( soundFeedbackFile )
      {
        signalFeedbackInfo.mHasSoundFeedbackInfo = true;
        signalFeedbackInfo.mSoundFeedbackFile = GetSoundPath( *soundFeedbackFile );
      }

      if( signalFeedbackInfo.mHasHapticFeedbackInfo || signalFeedbackInfo.mHasSoundFeedbackInfo )
      {
        AddSignalInfo( themeInfo, signalFeedbackInfo );
      }
    }

    mStyleInfoLut[key] = themeInfo;
  }
}

void FeedbackController::AddSignalInfo( FeedbackStyleInfo& styleInfo, SignalFeedbackInfo signalInfo )
{
  bool updated = false;
  std::vector<SignalFeedbackInfo>::iterator iter;

  // If info exists for the signal then update it, else add new
  for( iter = styleInfo.mSignalFeedbackInfoList.begin(); iter != styleInfo.mSignalFeedbackInfoList.end(); ++iter )
  {
    if( (*iter).mSignalName == signalInfo.mSignalName )
    {
      (*iter).mHasHapticFeedbackInfo = signalInfo.mHasHapticFeedbackInfo;
      (*iter).mHapticFeedbackPattern = signalInfo.mHapticFeedbackPattern;
      (*iter).mHapticFeedbackFile    = signalInfo.mHapticFeedbackFile;
      (*iter).mHasSoundFeedbackInfo  = signalInfo.mHasSoundFeedbackInfo;
      (*iter).mSoundFeedbackPattern  = signalInfo.mSoundFeedbackPattern;
      (*iter).mSoundFeedbackFile     = signalInfo.mSoundFeedbackFile;

      updated = true;
      break;
    }
  }

  if( !updated )
  {
    styleInfo.mSignalFeedbackInfoList.push_back( signalInfo );
  }
}

string FeedbackController::GetSoundPath( const string& key ) const
{
  std::map<const string, const string>::const_iterator iter( mSoundFilesLut.find( key ) );
  string path;
  if( iter != mSoundFilesLut.end() )
  {
    path = iter->second;
  }
  else
  {
    DALI_LOG_WARNING( "Request for sound file '%s' failed\n", key.c_str() );
    DALI_ASSERT_ALWAYS( !"Sound file does not exist" );
  }
  return path;
}

string FeedbackController::GetHapticPath( const string& key ) const
{
  std::map<const string, const string>::const_iterator iter( mHapticFilesLut.find( key ) );
  string path;
  if( iter != mHapticFilesLut.end() )
  {
    path = iter->second;
  }
  else
  {
    DALI_LOG_WARNING( "Request for haptic file '%s' failed\n", key.c_str() );
    DALI_ASSERT_ALWAYS( !"Haptic file does not exist" );
  }
  return path;
}

void FeedbackController::PlayFeedback(const string& type, const string& signalName)
{
  const FeedbackStyleInfo styleInfo = GetStyleInfo(type);
  SignalFeedbackInfoConstIter iter;

  for(iter = styleInfo.mSignalFeedbackInfoList.begin(); iter != styleInfo.mSignalFeedbackInfoList.end(); ++iter)
  {
    const SignalFeedbackInfo& info = *iter;

    if(info.mSignalName == signalName)
    {
      if(info.mHasHapticFeedbackInfo)
      {
        if(!info.mHapticFeedbackPattern.empty())
        {
          DALI_LOG_INFO( gLogFilter, Debug::Verbose, "FeedbackController::PlayFeedback Playing Haptic effect: Object type: %s, Signal type: %s, pattern type: %s\n",
              type.c_str(), signalName.c_str(), info.mHapticFeedbackPattern.c_str());

          mPlugin.PlayFeedbackPattern( FEEDBACK_TYPE_VIBRATION, GetFeedbackPattern(info.mHapticFeedbackPattern) );
        }
        else if(!info.mHapticFeedbackFile.empty())
        {
          mPlugin.PlayHaptic( info.mHapticFeedbackFile );
        }
      }

      if(info.mHasSoundFeedbackInfo)
      {
        if(!info.mSoundFeedbackPattern.empty())
        {
          DALI_LOG_INFO( gLogFilter, Debug::Verbose, "FeedbackController::PlayFeedback Playing Sound effect: Object type: %s, Signal type: %s, pattern type: %s\n",
              type.c_str(), signalName.c_str(), info.mHapticFeedbackPattern.c_str());

          mPlugin.PlayFeedbackPattern( FEEDBACK_TYPE_SOUND, GetFeedbackPattern(info.mSoundFeedbackPattern) );
        }
        else if(!info.mSoundFeedbackFile.empty())
        {
          mPlugin.PlaySound( info.mSoundFeedbackFile );
        }
      }

      break;
    }
  }
}

FeedbackPattern FeedbackController::GetFeedbackPattern( const string &pattern )
{
  if( 0 == mFeedbackPatternLut.size() )
  {
    mFeedbackPatternLut["FEEDBACK_PATTERN_NONE"]                = Dali::FEEDBACK_PATTERN_NONE;
    mFeedbackPatternLut["FEEDBACK_PATTERN_TAP"]                 = Dali::FEEDBACK_PATTERN_TAP;
    mFeedbackPatternLut["FEEDBACK_PATTERN_SIP"]                 = Dali::FEEDBACK_PATTERN_SIP;
    mFeedbackPatternLut["FEEDBACK_PATTERN_SIP_BACKSPACE"]       = Dali::FEEDBACK_PATTERN_SIP_BACKSPACE;
    mFeedbackPatternLut["FEEDBACK_PATTERN_MAX_CHARACTER"]       = Dali::FEEDBACK_PATTERN_MAX_CHARACTER;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY0"]                = Dali::FEEDBACK_PATTERN_KEY0;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY1"]                = Dali::FEEDBACK_PATTERN_KEY1;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY2"]                = Dali::FEEDBACK_PATTERN_KEY2;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY3"]                = Dali::FEEDBACK_PATTERN_KEY3;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY4"]                = Dali::FEEDBACK_PATTERN_KEY4;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY5"]                = Dali::FEEDBACK_PATTERN_KEY5;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY6"]                = Dali::FEEDBACK_PATTERN_KEY6;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY7"]                = Dali::FEEDBACK_PATTERN_KEY7;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY8"]                = Dali::FEEDBACK_PATTERN_KEY8;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY9"]                = Dali::FEEDBACK_PATTERN_KEY9;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY_STAR"]            = Dali::FEEDBACK_PATTERN_KEY_STAR;
    mFeedbackPatternLut["FEEDBACK_PATTERN_KEY_SHARP"]           = Dali::FEEDBACK_PATTERN_KEY_SHARP;
    mFeedbackPatternLut["FEEDBACK_PATTERN_HOLD"]                = Dali::FEEDBACK_PATTERN_HOLD;
    mFeedbackPatternLut["FEEDBACK_PATTERN_MULTI_TAP"]           = Dali::FEEDBACK_PATTERN_MULTI_TAP;
    mFeedbackPatternLut["FEEDBACK_PATTERN_HW_TAP"]              = Dali::FEEDBACK_PATTERN_HW_TAP;
    mFeedbackPatternLut["FEEDBACK_PATTERN_HW_HOLD"]             = Dali::FEEDBACK_PATTERN_HW_HOLD;
    mFeedbackPatternLut["FEEDBACK_PATTERN_MESSAGE"]             = Dali::FEEDBACK_PATTERN_MESSAGE;
    mFeedbackPatternLut["FEEDBACK_PATTERN_MESSAGE_ON_CALL"]     = Dali::FEEDBACK_PATTERN_MESSAGE_ON_CALL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_EMAIL"]               = Dali::FEEDBACK_PATTERN_EMAIL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_EMAIL_ON_CALL"]       = Dali::FEEDBACK_PATTERN_EMAIL_ON_CALL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_WAKEUP"]              = Dali::FEEDBACK_PATTERN_WAKEUP;
    mFeedbackPatternLut["FEEDBACK_PATTERN_WAKEUP_ON_CALL"]      = Dali::FEEDBACK_PATTERN_WAKEUP_ON_CALL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_SCHEDULE"]            = Dali::FEEDBACK_PATTERN_SCHEDULE;
    mFeedbackPatternLut["FEEDBACK_PATTERN_SCHEDULE_ON_CALL"]    = Dali::FEEDBACK_PATTERN_SCHEDULE_ON_CALL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_TIMER"]               = Dali::FEEDBACK_PATTERN_TIMER;
    mFeedbackPatternLut["FEEDBACK_PATTERN_TIMER_ON_CALL"]       = Dali::FEEDBACK_PATTERN_TIMER_ON_CALL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_GENERAL"]             = Dali::FEEDBACK_PATTERN_GENERAL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_GENERAL_ON_CALL"]     = Dali::FEEDBACK_PATTERN_GENERAL_ON_CALL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_POWERON"]             = Dali::FEEDBACK_PATTERN_POWERON;
    mFeedbackPatternLut["FEEDBACK_PATTERN_POWEROFF"]            = Dali::FEEDBACK_PATTERN_POWEROFF;
    mFeedbackPatternLut["FEEDBACK_PATTERN_CHARGERCONN"]         = Dali::FEEDBACK_PATTERN_CHARGERCONN;
    mFeedbackPatternLut["FEEDBACK_PATTERN_CHARGERCONN_ON_CALL"] = Dali::FEEDBACK_PATTERN_CHARGERCONN_ON_CALL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_FULLCHARGED"]         = Dali::FEEDBACK_PATTERN_FULLCHARGED;
    mFeedbackPatternLut["FEEDBACK_PATTERN_FULLCHARGED_ON_CALL"] = Dali::FEEDBACK_PATTERN_FULLCHARGED_ON_CALL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_LOWBATT"]             = Dali::FEEDBACK_PATTERN_LOWBATT;
    mFeedbackPatternLut["FEEDBACK_PATTERN_LOWBATT_ON_CALL"]     = Dali::FEEDBACK_PATTERN_LOWBATT_ON_CALL;
    mFeedbackPatternLut["FEEDBACK_PATTERN_LOCK"]                = Dali::FEEDBACK_PATTERN_LOCK;
    mFeedbackPatternLut["FEEDBACK_PATTERN_UNLOCK"]              = Dali::FEEDBACK_PATTERN_UNLOCK;
    mFeedbackPatternLut["FEEDBACK_PATTERN_CALLCONNECT"]         = Dali::FEEDBACK_PATTERN_CALLCONNECT;
    mFeedbackPatternLut["FEEDBACK_PATTERN_DISCALLCONNECT"]      = Dali::FEEDBACK_PATTERN_DISCALLCONNECT;
    mFeedbackPatternLut["FEEDBACK_PATTERN_MINUTEMINDER"]        = Dali::FEEDBACK_PATTERN_MINUTEMINDER;
    mFeedbackPatternLut["FEEDBACK_PATTERN_VIBRATION"]           = Dali::FEEDBACK_PATTERN_VIBRATION;
    mFeedbackPatternLut["FEEDBACK_PATTERN_SHUTTER"]             = Dali::FEEDBACK_PATTERN_SHUTTER;
    mFeedbackPatternLut["FEEDBACK_PATTERN_LIST_REORDER"]        = Dali::FEEDBACK_PATTERN_LIST_REORDER;
    mFeedbackPatternLut["FEEDBACK_PATTERN_SLIDER_SWEEP"]        = Dali::FEEDBACK_PATTERN_SLIDER_SWEEP;
  }

  std::map<const string, FeedbackPattern>::const_iterator iter( mFeedbackPatternLut.find( pattern ) );

  if( iter != mFeedbackPatternLut.end() )
  {
    return iter->second;
  }
  else
  {
    DALI_LOG_ERROR( "Unknown feedback pattern type: %s, So Defaulting to FEEDBACK_PATTERN_NONE!\n" );
    return Dali::FEEDBACK_PATTERN_NONE;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
