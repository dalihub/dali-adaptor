#ifndef __DALI_H__
#define __DALI_H__

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

#include <dali/public-api/dali-core.h>

// Application / UI Framework adaption

// These defines come from running pkg-config --cflags with the correct package config file

#if defined(DALI_APPLICATION)
#include <dali/public-api/application.h>
#endif

#include <dali/public-api/accessibility-manager.h>
#include <dali/public-api/adaptor.h>
#include <dali/public-api/clipboard.h>
#include <dali/public-api/clipboard-event-notifier.h>
#include <dali/public-api/color-controller.h>
#include <dali/public-api/device-layout.h>
#include <dali/public-api/drag-and-drop-detector.h>
#include <dali/public-api/haptic-player.h>
#include <dali/public-api/imf-manager.h>
#include <dali/public-api/key.h>
#include <dali/public-api/orientation.h>
#include <dali/public-api/pixmap-image.h>
#include <dali/public-api/render-surface.h>
#include <dali/public-api/sound-player.h>
#include <dali/public-api/style-change.h>
#include <dali/public-api/style-monitor.h>
#include <dali/public-api/timer.h>
#include <dali/public-api/tts-player.h>
#include <dali/public-api/virtual-keyboard.h>
#include <dali/public-api/window.h>

#include <dali/public-api/accessibility-action-handler.h>
#include <dali/public-api/accessibility-gesture-handler.h>
#include <dali/public-api/event-feeder.h>
#include <dali/public-api/feedback-plugin.h>
#include <dali/public-api/physical-keyboard.h>
#include <dali/public-api/tilt-sensor.h>

#endif //__DALI_H__
