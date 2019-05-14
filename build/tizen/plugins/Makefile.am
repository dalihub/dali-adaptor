#
# Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
lib_LTLIBRARIES += libdali-feedback-plugin-cxx11.la
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
                      -I../../../ \
                      -Werror -Wall

LIBDALI_FEEDBACK_PLUGIN_LA_LIBADD = \
                      $(DLOG_LIBS) \
                      $(MMFSOUND_LIBS)

LIBDALI_FEEDBACK_PLUGIN_LA_LDFLAGS = \
                      -rdynamic

LIBDALI_FEEDBACK_PLUGIN_LA_LIBADD += \
                      $(FEEDBACK_LIBS)

endif

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

