#
# Copyright (c) 2014 Samsung Electronics Co., Ltd.
#
# Licensed under the Flora License, Version 1.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://floralicense.org/license/
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Build the Dali feedback plugin

plugin_src_dir = ../../../plugins

include ../../../plugins/sounds/file.list
include ../../../plugins/file.list

plugin_sounds_dir = ../../../plugins/sounds

lib_LTLIBRARIES = libdali-feedback-plugin.la

if USE_BULLET
lib_LTLIBRARIES += libdali-bullet-plugin.la
endif

dalisounddir = ${dataReadOnlyDir}/plugins/sounds/
dalisound_DATA = ${dali_plugin_sound_files}

# Feedback
libdali_feedback_plugin_la_SOURCES = \
                     $(plugin_src_files)

libdali_feedback_plugin_la_DEPENDENCIES =

libdali_feedback_plugin_la_CXXFLAGS = -DDALI_COMPILATION \
                      -DDALI_SOUND_DIR="\"${dalisounddir}\"" \
                      $(DLOG_CFLAGS) \
                      $(DALICORE_CFLAGS) \
                      $(MMFSOUND_CFLAGS) \
                      $(HAPTIC_CFLAGS) \
                      $(FEEDBACK_CFLAGS) \
                      -I../../../adaptors/tizen \
                      -Werror -Wall

libdali_feedback_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(MMFSOUND_LIBS) \
                      $(HAPTIC_LIBS) \
                      $(FEEDBACK_LIBS)

libdali_feedback_plugin_la_LDFLAGS = \
                      -rdynamic

# Bullet Physics
libdali_bullet_plugin_la_SOURCES = \
                      $(bullet_plugin_src_files)

libdali_bullet_plugin_la_CXXFLAGS = -DDALI_COMPILATION \
                      $(DALICORE_CFLAGS) \
                      $(DLOG_CFLAGS) \
                      $(BULLET_CFLAGS) \
                      -I../../.. \
                      -Werror -Wall

libdali_bullet_plugin_la_LIBADD = \
                      $(DLOG_LIBS) \
                      $(BULLET_LIBS)

libdali_bullet_plugin_la_LDFLAGS = \
                      -rdynamic

libdali_bullet_plugin_la_DEPENDENCIES =
