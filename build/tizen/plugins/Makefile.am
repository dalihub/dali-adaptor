#
# Copyright (c) 2015 Samsung Electronics Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Build the Dali feedback plugin

plugin_src_dir = ../../../plugins

include ../../../plugins/sounds/file.list
include ../../../plugins/file.list

plugin_sounds_dir = ../../../plugins/sounds

lib_LTLIBRARIES =

if USE_FEEDBACK
lib_LTLIBRARIES += libdali-feedback-plugin.la
endif

if WAYLAND
if USE_VIDEOPLAYER
lib_LTLIBRARIES += libdali-video-player-plugin.la
endif
endif

if USE_FEEDBACK
dalisounddir = ${dataReadOnlyDir}/plugins/sounds/
dalisound_DATA = ${dali_plugin_sound_files}

# Feedback
libdali_feedback_plugin_la_SOURCES = \
                     $(feedback_plugin_src_files)

libdali_feedback_plugin_la_DEPENDENCIES =

libdali_feedback_plugin_la_CXXFLAGS = -DDALI_COMPILATION \
                      -DDALI_SOUND_DIR="\"${dalisounddir}\"" \
                      $(DLOG_CFLAGS) \
                      $(DALICORE_CFLAGS) \
                      $(MMFSOUND_CFLAGS) \
                      $(FEEDBACK_CFLAGS) \
                      $(DALI_PROFILE_CFLAGS) \
                      $(DALI_ADAPTOR_CFLAGS) \
                      -I../../../adaptors/public-api \
                      -I../../../adaptors/integration-api \
                      -I../../../adaptors/public-api/adaptor-framework \
                      -I../../../adaptors/devel-api/adaptor-framework \
                      -Werror -Wall

libdali_feedback_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(MMFSOUND_LIBS)

libdali_feedback_plugin_la_LDFLAGS = \
                      -rdynamic

libdali_feedback_plugin_la_LIBADD += \
                      $(FEEDBACK_LIBS)

endif

# VideoPlayer
if WAYLAND
if USE_VIDEOPLAYER

libdali_video_player_plugin_la_SOURCES = \
                     $(video_player_plugin_src_files)

libdali_video_player_plugin_la_DEPENDENCIES =

libdali_video_player_plugin_la_CXXFLAGS = -DDALI_COMPILATION \
                      -DDALI_ADAPTOR_COMPILATION \
                      $(DLOG_CFLAGS) \
                      $(DALICORE_CFLAGS) \
                      $(ECORE_WAYLAND_CFLAGS) \
                      $(CAPI_MEDIA_PLAYER_CFLAGS) \
                      $(DALI_PROFILE_CFLAGS) \
                      $(DALI_ADAPTOR_CFLAGS) \
                      -I../../../adaptors/public-api \
                      -I../../../adaptors/devel-api \
                      -I../../../adaptors/devel-api/adaptor-framework \
                      -I../../../adaptors/integration-api \
                      -I../../../adaptors/public-api/adaptor-framework \
                      -I../../../plugins/video-player \
                      -Werror -Wall

libdali_video_player_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(ECORE_WAYLAND_LIBS) \
                      $(CAPI_MEDIA_PLAYER_LIBS)

libdali_video_player_plugin_la_LDFLAGS = \
                      -rdynamic

endif # USE_VIDEOPLAYER
endif # WAYLAND
