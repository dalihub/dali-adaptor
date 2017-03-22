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
#include "key-impl.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace KeyLookup
{

// matches a DALI_KEY enum, to a key name
KeyLookup KeyLookupTable[]=
{
  // more than one key name can be assigned to a single dali-key code
  // e.g. "Menu" and "XF86Menu" are both assigned to  DALI_KEY_MENU

  { "Escape",                DALI_KEY_ESCAPE,       false },
  { "Menu",                  DALI_KEY_MENU,        false },

  // Now literal strings are used as key names instead of defined symbols in utilX,
  // since these definition in utilX.h is deprecated
  { "XF86Camera",            DALI_KEY_CAMERA,       false },
  { "XF86Camera_Full",       DALI_KEY_CONFIG,       false },
  { "XF86PowerOff",          DALI_KEY_POWER,       true  },
  { "XF86Standby",           DALI_KEY_PAUSE,       false },
  { "Cancel",                DALI_KEY_CANCEL,       false },
  { "XF86AudioPlay",         DALI_KEY_PLAY_CD,       false },
  { "XF86AudioStop",         DALI_KEY_STOP_CD,       false },
  { "XF86AudioPause",        DALI_KEY_PAUSE_CD,       false },
  { "XF86AudioNext",         DALI_KEY_NEXT_SONG,      false },
  { "XF86AudioPrev",         DALI_KEY_PREVIOUS_SONG,     false },
  { "XF86AudioRewind",       DALI_KEY_REWIND,       false },
  { "XF86AudioForward",      DALI_KEY_FASTFORWARD,      false },
  { "XF86AudioMedia",        DALI_KEY_MEDIA,       false },
  { "XF86AudioPlayPause",    DALI_KEY_PLAY_PAUSE,      false },
  { "XF86AudioMute",         DALI_KEY_MUTE,        false },
  { "XF86Menu",              DALI_KEY_MENU,        true  },
  { "XF86Home",              DALI_KEY_HOME,        true  },
  { "XF86Back",              DALI_KEY_BACK,        true  },
  { "XF86Send",              DALI_KEY_MENU,        true  },
  { "XF86Phone",             DALI_KEY_HOME,        true  },
  { "XF86Stop",              DALI_KEY_BACK,        true  },
  { "XF86HomePage",          DALI_KEY_HOMEPAGE,       false },
  { "XF86WWW",               DALI_KEY_WEBPAGE,       false },
  { "XF86Mail",              DALI_KEY_MAIL,        false },
  { "XF86ScreenSaver",       DALI_KEY_SCREENSAVER,      false },
  { "XF86MonBrightnessUp",   DALI_KEY_BRIGHTNESS_UP,     false },
  { "XF86MonBrightnessDown", DALI_KEY_BRIGHTNESS_DOWN,     false },
  { "XF86SoftKBD",           DALI_KEY_SOFT_KBD,       false },
  { "XF86QuickPanel",        DALI_KEY_QUICK_PANEL ,     false },
  { "XF86TaskPane",          DALI_KEY_TASK_SWITCH,      false },
  { "XF86Apps",              DALI_KEY_APPS,        false },
  { "XF86Search",            DALI_KEY_SEARCH,       false },
  { "XF86Voice",             DALI_KEY_VOICE,       false },
  { "Hangul",                DALI_KEY_LANGUAGE,       false },
  { "XF86AudioRaiseVolume",  DALI_KEY_VOLUME_UP,      true  },
  { "XF86AudioLowerVolume",  DALI_KEY_VOLUME_DOWN,      true  },
  { "BackSpace",             DALI_KEY_BACKSPACE,      false },
  { "Left",                  DALI_KEY_CURSOR_LEFT,      false },
  { "Right",                 DALI_KEY_CURSOR_RIGHT,      false },
  { "Shift_L",               DALI_KEY_SHIFT_LEFT,      false },
  { "Shift_R",               DALI_KEY_SHIFT_RIGHT,      false },
  {"1",       DALI_KEY_1,        false },
  {"2",       DALI_KEY_2,        false },
  {"3",       DALI_KEY_3,        false },
  {"4",       DALI_KEY_4,        false },
  {"5",       DALI_KEY_5,        false },
  {"6",       DALI_KEY_6,        false },
  {"7",       DALI_KEY_7,        false },
  {"8",       DALI_KEY_8,        false },
  {"9",       DALI_KEY_9,        false },
  {"0",       DALI_KEY_0,        false },
  {"minus",      DALI_KEY_MINUS,                      false },
  {"Return",     DALI_KEY_OK,                         false },
  {"XF86Red",     DALI_KEY_RED,                        true  },
  {"XF86Green",     DALI_KEY_GREEN,                      true  },
  {"XF86Yellow",    DALI_KEY_YELLOW,                     true  },
  {"XF86Blue",     DALI_KEY_BLUE,                       true  },
  {"XF86Display",    DALI_KEY_SOURCE,                     true  },
  {"XF86ChannelList",   DALI_KEY_CH_LIST,                    true  },
  {"XF86LSGT",     DALI_KEY_LSGT,           false },
  {"XF86LowerChannel",   DALI_KEY_CHDOWN,                     true  },
  {"XF86RaiseChannel",   DALI_KEY_CHUP,                       true  },
  {"Up",      DALI_KEY_CURSOR_UP,                  false },
  {"Down",      DALI_KEY_CURSOR_DOWN,                false },
  {"XF86LFDPowerOff",   DALI_KEY_LFD_POWEROFF,               true  },
  {"XF86LFDMagicInfo",   DALI_KEY_LFD_MAGICINFO,              true  },
  {"XF86SysMenu",    DALI_KEY_SYS_MENU,                   true  },
  {"XF86SimpleMenu",   DALI_KEY_TOOLS,                      false },
  {"XF86MBRRepeat",    DALI_KEY_REPEAT,                     false },
  {"XF86ChannelGuide",   DALI_KEY_GUIDE,                      true  },
  {"XF86PictureSize",   DALI_KEY_ASPECT,                     false },
  {"XF86PictureMode",   DALI_KEY_PMODE,                      false },
  {"XF86Hdmi",     DALI_KEY_HDMI,                       true  },
  {"XF86UsbHub",    DALI_KEY_USBHUB_SWITCH,              false },
  {"XF86EManual",    DALI_KEY_EMANUAL,                    true  },
  {"XF86LFDScreenLock",   DALI_KEY_LFD_LOCK,                   false },
  {"XF86More",     DALI_KEY_MORE,                       false },
  {"XF86FactoryMode",   DALI_KEY_FACTORY,                    false },
  {"XF86Sleep",     DALI_KEY_SLEEP,                      false },
  {"XF86TV",     DALI_KEY_TV,                         false },
  {"XF86DTV",     DALI_KEY_DTV,                        false },
  {"XF86STBPower",    DALI_KEY_STB_POWER,                  false },
  {"XF86ChannelAddDel",   DALI_KEY_ADDDEL,                     false },
  {"XF86PanelDown",    DALI_KEY_PANEL_DOWN,                 false },
  {"XF86BTColorMecha",   DALI_KEY_BT_COLOR_MECHA,             false },
  {"XF86StillPicture",   DALI_KEY_STILL_PICTURE,              false },
  {"XF86BTPairing",    DALI_KEY_BT_TRIGGER,                 false },
  {"XF86BTHotkey",    DALI_KEY_BT_HOTKEY,                  false },
  {"XF86CinemaMode",   DALI_KEY_CINEMA_MODE,                false },
  {"XF86BTContentsBar",   DALI_KEY_BT_CONTENTSBAR,             false },
  {"XF86Game",     DALI_KEY_GAME,                       false },
  {"XF86LFDUnset",    DALI_KEY_LFD_UNSET,                  false },
  {"XF86PIPChannelUp",   DALI_KEY_PIP_CHUP,                   false },
  {"XF86PIPChannelDown",  DALI_KEY_PIP_CHDOWN,                 false },
  {"XF86Antena",    DALI_KEY_ANTENA,                     false },
  {"XF86ChannelAutoTune",  DALI_KEY_AUTO_PROGRAM,               false },
  {"XF86PanelEnter",   DALI_KEY_PANEL_ENTER,                false },
  {"XF86MBRLink",    DALI_KEY_LINK,                       false },
  {"XF86PanelUp",    DALI_KEY_PANEL_UP,                   false },
  {"XF86AudioRecord",   DALI_KEY_REC,                        true  },
  {"XF86Game3D",    DALI_KEY_ANGLE,                      false },
  {"XF86WheelLeftKey",   DALI_KEY_WHEEL_LEFT,                 false },
  {"XF86WheelRightKey",   DALI_KEY_WHEEL_RIGHT,                false },
  {"XF86PanelExit",    DALI_KEY_PANEL_EXIT,                 false },
  {"XF86Exit",     DALI_KEY_EXIT,                       true  },
  {"XF86MBRTV",     DALI_KEY_MBR_TV,                     false },
  {"XF86MBRSTBGuide",   DALI_KEY_MBR_STB_GUIDE,              false },
  {"XF86MBRBDPopup",   DALI_KEY_MBR_BD_POPUP,               false },
  {"XF86MBRBDDVDPower",   DALI_KEY_MBR_BDDVD_POWER,            false },
  {"XF86MBRSetupFailure",  DALI_KEY_MBR_SETUP_FAILURE,          false },
  {"XF86MBRSetup",    DALI_KEY_MBR_SETUP,                  false },
  {"XF86MBRWatchTV",   DALI_KEY_MBR_WATCH_TV,               false },
  {"XF86PreviousChannel",  DALI_KEY_PRECH,                      true  },
  {"XF86FavoriteChannel",  DALI_KEY_FAVCH,                      true  },
  {"XF86Recommend",    DALI_KEY_RECOMMEND_SEARCH_TOGGLE,    false },
  {"XF86NumberPad",    DALI_KEY_BT_NUMBER,                  false },
  {"XF86AspectRatio169",  DALI_KEY_16_9,                       false },
  {"XF86MTS",     DALI_KEY_MTS,                        false },
  {"XF86Info",     DALI_KEY_INFO,                       false },
  {"XF86SoundMode",    DALI_KEY_SMODE,                      false },
  {"XF863XSpeed",    DALI_KEY_3SPEED,                     false },
  {"XF863D",     DALI_KEY_3D,                         false },
  {"XF86TTXMIX",    DALI_KEY_TTX_MIX,                    false },
  {"XF86SRSSXT",    DALI_KEY_SRSTSXT,                    false },
  {"XF86WIFIPairing",   DALI_KEY_WIFI_PAIRING,               false },
  {"XF86DualView",    DALI_KEY_BT_DUALVIEW,                false },
  {"XF86BTApps",    DALI_KEY_BT_SAMSUNG_APPS,            false },
  {"XF86FamilyMode",   DALI_KEY_FAMILY_MODE,                false },
  {"XF86EnergySaving",   DALI_KEY_ESAVING,                    false },
  {"XF86MBRClear",    DALI_KEY_CLEAR,                      false },
  {"XF86Subtitle",    DALI_KEY_SUB_TITLE,                  false },
  {"XF86TVSNS",     DALI_KEY_TV_SNS,                     false },
  {"XF86DVR",     DALI_KEY_DVR,                        false },
  {"XF86Caption",    DALI_KEY_CAPTION,                    false },
  {"XF86ZoomIn",    DALI_KEY_ZOOM1,                      false },
  {"XF86PanelPlus",    DALI_KEY_PANEL_PLUS,                 false },
  {"XF86BTVoice",    DALI_KEY_BT_VOICE,                   false },
  {"XF86PanelMinus",   DALI_KEY_PANEL_MINUS,                false },
  {"XF86SoccerMode",   DALI_KEY_SOCCER_MODE,                false },
  {"XF86Amazon",    DALI_KEY_FUNCTIONS_AMAZON,           false },
  {"XF86AudioDescription",  DALI_KEY_AD,                         false },
  {"XF86PreviousChapter",  DALI_KEY_REWIND_,                    false },
  {"XF86NextChapter",   DALI_KEY_FF_,                        false },
  {"XF86Netflix",    DALI_KEY_FUNCTIONS_NETFLIX,          false },
  {"XF86PIP",     DALI_KEY_PIP_ONOFF,                  false },
  {"XF86MBRWatchMovie",   DALI_KEY_MBR_WATCH_MOVIE,            false },
  {"XF86MBRMenu",    DALI_KEY_MBR_STBBD_MENU,             false },
  {"XF86MBRConfirm",   DALI_KEY_MBR_SETUP_CONFIRM,          false },
  {"XF86FamilyHub",    DALI_KEY_FAMILYHUB,                  false },
  {"XF86HDMICEC",    DALI_KEY_ANYVIEW,                    false },
  {"XF86BTDevice",    DALI_KEY_BT_DEVICE,                  false },
  {"XF86SoftWakeup",   DALI_KEY_SOFT_WAKE_UP,               false },
  {"XF86LFDSet",    DALI_KEY_LFD_SET,                    false },
  {"XF86Wakeup",    DALI_KEY_WAKE_UP_MODE,               false },
  {"XF86LFDBlank",    DALI_KEY_LFD_BLANK,                  false },
  {"XF86LeftPage",    DALI_KEY_PAGE_LEFT,                  false },
  {"XF86RightPage",    DALI_KEY_PAGE_RIGHT,                 false },
  {"XF86PlayBack",    DALI_KEY_PLAY_BACK,                  false },
  {"XF86ExtraApp",    DALI_KEY_EXTRA,                      false },
  {"XF86DiscretPowerOff",  DALI_KEY_DISCRET_POWER_OFF,          false },
  {"XF86DiscretPowerOn",  DALI_KEY_DISCRET_POWER_ON,           false },
  {"XF86DiscretSVideo1",  DALI_KEY_DISCRET_S_VIDEO1,           false },
  {"XF86DiscretSVideo2",  DALI_KEY_DISCRET_S_VIDEO2,           false },
  {"XF86DiscretSVideo3",  DALI_KEY_DISCRET_S_VIDEO3,           false },
  {"XF86DiscretComponent1",  DALI_KEY_DISCRET_COMPONENT1,         false },
  {"XF86DiscretComponent2",  DALI_KEY_DISCRET_COMPONENT2,         false },
  {"XF86DiscretComponent3",  DALI_KEY_DISCRET_COMPONENT3,         false },
  {"XF86DiscretHDMI1",   DALI_KEY_DISCRET_HDMI1,              false },
  {"XF86DiscretHDMI2",   DALI_KEY_DISCRET_HDMI2,              false },
  {"XF86DiscretHDMI3",   DALI_KEY_DISCRET_HDMI3,              false },
  {"XF86DiscretPC",    DALI_KEY_DISCRET_PC,                 false },
  {"XF86DiscretDVI1",   DALI_KEY_DISCRET_DVI1,               false },
  {"XF86DiscretDVI2",   DALI_KEY_DISCRET_DVI2,               false },
  {"XF86DiscretZoom1",   DALI_KEY_DISCRET_ZOOM1,              false },
  {"XF86DiscretZoom2",   DALI_KEY_DISCRET_ZOOM2,              false },
  {"XF86DiscretPanorama",  DALI_KEY_DISCRET_PANORAMA,           false },
  {"XF86Discret_4_3",   DALI_KEY_DISCRET_4_3,                false },
  {"XF86Discret_16_9",   DALI_KEY_DISCRET_16_9,      false },
  {"XF86LFDPowerOn",   DALI_KEY_LFD_POWERON,                false },
  {"XF86DiscretLFD_DP",   DALI_KEY_DISCRET_LFD_DP,             false },
  {"XF86DiscretSourceTV",  DALI_KEY_DISCRET_SOURCE_TV,          false },
  {"XF86Color",     DALI_KEY_COLOR,                      false },
  {"XF86DiscretLFD_HDBT_RX", DALI_KEY_DISCRET_LFD_HDBT_RX,        false },
  {"XF86DiscretLFD_OPS",  DALI_KEY_DISCRET_LFD_OPS,            false },
  {"XF86DiscretHDMI4",   DALI_KEY_DISCRET_HDMI4,              false },
  {"XF86DiscretAV1",   DALI_KEY_DISCRET_SOURCE_AV1,         false },
  {"XF86DiscretAV2",   DALI_KEY_DISCRET_SOURCE_AV2,         false },
  {"XF86DiscretAV3",   DALI_KEY_DISCRET_SOURCE_AV3,         false },
  {"XF86AVOpen",    DALI_KEY_AV_OPEN,                    false },
  {"XF86AVDiscMenu",   DALI_KEY_AV_DISC_MENU,               false },
  {"XF86AVTitleMenu",   DALI_KEY_AV_TITLE_MENU,              false },
  {"XF86Test",     DALI_KEY_TEST,                       false },
  {"XF86ColdStart",    DALI_KEY_COLD_START,                 false },
  {"XF86Version",    DALI_KEY_VERSION,                    false },
  {"XF86Undefined",    DALI_KEY_UNDEFINED,                  false },
  {"XF86ScreenFreeze",   DALI_KEY_SCREEN_FREEZE,              false },
  {"XF86ScreenMute",   DALI_KEY_SCREEN_MUTE,                false },
  {"XF86ListEnd",    DALI_KEY_LIST_END,                   false },
  {"XF86ListHome",    DALI_KEY_LIST_HOME,                  false },
  {"XF86VoiceControl",   DALI_KEY_VOICE_CONTROL,              false },
  {"XF86UpPage",    DALI_KEY_PAGE_UP,                    false },
  {"XF86DownPage",    DALI_KEY_PAGE_DOWN,                  false },
  {"XF86MBROption",    DALI_KEY_MBR_OPTION,                 false },
  {"XF86RoomControl",   DALI_KEY_ROOM_CONTROL,               false },
  {"XF86TemperatureUp",   DALI_KEY_TEMPERATURE_UP,             false },
  {"XF86TemperatureDown",  DALI_KEY_TEMPERATURE_DOWN,           false },
  {"XF86LightOn",    DALI_KEY_LIGHT_ON,                   false },
  {"XF86LightOff",    DALI_KEY_LIGHT_OFF,                  false },
  {"XF86DoNotDisturb",   DALI_KEY_DO_NOT_DISTURB,             false },
  {"XF86MakeUpRoom",   DALI_KEY_MAKE_UP_ROOM,               false },
  {"XF86Alarm",     DALI_KEY_ALARM,                      false },
  {"XF86FuncUSB",    DALI_KEY_FUNC_USB,                   false },
  {"XF86FuncDIN",    DALI_KEY_FUNC_DIN,                   false },
  {"XF86SoundEffect1",   DALI_KEY_SOUNDEFFECT1,               false },
  {"XF86SoundEffect2",   DALI_KEY_SOUNDEFFECT2,               false },
  {"XF86SoundEffect3",   DALI_KEY_SOUNDEFFECT3,               false },
  {"XF86SoundEffect4",   DALI_KEY_SOUNDEFFECT4,               false },
  {"XF86SoundEffect5",   DALI_KEY_SOUNDEFFECT5,               false },
  {"XF86SoundEffect6",   DALI_KEY_SOUNDEFFECT6,               false },
  {"XF86SoundEffect7",   DALI_KEY_SOUNDEFFECT7,               false },
  {"XF86SoundEffect8",   DALI_KEY_SOUNDEFFECT8,               false },
  {"XF86WooferPlus",   DALI_KEY_SWPLUS,                     false },
  {"XF86WooferMinus",   DALI_KEY_SWMINUS,                    false },
  {"XF86AnyNet",    DALI_KEY_ANYNET,                     false },
  {"XF86AutoPowerLink",   DALI_KEY_AUTOPOWER,                  false },
  {"XF86SoftAP",    DALI_KEY_SOFTAP,                     false },
  {"XF86AddSpeaker",   DALI_KEY_ADDSPK,                     false },
  {"XF86SCReconnect",   DALI_KEY_SCRECONNECT,                false },
  {"XF86NetworkStandBy",  DALI_KEY_NETWORKMD,                  false },
  {"XF86SurroundOff",   DALI_KEY_SURROUNDOFF,                false },
  {"XF86SurroundOn",   DALI_KEY_SURROUNDON,                 false },
  {"XF86BluetoothOnOff",  DALI_KEY_BLUETOOTH,                  false },
  {"XF86AutoPowerOnOff",  DALI_KEY_APDONOFF,                   false },
  {"XF86SoundControl",   DALI_KEY_SOUNDCONTROL,               true  },
  {"XF86NightMode",    DALI_KEY_NIGHTMODE,                  true  },
  {"XF86UserEQ",    DALI_KEY_USEREQ,                     true  },
  {"XF86EQLevelDown",   DALI_KEY_EQLEVELDOWN,                true  },
  {"XF86EQLevelUp",    DALI_KEY_EQLEVELUP,                  true  },
  {"XF86DRCOn",     DALI_KEY_DRCON,                      true  },
  {"XF86DRCOff",    DALI_KEY_DRCOFF,                     true  },
  {"XF86SpeakerLevel",   DALI_KEY_SPEAKERLEVEL,               true  },
  {"XF86WooferSet",    DALI_KEY_WOOFERRST,                  true  },
  {"XF86MiracastOn",   DALI_KEY_MIRACAST_ON,                true  },
  {"XF86MiracastOff",   DALI_KEY_MIRACAST_OFF,               true  },
  {"XF86FuncRadio",    DALI_KEY_FUNC_RADIO,                 true  },
  {"XF86SoundShareReset",  DALI_KEY_SOUNDSHARE_RESET,           true  },
  {"XF86TouchMode",    DALI_KEY_TOUCHMODE,                  true  },
  {"XF86Group",     DALI_KEY_GROUP,                      true  },
  {"XF86UnGroup",    DALI_KEY_UNGROUP,                    true  },
  {"XF86DeepSleep",    DALI_KEY_DEEP_SLEEP,                 true  },
  {"XF86RMLog",     DALI_KEY_RM_LOG,                     true  },
  {"XF86AuxDetect",    DALI_KEY_AUX,                        true  },
  {"XF86EQ",     DALI_KEY_EQ,                         true  },
  {"XF86GigaEQ",    DALI_KEY_GIGAEQ,                     true  },
  {"XF86DJBeat",    DALI_KEY_DJ_BEAT,                    true  },
  {"XF86Panning",    DALI_KEY_PANNING,                    true  },
  {"XF86MICConnect",   DALI_KEY_MIC_IN,                     true  },
  {"XF86MICDisconnect",   DALI_KEY_MIC_OUT,                    true  },
  {"XF86MICVolumeUp",   DALI_KEY_MIC_VOL_UP,                 true  },
  {"XF86MICVolumeDown",   DALI_KEY_MIC_VOL_DOWN,               true  },
  {"XF86GyroA",     DALI_KEY_GYRO_A,                     true  },
  {"XF86GyroB",     DALI_KEY_GYRO_B,                     true  },
  {"XF86FanetOK",    DALI_KEY_FANETOK,                    true  },
  {"XF86QuickStartPower",  DALI_KEY_QSPOWER,                    true  },
  {"XF86ShopMode",    DALI_KEY_SHOPMODE,                   true  },
  {"XF86WIFIOnOff",    DALI_KEY_WIFI_ONOFF,                 true  },
  {"XF86TVRemote",    DALI_KEY_TV_REMOTE,                  true  },
  {"XF86FuncAux",    DALI_KEY_FUNC_AUX,                   true  },
  {"XF86RaiseChannelCard1",  DALI_KEY_CHUP_CARD1,                 true  },
  {"XF86LowerChannelCard1",  DALI_KEY_CHDOWN_CARD1,               true  },
  {"XF86RaiseChannelCard2",  DALI_KEY_CHUP_CARD2,                 true  },
  {"XF86LowerChannelCard2",  DALI_KEY_CHDOWN_CARD2,               true  },
  {"XF86VolumeUpBathroom",  DALI_KEY_VOLUP_BATHROOM,             true  },
  {"XF86VolumeDownBathroom", DALI_KEY_VOLDOWN_BATHROOM,           true  },
  {"XF86SelectDevice",   DALI_KEY_SELECT_DEVICE,              true  },
  {"XF86SmartSoundOn",   DALI_KEY_SMARTSOUND_ON,              true  },
  {"XF86SmartSoundOff",   DALI_KEY_SMARTSOUND_OFF,             true  },
  {"XF86Bluray4xFF",   DALI_KEY_BLURAY_4X_FF,               true  },
  {"XF86Bluray4xRewind",  DALI_KEY_BLURAY_4X_REWIND,           true  },
  {"XF86Bluray4xStop",   DALI_KEY_BLURAY_4X_STOP,             true  },
  {"XF86DiscPlay",    DALI_KEY_DISC_PLAY,                  true  },
  {"XF86OCFConfirm",   DALI_KEY_OCF_CONFIRM,                true  },
  {"XF86MultiCodeSet",   DALI_KEY_MULTICODE_SET,              true  },
  {"XF86MultiCodeReset",  DALI_KEY_MULTICODE_RESET,            true  },
  {"XF86HotelModeMenu",   DALI_KEY_HOTEL_MODE_MENU,            true  },
  {"XF86HotelMovies",   DALI_KEY_HOTEL_MOVIES,               true  },
  {"XF86HotelLanguage",   DALI_KEY_HOTEL_LANGUAGE,             true  },
  {"XF86HotelTVGuide",   DALI_KEY_HOTEL_TV_GUIDE,             true  },
  {"XF86HotelAppsGuest",  DALI_KEY_HOTEL_APPS_GUESTs,           true  },
  {"XF86QuickMenu ",   DALI_KEY_QUICK_MENU,      true  },
  {"a",       DALI_KEY_a,        false },
  {"b",       DALI_KEY_b,        false },
  {"c",       DALI_KEY_c,        false },
  {"d",       DALI_KEY_d,        false },
  {"e",       DALI_KEY_e,        false },
  {"f",       DALI_KEY_f,        false },
  {"g",       DALI_KEY_g,        false },
  {"h",       DALI_KEY_h,        false },
  {"i",       DALI_KEY_i,        false },
  {"j",       DALI_KEY_j,        false },
  {"k",       DALI_KEY_k,        false },
  {"l",       DALI_KEY_l,        false },
  {"m",       DALI_KEY_m,        false },
  {"n",       DALI_KEY_n,        false },
  {"o",       DALI_KEY_o,        false },
  {"p",       DALI_KEY_p,        false },
  {"q",       DALI_KEY_q,        false },
  {"r",       DALI_KEY_r,        false },
  {"s",       DALI_KEY_s,        false },
  {"t",       DALI_KEY_t,        false },
  {"u",       DALI_KEY_u,        false },
  {"v",       DALI_KEY_v,        false },
  {"w",       DALI_KEY_w,        false },
  {"x",       DALI_KEY_x,        false },
  {"y",       DALI_KEY_y,        false },
  {"z",       DALI_KEY_z,        false },
  {"A",       DALI_KEY_A,        false },
  {"B",       DALI_KEY_B,        false },
  {"C",       DALI_KEY_C,        false },
  {"D",       DALI_KEY_D,        false },
  {"E",       DALI_KEY_E,        false },
  {"F",       DALI_KEY_F,        false },
  {"G",       DALI_KEY_G,        false },
  {"H",       DALI_KEY_H,        false },
  {"I",       DALI_KEY_I,        false },
  {"J",       DALI_KEY_J,        false },
  {"K",       DALI_KEY_K,        false },
  {"L",       DALI_KEY_L,        false },
  {"M",       DALI_KEY_M,        false },
  {"N",       DALI_KEY_N,        false },
  {"O",       DALI_KEY_O,        false },
  {"P",       DALI_KEY_P,        false },
  {"Q",       DALI_KEY_Q,        false },
  {"R",       DALI_KEY_R,        false },
  {"S",       DALI_KEY_S,        false },
  {"T",       DALI_KEY_T,        false },
  {"U",       DALI_KEY_U,        false },
  {"V",       DALI_KEY_V,        false },
  {"W",       DALI_KEY_W,        false },
  {"X",       DALI_KEY_X,        false },
  {"Y",       DALI_KEY_Y,        false },
  {"Z",       DALI_KEY_Z,        false }
};

const std::size_t KEY_LOOKUP_COUNT = (sizeof( KeyLookupTable))/ (sizeof(KeyLookup));

} // namespace KeyLookup

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
