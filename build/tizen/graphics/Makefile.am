# Copyright (c) 2019 Samsung Electronics Co., Ltd.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Parse each file.list
# These files define <module>_src_files & <module>_header_files,
# where <module> is the subdirectory from core_src_dir

#graphics_api_src_dir = ../../../../dali/graphics-api
#include ../../../../dali/graphics-api/file.list

graphics_src_dir = ../../../dali/graphics/
include ../../../dali/graphics/file.list

graphics_integration_dir = ../../../dali/integration-api/graphics
include ../../../dali/integration-api/graphics/file.list

# Build the graphics library
lib_LTLIBRARIES=

if ENABLE_CXX03_ABI

lib_LTLIBRARIES  += libdali-graphics-vk.la

else # ENABLE_CXX03_ABI
lib_LTLIBRARIES  += libdali-graphics-vk-cxx11.la

endif # ENABLE_CXX03_ABI


dali_includes = \
                     -I../../..

gcc_flags = -Wno-return-local-addr -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override \
            -Wstack-usage=256 -Wunsafe-loop-optimizations -Wzero-as-null-pointer-constant -Wuseless-cast

# -Wfloat-equal causes issues with vulkan.hpp, removed for now
cxx_more_warnings = -Wold-style-cast -Woverloaded-virtual -Wdouble-promotion -Wswitch-enum \
                    -Wshadow \
                    -Wcast-qual -Wcast-align \
                    -Wconversion -Wsign-conversion

#-Wlarger-than=1024  # Conflicts with --coverage

# the following warnings should not be enforced
cxx_warnings_to_remove = \
                         -Wno-c++98-compat \
                         -Wno-unused-parameter \
                         -Wno-unknown-warning-option \
                         -Wno-switch \
                         -Wno-switch-enum \
                         -Wno-error=switch \
                         -Wno-error=switch-enum

#the following warnings should be added back when possible
cxx_warnings_to_preserve = \
                           -Wno-weak-vtables

# vulkan.hpp doesn't pass all -Weffc++ checks, don't add it :/
cxx_flags = -fPIC -std=c++14 -Werror -Wall -Wextra -Wpedantic \
            $(cxx_more_warnings) $(cxx_warnings_to_remove) $(cxx_warnings_to_preserve)

if VULKAN_ENABLED
    vulkan_flags = -DVULKAN_HPP_NO_EXCEPTIONS
endif


if ENABLE_CXX03_ABI

libdali_graphics_vk_la_CXXFLAGS = $(cxx_flags) \
                               $(dali_includes) \
                               $(DALI_CORE_CFLAGS) \
                               $(DALI_ADAPTOR_CFLAGS) \
                               $(VULKAN_CFLAGS) \
                               $(vulkan_flags)

libdali_graphics_vk_la_SOURCES = $(graphics_src_files) \
                                 $(graphics_integration_src_files)

libdali_graphics_vk_la_LDFLAGS = -fPIC

libdali_graphics_vk_la_LIBADD = $(VULKAN_LIBS)


else # ENABLE_CXX03_ABI
libdali_graphics_vk_cxx11_la_CXXFLAGS = $(cxx_flags) \
                                        $(dali_includes) \
                                        $(DALI_CORE_CFLAGS) \
                                        $(DALI_ADAPTOR_CFLAGS) \
                                        $(VULKAN_CFLAGS) \
                                        $(vulkan_flags)

libdali_graphics_vk_cxx11_la_SOURCES = $(graphics_src_files) \
                                    $(graphics_integration_src_files)

libdali_graphics_vk_cxx11_la_LDFLAGS = -fPIC

libdali_graphics_vk_cxx11_la_LIBADD = $(VULKAN_LIBS)

endif # ENABLE_CXX03_ABI
