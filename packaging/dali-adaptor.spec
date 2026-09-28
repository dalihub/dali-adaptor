# NOTES
# This spec file builds one DALi Adaptor, used by every Tizen profile.

Name:       dali2-adaptor
Summary:    The DALi Tizen Adaptor
Version:    2.5.41
Release:    1
Group:      System/Libraries
License:    Apache-2.0 and BSD-3-Clause and MIT and CC0-1.0 and Unlicense and Zlib
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-adaptor.git;a=summary
Source0:    %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:       giflib

%define tizen_platform_config_supported 1
# Tizen Wayland backend: ECORE or TCORE. Example: gbs build ... --define "tizen_wayland_backend TCORE"
# Default ECORE until tcore backend is ready. Tizen < 11: always ECORE (TCORE override ignored).
%{!?tizen_wayland_backend: %global tizen_wayland_backend ECORE}
%if 0%{?tizen_version_major} < 11
%global tizen_wayland_backend ECORE
%endif
BuildRequires:  pkgconfig(libtzplatform-config)

# if 'mv_prj' is defined, this build targets the robot profile.
%if "%{mv_prj}" != "1"
%if "%{tizen_wayland_backend}" == "TCORE"
BuildRequires:  pkgconfig(screen_connector_provider_tcore)
%else
BuildRequires:  pkgconfig(screen_connector_provider)
%endif
%endif

BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(glesv2)
BuildRequires:  pkgconfig(ttrace)

BuildRequires:  dali2-devel
BuildRequires:  dali2-integration-devel

BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(rpc-port)
BuildRequires:  gawk
BuildRequires:  cmake
BuildRequires:  python3
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
BuildRequires:  pkgconfig(capi-web-url-download)
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

# We use ecore mainloop (ECORE) or tizen-core (TCORE)
%if "%{?tizen_wayland_backend}" == "TCORE"
BuildRequires:  pkgconfig(tizen-core)
BuildRequires:  pkgconfig(tizen-core-wl)
%else
BuildRequires:  pkgconfig(ecore-wl2)
%endif
BuildRequires:  pkgconfig(wayland-egl-tizen)

# We need tbm_surface in tizen 3.0 wayland
BuildRequires:  pkgconfig(libtbm)

# for the adaptor
BuildRequires:  pkgconfig(capi-appfw-app-control)
BuildRequires:  pkgconfig(capi-appfw-app-common)
%if "%{tizen_wayland_backend}" == "TCORE"
BuildRequires:  pkgconfig(app-core-tcore-cpp)
%else
BuildRequires:  pkgconfig(app-core-ui-cpp)
%endif
BuildRequires:  pkgconfig(app-core-cpp)
BuildRequires:  pkgconfig(appcore-common)
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-creds-self)

%if "%{mv_prj}" != "1"
%if "%{tizen_wayland_backend}" == "TCORE"
BuildRequires:  pkgconfig(appcore-widget-base-tcore)
%else
BuildRequires:  pkgconfig(appcore-widget-base)
%endif
BuildRequires:  pkgconfig(component-based-core-base)
%endif

BuildRequires:  pkgconfig(bundle)
%if "%{?tizen_wayland_backend}" == "TCORE"
BuildRequires:  pkgconfig(tizen-core-imf)
%else
BuildRequires:  pkgconfig(ecore-imf)
%endif

BuildRequires:  pkgconfig(capi-system-system-settings)

%if 0%{?tizen_version_major} >= 11
Requires: capi-system-system-settings-util-lib
%endif

# for ATSPI (Accessibility) support
BuildRequires:  pkgconfig(eldbus)

# for feedback plugin
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(feedback)

BuildRequires:  pkgconfig(thorvg)

# For ASAN test
%if "%{vd_asan}" == "1" || "%{asan}" == "1"
BuildRequires: asan-force-options
BuildRequires: asan-build-env
BuildRequires: libasan
%endif

# Absorbs the profile_mobile, profile_tv and profile_common packages an image
# may still have installed. The library they carried is in this package.
Provides:   %{name}-profile_common = %{version}-%{release}
Provides:   %{name}-profile_mobile = %{version}-%{release}
Provides:   %{name}-profile_tv = %{version}-%{release}
Obsoletes:  %{name}-profile_common < %{version}-%{release}
Obsoletes:  %{name}-profile_mobile < %{version}-%{release}
Obsoletes:  %{name}-profile_tv < %{version}-%{release}

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

##############################
# devel
##############################
%package devel
Summary:    Development components for the DALi Tizen Adaptor
Group:      Development/Building
Requires:   %{name} = %{version}-%{release}

%description devel
Development components for the DALi Tizen Adaptor - public headers and package configs

##############################
# integration-devel
##############################
%package integration-devel
Summary:    Integration development package for the Adaptor
Group:      Development/Building
Requires:   %{name} = %{version}-%{release}
Requires:   %{name}-devel = %{version}-%{release}

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

%define user_shader_cache_dir %{dali_data_ro_dir}/core/shaderbin/
%define system_cache_dir      /home/owner/.cache/dali_common_caches/

%define dali_plugin_sound_files_install_dir /plugins/sounds/

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

# Tizen Wayland backend (see default at top of spec)
%if "%{tizen_wayland_backend}" == "TCORE"
cmake_flags+=" -DTIZEN_WAYLAND_BACKEND=TCORE"
%else
cmake_flags+=" -DTIZEN_WAYLAND_BACKEND=ECORE"
%endif

# Enable preinitialized adaptor as default if not defined
%{!?enable_preinitialize_adaptor: %global enable_preinitialize_adaptor 1}

%if 0%{?enable_preinitialize_adaptor}
cmake_flags+=" -DENABLE_PREINITIALIZE_ADAPTOR=ON"
%else
cmake_flags+=" -DENABLE_PREINITIALIZE_ADAPTOR=OFF"
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
cmake_flags+=" -DENABLE_WARNING_TO_ERROR=ON"
%else
cmake_flags+=" -DENABLE_WARNING_TO_ERROR=OFF"
%endif

# Use this conditional when Tizen version is 11.x or greater
%if 0%{?tizen_version_major} >= 11
CXXFLAGS+=" -DOVER_TIZEN_VERSION_11"
%endif

%if "%{vd_asan}" == "1" || "%{asan}" == "1"
CFLAGS+=" -fsanitize=address"
CXXFLAGS+=" -fsanitize=address"
LDFLAGS+=" -fsanitize=address"
cmake_flags+=" -DENABLE_ASAN=ON"
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

%if "%{mv_prj}" != "1"
cmake_flags+=" -DROBOT_PROFILE=NO"
%else
cmake_flags+=" -DROBOT_PROFILE=YES"
%endif

cmake_flags+=" -DENABLE_APPFW=YES"
cmake_flags+=" -DCOMPONENT_APPLICATION_SUPPORT=YES"

# Set up the build via Cmake
#######################################################################

mkdir -p build
pushd build

cmake -DENABLE_PROFILE=TIZEN $cmake_flags ..

# Build.
make %{?jobs:-j%jobs}
popd

##############################
# Installation
##############################
%install
rm -rf %{buildroot}

pushd %{_builddir}/%{name}-%{version}/build/tizen

pushd build
%make_install
popd

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
/sbin/ldconfig
rm -rf %{system_cache_dir}shader/  # this code is used to clear all existing binaries when installing Tizen packages. see build/tizen/shader-cache-path.in.
exit 0

%preun
exit 0

%postun
/sbin/ldconfig
exit 0

##############################
##############################
# Files in Binary Packages
##############################

%files
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%dir %{user_shader_cache_dir}
%{_bindir}/*
%license LICENSE
%license LICENSE.BSD-3-Clause
%license LICENSE.CC0-1.0
%license LICENSE.MIT
%license LICENSE.Unlicense
%license LICENSE.Zlib
%defattr(-,root,root,-)
%{_libdir}/libdali2-adaptor.so
%{_libdir}/libdali2-adaptor.so.2
%{_libdir}/libdali2-adaptor.so.2.0.0

# Internal so files
%{_libdir}/libdali2-adaptor-gles.so
%{_libdir}/libdali2-adaptor-gl-window-addon.so
%{_libdir}/libdali2-adaptor-application-normal.so*
%{_libdir}/libdali2-file-download-plugin-curl.so
%{_libdir}/libdali2-file-download-plugin-download-api.so

%if "%{mv_prj}" != "1"
%{_libdir}/libdali2-adaptor-application-widget.so*
%{_libdir}/libdali2-adaptor-application-component-based.so*
%endif

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
%{dali_plugin_sound_files_install_dir}/*

#################################################

%files devel
%defattr(-,root,root,-)
%{_includedir}/dali/dali.h
%{_includedir}/dali/public-api/*
%{_includedir}/dali/extension-api/*
%{_includedir}/dali/doc/*
%{_libdir}/pkgconfig/dali2-adaptor.pc

%files integration-devel
%defattr(-,root,root,-)
%{_includedir}/dali/devel-api/*
%{_includedir}/dali/integration-api/adaptor-framework/*
%{_includedir}/dali/integration-api/system/*
%{_includedir}/dali/integration-api/adaptors
%{_libdir}/pkgconfig/dali2-adaptor-integration.pc
