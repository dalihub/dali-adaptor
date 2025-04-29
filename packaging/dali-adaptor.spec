# NOTES
# This spec file is used to build DALi Adaptor for different Tizen Profiles
# Current profiles are:  Mobile, TV, Wearable, Common
#
# The profile variable is defined outside of the spec file in a build.conf file.
# It will contain the profile and whether or not to build with X11 or Wayland
#
# gbs will try to download the build.conf for the platform automatically from the repo location when
# performing a gbs build ( use gbs build -v to see it download location) E.g.
# http://download.tizen.org/snapshots/tizen/tv/tizen-tv/repos/arm-wayland/packages/repodata/xxxx-build.conf.gz

# Do not provide .so automatically for the extensions.
# This if statement is for backward compatibility with GBM/Obsolete build systems
%if "%{?profile}" != "wearable" && "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%global __provides_exclude_from ^.*\\.(wearable|mobile|tv|ivi|common)$
%endif

Name:       dali2-adaptor
Summary:    The DALi Tizen Adaptor
Version:    2.4.16
Release:    1
Group:      System/Libraries
License:    Apache-2.0 and BSD-3-Clause and MIT
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-adaptor.git;a=summary
Source0:    %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:       giflib

%define tizen_platform_config_supported 1
BuildRequires:  pkgconfig(libtzplatform-config)

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
BuildRequires:  pkgconfig(capi-appfw-watch-application)
BuildRequires:  pkgconfig(appcore-watch)
%endif

BuildRequires:  pkgconfig(screen_connector_provider)
BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(glesv2)
BuildRequires:  pkgconfig(ttrace)

BuildRequires:  dali2-devel
BuildRequires:  dali2-integration-devel

BuildRequires:  pkgconfig
BuildRequires:  gawk
BuildRequires:  cmake
BuildRequires:  giflib-devel
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  pkgconfig(vconf)
BuildRequires:  tts-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  libdrm-devel
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  libcurl-devel
BuildRequires:  pkgconfig(harfbuzz)
BuildRequires:  hyphen-devel
BuildRequires:  fribidi-devel

BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(capi-system-sensor)

BuildRequires:  pkgconfig(vulkan)
BuildRequires:  glslang-devel
BuildRequires:  glslang
BuildRequires:  pkgconfig(egl)
BuildRequires:  pkgconfig(wayland-egl)

BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(input-method-client)
BuildRequires:  wayland-devel
BuildRequires:  wayland-extension-client-devel

# WebP support only from Tizen 6 onwards
%if 0%{?tizen_version_major} >= 6
BuildRequires:  pkgconfig(libwebp)
BuildRequires:  pkgconfig(libwebpdecoder)
BuildRequires:  pkgconfig(libwebpdemux)
BuildRequires:  pkgconfig(libwebpmux)
%endif

# We use ecore mainloop
BuildRequires:  pkgconfig(ecore-wl2)
BuildRequires:  pkgconfig(wayland-egl-tizen)

# We need tbm_surface in tizen 3.0 wayland
BuildRequires:  pkgconfig(libtbm)

# for the adaptor
BuildRequires:  pkgconfig(app-core-ui-cpp)
BuildRequires:  pkgconfig(appcore-widget-base)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(capi-appfw-app-common)
BuildRequires:  pkgconfig(capi-appfw-app-control)
BuildRequires:  pkgconfig(ecore-imf)

BuildRequires:  pkgconfig(capi-system-system-settings)

# for ATSPI (Accessibility) support
BuildRequires:  pkgconfig(eldbus)

# for feedback plugin
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(feedback)
BuildRequires:  pkgconfig(component-based-core-base)

BuildRequires:  pkgconfig(thorvg)

# For ASAN test
%if "%{vd_asan}" == "1" || "%{asan}" == "1"
BuildRequires: asan-force-options
BuildRequires: asan-build-env
BuildRequires: libasan
%endif

# for multiprofile
Requires:   %{name}-compat = %{version}-%{release}
Recommends: %{name}-profile_common = %{version}-%{release}

%description
The DALi Tizen Adaptor provides a Tizen specific implementation of the dali-core
platform abstraction and application shell

###########################################
# Vulkan Graphics Backend
###########################################
%package vulkan
Summary:        The DALi Tizen Adaptor with the Vulkan library
Requires:       %{name}
Requires:       glslang
%description vulkan
The DALi Tizen Adaptor with the Vulkan library.

###########################################
# Dali adapter for profiles
###########################################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%package profile_mobile
Summary:        The DALi Tizen Adaptor for mobile
Provides:       %{name}-compat = %{version}-%{release}
Conflicts:      %{name}-profile_tv
Conflicts:      %{name}-profile_wearable
Conflicts:      %{name}-profile_ivi
Conflicts:      %{name}-profile_common
Requires:       %{name}
%description profile_mobile
The DALi Tizen Adaptor for mobile.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%package profile_tv
Summary:        The DALi Tizen Adaptor for tv
Provides:       %{name}-compat = %{version}-%{release}
Conflicts:      %{name}-profile_mobile
Conflicts:      %{name}-profile_wearable
Conflicts:      %{name}-profile_ivi
Conflicts:      %{name}-profile_common
Requires:       %{name}
%description profile_tv
The DALi Tizen Adaptor for tv.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%package profile_wearable
Summary:        The DALi Tizen Adaptor for wearable
Provides:       %{name}-compat = %{version}-%{release}
Conflicts:      %{name}-profile_mobile
Conflicts:      %{name}-profile_tv
Conflicts:      %{name}-profile_ivi
Conflicts:      %{name}-profile_common
Requires:       %{name}
%description profile_wearable
The DALi Tizen Adaptor for wearable.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
%package profile_ivi
Summary:        The DALi Tizen Adaptor for ivi
Provides:       %{name}-compat = %{version}-%{release}
Conflicts:      %{name}-profile_mobile
Conflicts:      %{name}-profile_wearable
Conflicts:      %{name}-profile_tv
Conflicts:      %{name}-profile_common
Requires:       %{name}
%description profile_ivi
The DALi Tizen Adaptor for ivi.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
# Currently Tizen Common we use does not have wayland extensions like xdg-shell
%package profile_common
Summary:        The DALi Tizen Adaptor for common
Provides:       %{name}-compat = %{version}-%{release}
Conflicts:      %{name}-profile_mobile
Conflicts:      %{name}-profile_wearable
Conflicts:      %{name}-profile_tv
Conflicts:      %{name}-profile_ivi
Requires:       %{name}
%description profile_common
The DALi Tizen Adaptor for common.
%endif

##############################
# devel
##############################
%package devel
Summary:    Development components for the DALi Tizen Adaptor
Group:      Development/Building
Requires:   %{name} = %{version}-%{release}
Requires:   %{name}-integration-devel = %{version}-%{release}

%description devel
Development components for the DALi Tizen Adaptor - public headers and package configs

##############################
# integration-devel
##############################
%package integration-devel
Summary:    Integration development package for the Adaptor
Group:      Development/Building
Requires:   %{name} = %{version}-%{release}

%description integration-devel
Integration development package for the Adaptor - headers for integrating with an adaptor library.

##############################
# Dali Feedback Plugin
##############################
%package dali2-feedback-plugin
Summary:    Plugin to play haptic and audio feedback for Dali
Group:      System/Libraries
Requires:   %{name} = %{version}-%{release}
%description dali2-feedback-plugin
Feedback plugin to play haptic and audio feedback for Dali

#Use TZ_PATH when tizen version is 3.x or greater
%define dali_data_rw_dir         %TZ_SYS_RO_SHARE/dali/
%define dali_data_ro_dir         %TZ_SYS_RO_SHARE/dali/
%define font_preloaded_path      %TZ_SYS_RO_SHARE/fonts/
%define font_downloaded_path     %TZ_SYS_SHARE/fonts/
%define font_application_path    %TZ_SYS_RO_SHARE/app_fonts/
%define font_configuration_file  %TZ_SYS_ETC/fonts/conf.avail/99-slp.conf

%define user_shader_cache_dir    %{dali_data_ro_dir}/core/shaderbin/
%define dali_plugin_sound_files  /plugins/sounds/

##############################
# Preparation
##############################
%prep
%setup -q


##############################
# Build
##############################
%build
PREFIX+="/usr"
CXXFLAGS+=" -Wall -g -Os -fPIC -fvisibility-inlines-hidden -fdata-sections -ffunction-sections -DGL_GLEXT_PROTOTYPES -Wno-psabi"
LDFLAGS+=" -Wl,--rpath=%{_libdir} -Wl,--as-needed -Wl,--gc-sections -lttrace -Wl,-Bsymbolic-functions "

%ifarch %{arm}
CXXFLAGS+=" -D_ARCH_ARM_ -lgcc"
%endif

CFLAGS+=" -DWAYLAND -DEFL_BETA_API_SUPPORT"
CXXFLAGS+=" -DWAYLAND -DEFL_BETA_API_SUPPORT"
cmake_flags=" -DENABLE_WAYLAND=ON -DENABLE_ATSPI=ON"

%if 0%{?enable_streamline}
cmake_flags+=" -DENABLE_TRACE_STREAMLINE=ON"
%else
cmake_flags+=" -DENABLE_TRACE=ON"
%endif

# Use this conditional when Tizen version is 7.x or greater
%if 0%{?tizen_version_major} >= 7
CXXFLAGS+=" -DOVER_TIZEN_VERSION_7"
%endif

# Use this conditional when Tizen version is 8.x or greater
%if 0%{?tizen_version_major} >= 8
CXXFLAGS+=" -DOVER_TIZEN_VERSION_8"
%endif

# Use this conditional when Tizen version is 9.x or greater
%if 0%{?tizen_version_major} >= 9
CXXFLAGS+=" -DOVER_TIZEN_VERSION_9"
%endif

# Use this conditional when Tizen version is 10.x or greater
%if 0%{?tizen_version_major} >= 10
CXXFLAGS+=" -DOVER_TIZEN_VERSION_10"
%endif

%if "%{vd_asan}" == "1" || "%{asan}" == "1"
CFLAGS+=" -fsanitize=address"
CXXFLAGS+=" -fsanitize=address"
LDFLAGS+=" -fsanitize=address"
%endif

%if 0%{?enable_debug}
cmake_flags+=" -DCMAKE_BUILD_TYPE=Debug"
%endif

%if 0%{?enable_logging}
cmake_flags+=" -DENABLE_NETWORK_LOGGING=ON"
%endif

libtoolize --force
cd %{_builddir}/%{name}-%{version}/build/tizen

DALI_DATA_RW_DIR="%{dali_data_rw_dir}" ; export DALI_DATA_RW_DIR
DALI_DATA_RO_DIR="%{dali_data_ro_dir}"  ; export DALI_DATA_RO_DIR
FONT_PRELOADED_PATH="%{font_preloaded_path}" ; export FONT_PRELOADED_PATH
FONT_DOWNLOADED_PATH="%{font_downloaded_path}" ; export FONT_DOWNLOADED_PATH
FONT_APPLICATION_PATH="%{font_application_path}"  ; export FONT_APPLICATION_PATH
FONT_CONFIGURATION_FILE="%{font_configuration_file}" ; export FONT_CONFIGURATION_FILE
%if 0%{?tizen_platform_config_supported}
TIZEN_PLATFORM_CONFIG_SUPPORTED="%{tizen_platform_config_supported}" ; export TIZEN_PLATFORM_CONFIG_SUPPORTED
%endif

cmake_flags+=" -DCMAKE_INSTALL_PREFIX=$PREFIX"
cmake_flags+=" -DCMAKE_INSTALL_LIBDIR=%{_libdir}"
cmake_flags+=" -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir}"
cmake_flags+=" -DENABLE_TIZEN_MAJOR_VERSION=%{tizen_version_major}"
cmake_flags+=" -DENABLE_FEEDBACK=YES"
cmake_flags+=" -DENABLE_APPMODEL=ON"
cmake_flags+=" -DENABLE_APPFW=YES"
cmake_flags+=" -DCOMPONENT_APPLICATION_SUPPORT=YES"

# Set up the build via Cmake
#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"

mkdir -p mobile
pushd mobile

cmake -DENABLE_PROFILE=MOBILE $cmake_flags ..

# Build.
make %{?jobs:-j%jobs}
popd

%endif

#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"

mkdir -p tv
pushd tv

cmake -DENABLE_PROFILE=TV $cmake_flags ..

# Build.
make %{?jobs:-j%jobs}
popd

%endif

#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"

mkdir -p wearable
pushd wearable

cmake -DENABLE_PROFILE=WEARABLE $cmake_flags ..

# Build.
make %{?jobs:-j%jobs}
popd

%endif

#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"

mkdir -p ivi
pushd ivi

cmake -DENABLE_PROFILE=IVI $cmake_flags ..

# Build.
make %{?jobs:-j%jobs}
popd

%endif

#######################################################################
# common
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"

mkdir -p common
pushd common

cmake -DENABLE_PROFILE=COMMON $cmake_flags ..

# Build.
make %{?jobs:-j%jobs}
popd

%endif

##############################
# Installation
##############################
%install
rm -rf %{buildroot}

pushd %{_builddir}/%{name}-%{version}/build/tizen

# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
pushd mobile
%make_install
%if "%{?profile}" != "mobile"
pushd  %{buildroot}%{_libdir}
cp libdali2-adaptor.so.*.*.* libdali2-adaptor.so.mobile # If we're only building this profile, then there's no need to copy the lib
popd
make clean # So that we can gather symbol/size information for only one profile if we're building all profiles
%endif
popd
%endif

# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
pushd tv
%make_install
%if "%{?profile}" != "tv"
pushd  %{buildroot}%{_libdir}
cp libdali2-adaptor.so.*.*.* libdali2-adaptor.so.tv # If we're only building this profile, then there's no need to copy the lib
popd
make clean # So that we can gather symbol/size information for only one profile if we're building all profiles
%endif
popd
%endif

# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
pushd wearable
%make_install
%if "%{?profile}" != "wearable"
pushd  %{buildroot}%{_libdir}
cp libdali2-adaptor.so.*.*.* libdali2-adaptor.so.wearable # If we're only building this profile, then there's no need to copy the lib
popd
make clean # So that we can gather symbol/size information for only one profile if we're building all profiles
%endif
popd
%endif

# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
pushd ivi
%make_install
%if "%{?profile}" != "ivi"
pushd  %{buildroot}%{_libdir}
cp libdali2-adaptor.so.*.*.* libdali2-adaptor.so.ivi # If we're only building this profile, then there's no need to copy the lib
popd
make clean # So that we can gather symbol/size information for only one profile if we're building all profiles
%endif
popd
%endif

# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
pushd common
%make_install
# No clean so we can gather symbol/size information for the common profile
popd
%endif

# Create a symbolic link in integration-api to preserve legacy repo build
pushd %{buildroot}%{_includedir}/dali/integration-api
ln -sf adaptor-framework adaptors
popd

##############################
# Upgrade order:
# 1 - Pre Install new package
# 2 - Install new package
# 3 - Post install new package
# 4 - Pre uninstall old package
# 5 - Remove files not overwritten by new package
# 6 - Post uninstall old package
##############################

##############################
# Adaptor package Commands
%pre
exit 0

%post
pushd %{_libdir}
for i in mobile tv wearable ivi; do [[ -f libdali2-adaptor.so.$i ]] && ln -sf libdali2-adaptor.so.$i libdali2-adaptor.so.2.0.0; done
popd
/sbin/ldconfig
rm -rf /home/owner/.cache/dali_common_caches/shader/ # this code is used to clear all existing binaries when installing Tizen packages. see build/tizen/shader-cache-path.in.
exit 0

%preun
exit 0

%postun
/sbin/ldconfig
exit 0

##############################
# Mobile Profile Commands
# if mobile || "undefined"
# No need to create a symbolic link on install required if only building this profile
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%post profile_mobile
%if "%{?profile}" != "mobile"
pushd %{_libdir}
ln -sf libdali2-adaptor.so.mobile libdali2-adaptor.so.2.0.0
popd
%endif
/sbin/ldconfig
exit 0

%postun profile_mobile
/sbin/ldconfig
exit 0
%endif

##############################
# TV Profile Commands
# No need to create a symbolic link on install required if only building this profile
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%post profile_tv
%if "%{?profile}" != "tv"
pushd %{_libdir}
ln -sf libdali2-adaptor.so.tv libdali2-adaptor.so.2.0.0
popd
%endif
/sbin/ldconfig
exit 0

%postun profile_tv
/sbin/ldconfig
exit 0
%endif

##############################
# Wearable Profile Commands
# No need to create a symbolic link on install required if only building this profile
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%post profile_wearable
%if "%{?profile}" != "wearable"
pushd %{_libdir}
ln -sf libdali2-adaptor.so.wearable libdali2-adaptor.so.2.0.0
popd
%endif
/sbin/ldconfig
exit 0

%postun profile_wearable
/sbin/ldconfig
exit 0
%endif

##############################
# IVI Profile Commands
# No need to create a symbolic link on install required if only building this profile
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
%post profile_ivi
%if "%{?profile}" != "ivi"
pushd %{_libdir}
ln -sf libdali2-adaptor.so.ivi libdali2-adaptor.so.2.0.0
popd
%endif
/sbin/ldconfig
exit 0

%postun profile_ivi
/sbin/ldconfig
exit 0
%endif

##############################
# Common Profile Commands
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%post profile_common
/sbin/ldconfig
exit 0

%postun profile_common
/sbin/ldconfig
exit 0
%endif

##############################
# Files in Binary Packages
##############################

%files
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%dir %{user_shader_cache_dir}
%{_bindir}/*
%license LICENSE
%defattr(-,root,root,-)
%{_libdir}/libdali2-adaptor.so
%{_libdir}/libdali2-adaptor.so.2
%{_libdir}/libdali2-adaptor.so.2.0.0
%{_libdir}/libdali2-adaptor-gles.so
%{_libdir}/libdali2-adaptor-gl-window-addon.so
%{_libdir}/libdali2-adaptor-application-normal.so*
%{_libdir}/libdali2-adaptor-application-widget.so*
%{_libdir}/libdali2-adaptor-application-watch.so*
%{_libdir}/libdali2-adaptor-application-component-based.so*

#################################################

%files vulkan
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-adaptor-vulkan.so

#################################################

%files dali2-feedback-plugin
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-feedback-plugin.so*
%{dali_plugin_sound_files}/*

#################################################

# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%files profile_common
%manifest dali-adaptor.manifest
# default .so files are housed in the main pkg.
%endif

# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%files profile_mobile
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%if "%{?profile}" != "mobile"
%{_libdir}/libdali2-adaptor.so.mobile
%endif
%endif

# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%files profile_tv
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%if "%{?profile}" != "tv"
%{_libdir}/libdali2-adaptor.so.tv
%endif
%endif

# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%files profile_wearable
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%if "%{?profile}" != "wearable"
%{_libdir}/libdali2-adaptor.so.wearable
%endif
%endif

# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
%files profile_ivi
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%if "%{?profile}" != "ivi"
%{_libdir}/libdali2-adaptor.so.ivi
%endif
%endif

%files devel
%defattr(-,root,root,-)
%{_includedir}/dali/dali.h
%{_includedir}/dali/public-api/*
%{_includedir}/dali/devel-api/*
%{_includedir}/dali/doc/*
%{_libdir}/pkgconfig/dali2-adaptor.pc

%files integration-devel
%defattr(-,root,root,-)
%{_includedir}/dali/integration-api/adaptor-framework/*
%{_includedir}/dali/integration-api/adaptors
%{_libdir}/pkgconfig/dali2-adaptor-integration.pc
