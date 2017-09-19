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

if ENABLE_CXX03_ABI

if USE_FEEDBACK
lib_LTLIBRARIES += libdali-feedback-plugin.la
endif

if USE_WEBENGINELITE
lib_LTLIBRARIES += libdali-web-engine-lite-plugin.la
endif

if WAYLAND
if USE_VIDEOPLAYER
lib_LTLIBRARIES += libdali-video-player-plugin.la
endif
endif

else

if USE_FEEDBACK
lib_LTLIBRARIES += libdali-feedback-plugin-cxx11.la
endif

if USE_WEBENGINELITE
lib_LTLIBRARIES += libdali-web-engine-lite-plugin-cxx11.la
endif

if WAYLAND

if USE_VIDEOPLAYER
lib_LTLIBRARIES += libdali-video-player-plugin-cxx11.la
endif
endif
endif

if USE_FEEDBACK
dalisounddir = ${dataReadOnlyDir}/plugins/sounds/
dalisound_DATA = ${dali_plugin_sound_files}

# Feedback
LIBDALI_FEEDBACK_PLUGIN_LA_SOURCES = \
                     $(feedback_plugin_src_files)


libdali_feedback_plugin_la_DEPENDENCIES =

LIBDALI_FEEDBACK_PLUGIN_LA_CXXFLAGS = -DDALI_COMPILATION \
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

LIBDALI_FEEDBACK_PLUGIN_LA_LIBADD = \
                      $(DLOG_LIBS) \
                      $(MMFSOUND_LIBS)

LIBDALI_FEEDBACK_PLUGIN_LA_LDFLAGS = \
                      -rdynamic

LIBDALI_FEEDBACK_PLUGIN_LA_LIBADD += \
                      $(FEEDBACK_LIBS)

endif

if USE_WEBENGINELITE
LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_SOURCES = \
                     $(web_engine_lite_plugin_src_files)

libdali_web_engine_lite_plugin_la_DEPENDENCIES =

LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_CXXFLAGS = -DDALI_COMPILATION \
                      $(DLOG_CFLAGS) \
                      $(DALICORE_CFLAGS) \
                      $(WEB_ENGINE_LITE_CFLAGS) \
                      $(DALI_PROFILE_CFLAGS) \
                      $(DALI_ADAPTOR_CFLAGS) \
                      -I../../../adaptors/public-api \
                      -I../../../adaptors/integration-api \
                      -I../../../adaptors/public-api/adaptor-framework \
                      -I../../../adaptors/devel-api/adaptor-framework \
                      -Werror -Wall

LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_LIBADD = \
                      $(DLOG_LIBS) \
                      -llightweight-web-engine

LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_LDFLAGS = \
                      -rdynamic

endif # WEB_ENGINE_LITE

# VideoPlayer
if WAYLAND
if USE_VIDEOPLAYER

LIBDALI_VIDEO_PLAYER_PLUGIN_LA_SOURCES = \
                     $(video_player_plugin_src_files)

libdali_video_player_plugin_la__DEPENDENCIES =

LIBDALI_VIDEO_PLAYER_PLUGIN_LA_CXXFLAGS = -DDALI_COMPILATION \
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

LIBDALI_VIDEO_PLAYER_PLUGIN_LA_LIBADD = \
                      $(DLOG_LIBS) \
                      $(ECORE_WAYLAND_LIBS) \
                      $(CAPI_MEDIA_PLAYER_LIBS)

LIBDALI_VIDEO_PLAYER_PLUGIN_LA_LDFLAGS = \
                      -rdynamic

endif # USE_VIDEOPLAYER
endif # WAYLAND

if ENABLE_CXX03_ABI

if USE_FEEDBACK
libdali_feedback_plugin_la_SOURCES = $(LIBDALI_FEEDBACK_PLUGIN_LA_SOURCES)
libdali_feedback_plugin_la_LIBADD = $(LIBDALI_FEEDBACK_PLUGIN_LA_LIBADD)
libdali_feedback_plugin_la_CXXFLAGS = $(LIBDALI_FEEDBACK_PLUGIN_LA_CXXFLAGS)
libdali_feedback_plugin_la_LDFLAGS = $(LIBDALI_FEEDBACK_PLUGIN_LA_LDFLAGS)

if ENABLE_RENAME_SO
#rename
install: install-am
	rm -rf $(libdir)/libdali_feedback_plugin.so
	rm -rf $(libdir)/libdali_feedback_plugin-cxx03.so
	ln -s $(libdir)/libdali_feedback_plugin.so.0.0.* $(libdir)/libdali_feedback_plugin-cxx03.so
endif
endif

if USE_WEBENGINELITE
libdali_web_engine_lite_plugin_la_SOURCES = $(LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_SOURCES)
libdali_web_engine_lite_plugin_la_LIBADD = $(LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_LIBADD)
libdali_web_engine_lite_plugin_la_CXXFLAGS = $(LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_CXXFLAGS)
libdali_web_engine_lite_plugin_la_LDFLAGS = $(LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_LDFLAGS)

if ENABLE_RENAME_SO
#rename
install: install-am
	rm -rf $(libdir)/libdali_web_engine_lite_plugin.so
	rm -rf $(libdir)/libdali_web_engine_lite_plugin-cxx03.so
	ln -s $(libdir)/libdali_web_engine_lite_plugin.so.0.0.* $(libdir)/libdali_web_engine_lite_plugin-cxx03.so
endif
endif

if WAYLAND
if USE_VIDEOPLAYER
libdali_video_player_plugin_la_SOURCES = $(LIBDALI_VIDEO_PLAYER_PLUGIN_LA_SOURCES)
libdali_video_player_plugin_la_CXXFLAGS = $(LIBDALI_VIDEO_PLAYER_PLUGIN_LA_CXXFLAGS)
libdali_video_player_plygin_la_LIBADD = $(LIBDALI_VIDEO_PLAYER_PLUGIN_LA_LIBADD)
libdali_video_player_plugin_la_LDFLAGS = $(LIBDALI_VIDEO_PLAYER_PLUGIN_LA_LDFLAGS)

if ENABLE_RENAME_SO
#rename
install: install-am
	rm -rf $(libdir)/libdali_video_player_plugin.so
	rm -rf $(libdir)/libdali_video_player_plugin-cxx03.so
	ln -s $(libdir)/libdali_video_player_plugin.so.0.0.* $(libdir)/libdali_video_player_plugin-cxx03.so
endif
endif
endif

else # CXX03

if USE_FEEDBACK
libdali_feedback_plugin_cxx11_la_SOURCES = $(LIBDALI_FEEDBACK_PLUGIN_LA_SOURCES)
libdali_feedback_plugin_cxx11_la_LIBADD = $(LIBDALI_FEEDBACK_PLUGIN_LA_LIBADD)
libdali_feedback_plugin_cxx11_la_CXXFLAGS = $(LIBDALI_FEEDBACK_PLUGIN_LA_CXXFLAGS)
libdali_feedback_plugin_cxx11_la_LDFLAGS = $(LIBDALI_FEEDBACK_PLUGIN_LA_LDFLAGS)

if ENABLE_RENAME_SO
#rename
install: install-am
	rm -rf $(libdir)/libdali_feedback_plugin-cxx11.so
	rm -rf $(libdir)/libdali_feedback_plugin.so
	ln -s $(libdir)/libdali_feedback_plugin-cxx11.so.0.0.* $(libdir)/libdali_feedback_plugin.so
endif
endif

if USE_WEBENGINELITE
libdali_web_engine_lite_plugin_cxx11_la_SOURCES = $(LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_SOURCES)
libdali_web_engine_lite_plugin_cxx11_la_LIBADD = $(LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_LIBADD)
libdali_web_engine_lite_plugin_cxx11_la_CXXFLAGS = $(LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_CXXFLAGS)
libdali_web_engine_lite_plugin_cxx11_la_LDFLAGS = $(LIBDALI_WEB_ENGINE_LITE_PLUGIN_LA_LDFLAGS)

if ENABLE_RENAME_SO
#rename
install: install-am
	rm -rf $(libdir)/libdali_web_engine_lite_plugin-cxx11.so
	rm -rf $(libdir)/libdali_web_engine_lite_plugin.so
	ln -s $(libdir)/libdali_web_engine_lite_plugin-cxx11.so.0.0.* $(libdir)/libdali_web_engine_lite_plugin.so
endif
endif

if WAYLAND
if USE_VIDEOPLAYER
libdali_video_player_plugin_cxx11_la_SOURCES = $(LIBDALI_VIDEO_PLAYER_PLUGIN_LA_SOURCES)
libdali_video_player_plugin_cxx11_la_CXXFLAGS = $(LIBDALI_VIDEO_PLAYER_PLUGIN_LA_CXXFLAGS)
libdali_video_player_plygin_cxx11_la_LIBADD = $(LIBDALI_VIDEO_PLAYER_PLUGIN_LA_LIBADD)
libdali_video_player_plugin_cxx11_la_LDFLAGS = $(LIBDALI_VIDEO_PLAYER_PLUGIN_LA_LDFLAGS)

if ENABLE_RENAME_SO
#rename
install: install-am
	rm -rf $(libdir)/libdali_video_player_plugin-cxx11.so
	rm -rf $(libdir)/libdali_video_player_plugin.so
	ln -s $(libdir)/libdali_video_player_plugin-cxx11.so.0.0.* $(libdir)/libdali_video_player_plugin.so
endif
endif
endif

endif # CXX03
