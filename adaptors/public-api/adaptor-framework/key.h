#ifndef __DALI_KEY_H__
#define __DALI_KEY_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/public-api/events/key-event.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief Enumeration for mapping keyboard and mouse button event keycodes to platform specific codes.
 * @SINCE_1_0.0
 */

enum KEY
{
  DALI_KEY_INVALID       = -1,       ///< Invalid key value @SINCE_1_0.0
  DALI_KEY_ESCAPE      = 9,        ///< Escape key @SINCE_1_0.0
  DALI_KEY_BACKSPACE     = 22,       ///< Backspace key @SINCE_1_0.0
  DALI_KEY_SHIFT_LEFT     = 50,       ///< Shift Left key @SINCE_1_0.0
  DALI_KEY_SHIFT_RIGHT     = 62,       ///< Shift Right key @SINCE_1_0.0
  DALI_KEY_CURSOR_UP     = 111,      ///< Cursor up key @SINCE_1_0.0
  DALI_KEY_CURSOR_LEFT     = 113,      ///< Cursor left key @SINCE_1_0.0
  DALI_KEY_CURSOR_RIGHT     = 114,      ///< Cursor right key @SINCE_1_0.0
  DALI_KEY_CURSOR_DOWN     = 116,      ///< Cursor down key @SINCE_1_0.0
  DALI_KEY_BACK       = 166,      ///< Back key @SINCE_1_0.0
  DALI_KEY_CAMERA      = 167,      ///< Camera key @SINCE_1_0.0
  DALI_KEY_CONFIG      = 168,      ///< Config key @SINCE_1_0.0
  DALI_KEY_POWER      = 169,      ///< Power key @SINCE_1_0.0
  DALI_KEY_PAUSE      = 170,      ///< Pause key @SINCE_1_0.0
  DALI_KEY_CANCEL      = 171,      ///< Cancel key @SINCE_1_0.0
  DALI_KEY_PLAY_CD      = 172,      ///< Play CD key @SINCE_1_0.0
  DALI_KEY_STOP_CD      = 173,      ///< Stop CD key @SINCE_1_0.0
  DALI_KEY_PAUSE_CD      = 174,      ///< Pause CD key @SINCE_1_0.0
  DALI_KEY_NEXT_SONG     = 175,      ///< Next song key @SINCE_1_0.0
  DALI_KEY_PREVIOUS_SONG    = 176,      ///< Previous song key @SINCE_1_0.0
  DALI_KEY_REWIND      = 177,      ///< Rewind key @SINCE_1_0.0
  DALI_KEY_FASTFORWARD     = 178,      ///< Fastforward key @SINCE_1_0.0
  DALI_KEY_MEDIA      = 179,      ///< Media key @SINCE_1_0.0
  DALI_KEY_PLAY_PAUSE     = 180,      ///< Play pause key @SINCE_1_0.0
  DALI_KEY_MUTE       = 181,      ///< Mute key @SINCE_1_0.0
  DALI_KEY_MENU       = 182,      ///< Menu key @SINCE_1_0.0
  DALI_KEY_HOME       = 183,      ///< Home key @SINCE_1_0.0
  DALI_KEY_HOMEPAGE      = 187,      ///< Homepage key @SINCE_1_0.0
  DALI_KEY_WEBPAGE      = 188,      ///< Webpage key @SINCE_1_0.0
  DALI_KEY_MAIL       = 189,      ///< Mail key @SINCE_1_0.0
  DALI_KEY_SCREENSAVER     = 190,      ///< Screensaver key @SINCE_1_0.0
  DALI_KEY_BRIGHTNESS_UP    = 191,      ///< Brightness up key @SINCE_1_0.0
  DALI_KEY_BRIGHTNESS_DOWN    = 192,      ///< Brightness down key @SINCE_1_0.0
  DALI_KEY_SOFT_KBD      = 193,      ///< Soft KBD key @SINCE_1_0.0
  DALI_KEY_QUICK_PANEL     = 194,      ///< Quick panel key @SINCE_1_0.0
  DALI_KEY_TASK_SWITCH     = 195,      ///< Task switch key @SINCE_1_0.0
  DALI_KEY_APPS       = 196,      ///< Apps key @SINCE_1_0.0
  DALI_KEY_SEARCH      = 197,      ///< Search key @SINCE_1_0.0
  DALI_KEY_VOICE      = 198,      ///< Voice key @SINCE_1_0.0
  DALI_KEY_LANGUAGE    = 199,      ///< Language key @SINCE_1_0.0
  DALI_KEY_VOLUME_UP      = 200,      ///< Volume up key @SINCE_1_0.0
  DALI_KEY_VOLUME_DOWN    = 201,       ///< Volume down key @SINCE_1_0.0
  DALI_KEY_1       = 10,
  DALI_KEY_2       = 11,
  DALI_KEY_3       = 12,
  DALI_KEY_4       = 13,
  DALI_KEY_5       = 14,
  DALI_KEY_6       = 15,
  DALI_KEY_7       = 16,
  DALI_KEY_8       = 17,
  DALI_KEY_9       = 18,
  DALI_KEY_0       = 19,
  DALI_KEY_MINUS      = 20,
  DALI_KEY_OK       = 36,
  DALI_KEY_RED       = 67,
  DALI_KEY_GREEN      = 68,
  DALI_KEY_YELLOW      = 69,
  DALI_KEY_BLUE       = 70,
  DALI_KEY_SOURCE      = 72,
  DALI_KEY_CH_LIST      = 73,
  DALI_KEY_LSGT       = 94,
  DALI_KEY_CHDOWN      = 95,
  DALI_KEY_CHUP       = 96,
  DALI_KEY_LFD_POWEROFF     = 120,
  DALI_KEY_LFD_MAGICINFO    = 128,
  DALI_KEY_SYS_MENU      = 133,
  DALI_KEY_TOOLS      = 135,
  DALI_KEY_REPEAT      = 137,
  DALI_KEY_GUIDE      = 138,
  DALI_KEY_ASPECT      = 140,
  DALI_KEY_PMODE      = 141,
  DALI_KEY_HDMI       = 143,
  DALI_KEY_USBHUB_SWITCH    = 144,
  DALI_KEY_EMANUAL      = 146,
  DALI_KEY_LFD_LOCK      = 147,
  DALI_KEY_MORE       = 148,
  DALI_KEY_FACTORY      = 149,
  DALI_KEY_SLEEP      = 150,
  DALI_KEY_TV       = 153,
  DALI_KEY_DTV       = 154,
  DALI_KEY_STB_POWER     = 155,
  DALI_KEY_ADDDEL      = 156,
  DALI_KEY_PANEL_DOWN     = 157,
  DALI_KEY_BT_COLOR_MECHA    = 159,
  DALI_KEY_STILL_PICTURE    = 160,
  DALI_KEY_BT_TRIGGER     = 161,
  DALI_KEY_BT_HOTKEY     = 162,
  DALI_KEY_CINEMA_MODE     = 163,
  DALI_KEY_BT_CONTENTSBAR    = 164,
  DALI_KEY_GAME       = 165,
  DALI_KEY_MBR_STB_GUIDE    = 184,
  DALI_KEY_MBR_BD_POPUP     = 185,
  DALI_KEY_MBR_BDDVD_POWER    = 186,
  DALI_KEY_WIFI_PAIRING     = 202,
  DALI_KEY_BT_DUALVIEW     = 210,
  DALI_KEY_BT_SAMSUNG_APPS    = 211,
  DALI_KEY_FAMILY_MODE     = 212,
  DALI_KEY_ESAVING      = 213,
  DALI_KEY_CLEAR      = 214,
  DALI_KEY_SUB_TITLE     = 215,
  DALI_KEY_TV_SNS      = 217,
  DALI_KEY_DVR       = 218,
  DALI_KEY_CAPTION      = 221,
  DALI_KEY_ZOOM1      = 222,
  DALI_KEY_PANEL_PLUS     = 223,
  DALI_KEY_BT_VOICE      = 224,
  DALI_KEY_PANEL_MINUS     = 227,
  DALI_KEY_SOCCER_MODE     = 228,
  DALI_KEY_FUNCTIONS_AMAZON    = 229,
  DALI_KEY_AD       = 230,
  DALI_KEY_REWIND_      = 232,
  DALI_KEY_FF_       = 233,
  DALI_KEY_FUNCTIONS_NETFLIX   = 234,
  DALI_KEY_PIP_ONOFF     = 235,
  DALI_KEY_MBR_WATCH_MOVIE    = 236,
  DALI_KEY_MBR_STBBD_MENU    = 237,
  DALI_KEY_MBR_SETUP_CONFIRM   = 238,
  DALI_KEY_FAMILYHUB     = 239,
  DALI_KEY_ANYVIEW      = 240,
  DALI_KEY_BT_DEVICE     = 241,
  DALI_KEY_SOFT_WAKE_UP     = 244,
  DALI_KEY_LFD_SET      = 245,
  DALI_KEY_WAKE_UP_MODE     = 246,
  DALI_KEY_LFD_BLANK     = 247,
  DALI_KEY_PAGE_LEFT     = 249,
  DALI_KEY_PAGE_RIGHT     = 250,
  DALI_KEY_PLAY_BACK     = 252,
  DALI_KEY_EXTRA      = 253,
  DALI_KEY_DISCRET_POWER_OFF   = 360,
  DALI_KEY_DISCRET_POWER_ON    = 361,
  DALI_KEY_DISCRET_S_VIDEO1    = 365,
  DALI_KEY_DISCRET_S_VIDEO2    = 366,
  DALI_KEY_DISCRET_S_VIDEO3    = 367,
  DALI_KEY_DISCRET_COMPONENT1   = 368,
  DALI_KEY_DISCRET_COMPONENT2   = 369,
  DALI_KEY_DISCRET_COMPONENT3   = 370,
  DALI_KEY_DISCRET_HDMI1    = 371,
  DALI_KEY_DISCRET_HDMI2    = 372,
  DALI_KEY_DISCRET_HDMI3    = 373,
  DALI_KEY_DISCRET_PC     = 374,
  DALI_KEY_DISCRET_DVI1     = 375,
  DALI_KEY_DISCRET_DVI2     = 376,
  DALI_KEY_DISCRET_ZOOM1    = 377,
  DALI_KEY_DISCRET_ZOOM2    = 378,
  DALI_KEY_DISCRET_PANORAMA    = 379,
  DALI_KEY_DISCRET_4_3     = 380,
  DALI_KEY_DISCRET_16_9     = 381,
  DALI_KEY_LFD_POWERON     = 382,
  DALI_KEY_DISCRET_LFD_DP    = 383,
  DALI_KEY_DISCRET_SOURCE_TV   = 384,
  DALI_KEY_COLOR      = 385,
  DALI_KEY_DISCRET_LFD_HDBT_RX   = 386,
  DALI_KEY_DISCRET_LFD_OPS    = 387,
  DALI_KEY_DISCRET_HDMI4    = 388,
  DALI_KEY_DISCRET_SOURCE_AV1   = 389,
  DALI_KEY_DISCRET_SOURCE_AV2   = 390,
  DALI_KEY_DISCRET_SOURCE_AV3   = 391,
  DALI_KEY_AV_OPEN      = 392,
  DALI_KEY_AV_DISC_MENU     = 393,
  DALI_KEY_AV_TITLE_MENU    = 394,
  DALI_KEY_TEST       = 395,
  DALI_KEY_COLD_START     = 396,
  DALI_KEY_VERSION      = 397,
  DALI_KEY_UNDEFINED     = 398,
  DALI_KEY_SCREEN_FREEZE    = 399,
  DALI_KEY_SCREEN_MUTE     = 400,
  DALI_KEY_LIST_END      = 405,
  DALI_KEY_LIST_HOME     = 406,
  DALI_KEY_VOICE_CONTROL    = 407,
  DALI_KEY_PAGE_UP      = 408,
  DALI_KEY_PAGE_DOWN     = 409,
  DALI_KEY_MBR_OPTION     = 410,
  DALI_KEY_ROOM_CONTROL     = 411,
  DALI_KEY_TEMPERATURE_UP    = 412,
  DALI_KEY_TEMPERATURE_DOWN    = 413,
  DALI_KEY_LIGHT_ON      = 414,
  DALI_KEY_LIGHT_OFF     = 415,
  DALI_KEY_DO_NOT_DISTURB    = 416,
  DALI_KEY_MAKE_UP_ROOM     = 417,
  DALI_KEY_ALARM      = 418,
  DALI_KEY_FUNC_USB      = 419,
  DALI_KEY_FUNC_DIN      = 420,
  DALI_KEY_SOUNDEFFECT1     = 421,
  DALI_KEY_SOUNDEFFECT2     = 422,
  DALI_KEY_SOUNDEFFECT3     = 423,
  DALI_KEY_SOUNDEFFECT4     = 424,
  DALI_KEY_SOUNDEFFECT5     = 425,
  DALI_KEY_SOUNDEFFECT6     = 426,
  DALI_KEY_SOUNDEFFECT7     = 427,
  DALI_KEY_SOUNDEFFECT8     = 428,
  DALI_KEY_SWPLUS      = 429,
  DALI_KEY_SWMINUS      = 430,
  DALI_KEY_ANYNET      = 431,
  DALI_KEY_AUTOPOWER     = 432,
  DALI_KEY_SOFTAP      = 433,
  DALI_KEY_ADDSPK      = 434,
  DALI_KEY_SCRECONNECT     = 435,
  DALI_KEY_NETWORKMD     = 436,
  DALI_KEY_SURROUNDOFF     = 437,
  DALI_KEY_SURROUNDON     = 438,
  DALI_KEY_BLUETOOTH     = 439,
  DALI_KEY_APDONOFF      = 440,
  DALI_KEY_SOUNDCONTROL     = 441,
  DALI_KEY_NIGHTMODE     = 442,
  DALI_KEY_USEREQ      = 443,
  DALI_KEY_EQLEVELDOWN     = 444,
  DALI_KEY_EQLEVELUP     = 445,
  DALI_KEY_DRCON      = 446,
  DALI_KEY_DRCOFF      = 447,
  DALI_KEY_SPEAKERLEVEL     = 448,
  DALI_KEY_WOOFERRST     = 449,
  DALI_KEY_MIRACAST_ON     = 450,
  DALI_KEY_MIRACAST_OFF     = 451,
  DALI_KEY_FUNC_RADIO     = 452,
  DALI_KEY_SOUNDSHARE_RESET    = 453,
  DALI_KEY_TOUCHMODE     = 454,
  DALI_KEY_GROUP      = 455,
  DALI_KEY_UNGROUP      = 456,
  DALI_KEY_DEEP_SLEEP     = 457,
  DALI_KEY_RM_LOG      = 458,
  DALI_KEY_AUX       = 459,
  DALI_KEY_EQ       = 460,
  DALI_KEY_GIGAEQ      = 461,
  DALI_KEY_DJ_BEAT      = 462,
  DALI_KEY_PANNING      = 463,
  DALI_KEY_MIC_IN      = 464,
  DALI_KEY_MIC_OUT      = 465,
  DALI_KEY_MIC_VOL_UP     = 466,
  DALI_KEY_MIC_VOL_DOWN     = 467,
  DALI_KEY_GYRO_A      = 468,
  DALI_KEY_GYRO_B      = 469,
  DALI_KEY_FANETOK      = 470,
  DALI_KEY_QSPOWER      = 471,
  DALI_KEY_SHOPMODE      = 472,
  DALI_KEY_WIFI_ONOFF     = 473,
  DALI_KEY_OPTICAL_DET     = 474,
  DALI_KEY_TV_REMOTE     = 475,
  DALI_KEY_FUNC_AUX      = 476,
  DALI_KEY_CHUP_CARD1     = 477,
  DALI_KEY_CHDOWN_CARD1     = 478,
  DALI_KEY_CHUP_CARD2     = 479,
  DALI_KEY_CHDOWN_CARD2     = 480,
  DALI_KEY_VOLUP_BATHROOM    = 481,
  DALI_KEY_VOLDOWN_BATHROOM    = 482,
  DALI_KEY_SELECT_DEVICE    = 483,
  DALI_KEY_SMARTSOUND_ON    = 484,
  DALI_KEY_SMARTSOUND_OFF    = 485,
  DALI_KEY_BLURAY_4X_FF     = 486,
  DALI_KEY_BLURAY_4X_REWIND    = 487,
  DALI_KEY_BLURAY_4X_STOP    = 488,
  DALI_KEY_DISC_PLAY     = 489,
  DALI_KEY_OCF_CONFIRM     = 490,
  DALI_KEY_MULTICODE_SET    = 491,
  DALI_KEY_MULTICODE_RESET    = 492,
  DALI_KEY_HOTEL_MODE_MENU    = 493,
  DALI_KEY_HOTEL_MOVIES     = 494,
  DALI_KEY_HOTEL_LANGUAGE    = 495,
  DALI_KEY_HOTEL_TV_GUIDE    = 496,
  DALI_KEY_HOTEL_APPS_GUESTs   = 497,
  DALI_KEY_QUICK_MENU     = 498,
  DALI_KEY_LFD_UNSET     = 500,
  DALI_KEY_PIP_CHUP      = 501,
  DALI_KEY_PIP_CHDOWN     = 502,
  DALI_KEY_ANTENA      = 503,
  DALI_KEY_AUTO_PROGRAM     = 504,
  DALI_KEY_PANEL_ENTER     = 505,
  DALI_KEY_LINK       = 506,
  DALI_KEY_PANEL_UP      = 507,
  DALI_KEY_REC       = 508,
  DALI_KEY_ANGLE      = 509,
  DALI_KEY_WHEEL_LEFT     = 510,
  DALI_KEY_WHEEL_RIGHT     = 511,
  DALI_KEY_PANEL_EXIT     = 512,
  DALI_KEY_EXIT       = 513,
  DALI_KEY_MBR_TV      = 514,
  DALI_KEY_MBR_SETUP_FAILURE   = 515,
  DALI_KEY_MBR_SETUP     = 516,
  DALI_KEY_MBR_WATCH_TV     = 517,
  DALI_KEY_PRECH      = 518,
  DALI_KEY_FAVCH      = 519,
  DALI_KEY_RECOMMEND_SEARCH_TOGGLE  = 520,
  DALI_KEY_BT_NUMBER     = 521,
  DALI_KEY_16_9       = 522,
  DALI_KEY_MTS       = 523,
  DALI_KEY_INFO       = 524,
  DALI_KEY_SMODE      = 525,
  DALI_KEY_3SPEED      = 526,
  DALI_KEY_3D       = 527,
  DALI_KEY_TTX_MIX      = 528,
  DALI_KEY_SRSTSXT      = 529,
  DALI_KEY_a       = 20000,
  DALI_KEY_b       = 20001,
  DALI_KEY_c       = 20002,
  DALI_KEY_d       = 20003,
  DALI_KEY_e       = 20004,
  DALI_KEY_f       = 20005,
  DALI_KEY_g       = 20006,
  DALI_KEY_h       = 20007,
  DALI_KEY_i       = 20008,
  DALI_KEY_j       = 20009,
  DALI_KEY_k       = 30000,
  DALI_KEY_l       = 30001,
  DALI_KEY_m       = 30002,
  DALI_KEY_n       = 30003,
  DALI_KEY_o       = 30004,
  DALI_KEY_p       = 30005,
  DALI_KEY_q       = 30006,
  DALI_KEY_r       = 30007,
  DALI_KEY_s       = 30008,
  DALI_KEY_t       = 30009,
  DALI_KEY_u       = 40000,
  DALI_KEY_v       = 40001,
  DALI_KEY_w       = 40002,
  DALI_KEY_x       = 40003,
  DALI_KEY_y       = 40004,
  DALI_KEY_z       = 40005,
  DALI_KEY_A       = 40006,
  DALI_KEY_B       = 40007,
  DALI_KEY_C       = 40008,
  DALI_KEY_D       = 40009,
  DALI_KEY_E       = 50000,
  DALI_KEY_F       = 50001,
  DALI_KEY_G       = 50002,
  DALI_KEY_H       = 50003,
  DALI_KEY_I       = 50004,
  DALI_KEY_J       = 50005,
  DALI_KEY_K       = 50006,
  DALI_KEY_L       = 50007,
  DALI_KEY_M       = 50008,
  DALI_KEY_N       = 50009,
  DALI_KEY_O       = 60000,
  DALI_KEY_P       = 60001,
  DALI_KEY_Q       = 60002,
  DALI_KEY_R       = 60003,
  DALI_KEY_S       = 60004,
  DALI_KEY_T       = 60005,
  DALI_KEY_U       = 60006,
  DALI_KEY_V       = 60007,
  DALI_KEY_W       = 60008,
  DALI_KEY_X       = 60009,
  DALI_KEY_Y       = 70000,
  DALI_KEY_Z       = 70001,
};

/**
 * @brief Checks if a key event is for a specific DALI KEY.
 *
 * @SINCE_1_0.0
 * @param keyEvent reference to a keyEvent structure
 * @param daliKey Dali key enum
 * @return @c true if the key is matched, @c false if not
 */
DALI_IMPORT_API bool IsKey( const Dali::KeyEvent& keyEvent, Dali::KEY daliKey);

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_KEY_H__
