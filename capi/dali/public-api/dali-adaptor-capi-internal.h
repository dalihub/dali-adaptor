#ifndef __DALI_ADAPTOR_CAPI_INTERNAL_H__
#define __DALI_ADAPTOR_CAPI_INTERNAL_H__

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

// These defines come from running pkg-config --cflags with the correct package config file

#if defined(DALI_EVAS_PLUGIN)
#include <dali/public-api/adaptor-framework/evas-plugin.h>
#endif

#if defined(DALI_NATIVE_BUFFER_PLUGIN)
#include <dali/public-api/adaptor-framework/native-buffer-plugin.h>
#endif

#include <dali/public-api/adaptor-framework/common/accessibility-manager.h>
#include <dali/public-api/adaptor-framework/common/adaptor.h>
#include <dali/public-api/adaptor-framework/common/clipboard.h>
#include <dali/public-api/adaptor-framework/common/clipboard-event-notifier.h>
#include <dali/public-api/adaptor-framework/common/device-layout.h>
#include <dali/public-api/adaptor-framework/common/drag-and-drop-detector.h>
#include <dali/public-api/adaptor-framework/common/haptic-player.h>
#include <dali/public-api/adaptor-framework/common/imf-manager.h>
#include <dali/public-api/adaptor-framework/common/key.h>
#include <dali/public-api/adaptor-framework/common/orientation.h>
#include <dali/public-api/adaptor-framework/common/pixmap-image.h>
#include <dali/public-api/adaptor-framework/common/render-surface.h>
#include <dali/public-api/adaptor-framework/common/sound-player.h>
#include <dali/public-api/adaptor-framework/common/style-change.h>
#include <dali/public-api/adaptor-framework/common/style-monitor.h>
#include <dali/public-api/adaptor-framework/common/timer.h>
#include <dali/public-api/adaptor-framework/common/tts-player.h>
#include <dali/public-api/adaptor-framework/common/virtual-keyboard.h>
#include <dali/public-api/adaptor-framework/common/window.h>

#endif //__DALI_ADAPTOR_CAPI_INTERNAL_H__
