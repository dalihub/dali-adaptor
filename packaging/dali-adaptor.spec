# dali2 is the same code built a second time under the dali2 name, for
# applications that still use it. To drop it, remove the "with dali2" blocks.
%bcond_without dali2

# NOTES
# This spec file builds one DALi Adaptor, used by every Tizen profile.

Name:       dali3-adaptor
Summary:    The DALi Tizen Adaptor
Version:    2.5.45
Release:    1
Group:      System/Libraries
License:    Apache-2.0 and BSD-3-Clause and MIT and CC0-1.0 and Unlicense and Zlib
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-adaptor.git;a=summary
Source0:    %{name}-%{version}.tar.gz

# graphics_backend selects ENABLE_GRAPHICS_BACKEND (see build/tizen/deps-check.cmake):
# DYNAMIC builds the GLES and Vulkan backends as separate libraries selectable
# at runtime; GLES or VULKAN builds only that backend into the main library
# (GLES needs no vulkan/glslang, neither produces the -vulkan subpackage).
# Default: DYNAMIC on Tizen 9 or newer, GLES on Tizen 8 and older, whose glslang
# packages cannot build the Vulkan backend.
# Example: gbs build ... --define "graphics_backend GLES"
%if 0%{?tizen_version_major} >= 9
%{!?graphics_backend: %define graphics_backend DYNAMIC}
%else
%{!?graphics_backend: %define graphics_backend GLES}
%endif
%if "%{graphics_backend}" != "DYNAMIC" && "%{graphics_backend}" != "GLES" && "%{graphics_backend}" != "VULKAN"
%{error:graphics_backend must be DYNAMIC, GLES or VULKAN}
%endif

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:       giflib
%if "%{graphics_backend}" == "VULKAN"
Requires:       glslang
%endif

%define tizen_platform_config_supported 1
# BUILD_TCORE_ENABLE: 0 (default) = ECORE, 1 = TCORE, for Tizen 11 only.
# Example: gbs build ... --define "BUILD_TCORE_ENABLE 1"
# Resolve the backend here for both dependencies and build options.
%{!?BUILD_TCORE_ENABLE:%global BUILD_TCORE_ENABLE 0}
%global tizen_wayland_backend ECORE
%if 0%{?tizen_version_major} == 11
%if "%{BUILD_TCORE_ENABLE}" != "0" && "%{BUILD_TCORE_ENABLE}" != "1"
%{error:BUILD_TCORE_ENABLE must be 0 or 1}
%endif
%if "%{BUILD_TCORE_ENABLE}" == "1"
%global tizen_wayland_backend TCORE
%endif
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

BuildRequires:  dali3-devel
BuildRequires:  dali3-integration-devel
%if %{with dali2}
BuildRequires:  dali2-devel
BuildRequires:  dali2-integration-devel
%endif

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

%if "%{graphics_backend}" != "GLES"
BuildRequires:  pkgconfig(vulkan)
BuildRequires:  glslang-devel
BuildRequires:  glslang
%endif
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
BuildRequires:  pkgconfig(component-based-core-base-tcore)
%else
BuildRequires:  pkgconfig(appcore-widget-base)
BuildRequires:  pkgconfig(component-based-core-base)
%endif
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

%description
The DALi Tizen Adaptor provides a Tizen specific implementation of the dali-core
platform abstraction and application shell

###########################################
# Vulkan Graphics Backend
###########################################
%if "%{graphics_backend}" == "DYNAMIC"
%package vulkan
Summary:        The DALi Tizen Adaptor with the Vulkan library
Requires:       %{name}
Requires:       glslang
%description vulkan
The DALi Tizen Adaptor with the Vulkan library.
%endif

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
%package dali3-feedback-plugin
Summary:    Plugin to play haptic and audio feedback for Dali
Group:      System/Libraries
Requires:   %{name} = %{version}-%{release}
%description dali3-feedback-plugin
Feedback plugin to play haptic and audio feedback for Dali

%if %{with dali2}
##############################
# dali2
##############################
%package -n dali2-adaptor
Summary:    The DALi Tizen Adaptor
Group:      System/Libraries
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:       giflib
%if "%{graphics_backend}" == "VULKAN"
Requires:       glslang
%endif
%if 0%{?tizen_version_major} >= 11
Requires: capi-system-system-settings-util-lib
%endif
# Absorbs the profile_mobile, profile_tv and profile_common packages an image
# may still have installed. The library they carried is in this package.
Provides:   dali2-adaptor-profile_common = %{version}-%{release}
Provides:   dali2-adaptor-profile_mobile = %{version}-%{release}
Provides:   dali2-adaptor-profile_tv = %{version}-%{release}
Obsoletes:  dali2-adaptor-profile_common < %{version}-%{release}
Obsoletes:  dali2-adaptor-profile_mobile < %{version}-%{release}
Obsoletes:  dali2-adaptor-profile_tv < %{version}-%{release}

%description -n dali2-adaptor
The DALi Tizen Adaptor - the same code as %{name}, under the dali2 name

%if "%{graphics_backend}" == "DYNAMIC"
%package -n dali2-adaptor-vulkan
Summary:        The DALi Tizen Adaptor with the Vulkan library
Requires:       dali2-adaptor
Requires:       glslang
%description -n dali2-adaptor-vulkan
The DALi Tizen Adaptor with the Vulkan library.
%endif

%package -n dali2-adaptor-devel
Summary:    Development components for the DALi Tizen Adaptor
Group:      Development/Building
Requires:   dali2-adaptor = %{version}-%{release}

%description -n dali2-adaptor-devel
Development components for the DALi Tizen Adaptor - public headers and package configs

%package -n dali2-adaptor-integration-devel
Summary:    Integration development package for the Adaptor
Group:      Development/Building
Requires:   dali2-adaptor = %{version}-%{release}
Requires:   dali2-adaptor-devel = %{version}-%{release}

%description -n dali2-adaptor-integration-devel
Integration development package for the Adaptor - headers for integrating with an adaptor library.

%package -n dali2-adaptor-dali2-feedback-plugin
Summary:    Plugin to play haptic and audio feedback for Dali
Group:      System/Libraries
Requires:   dali2-adaptor = %{version}-%{release}
%description -n dali2-adaptor-dali2-feedback-plugin
Feedback plugin to play haptic and audio feedback for Dali
%endif

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
PREFIX="$PREFIX/usr"
CXXFLAGS="$CXXFLAGS -Wall -g -Os -fPIC -fvisibility-inlines-hidden -fdata-sections -ffunction-sections -DGL_GLEXT_PROTOTYPES -Wno-psabi"
LDFLAGS="$LDFLAGS -Wl,--rpath=%{_libdir} -Wl,--as-needed -Wl,--gc-sections -lttrace -Wl,-Bsymbolic-functions "

%ifarch %{arm}
CXXFLAGS="$CXXFLAGS -D_ARCH_ARM_ -lgcc"
%endif

CFLAGS="$CFLAGS -DWAYLAND -DEFL_BETA_API_SUPPORT"
CXXFLAGS="$CXXFLAGS -DWAYLAND -DEFL_BETA_API_SUPPORT"
cmake_flags=" -DENABLE_WAYLAND=ON -DENABLE_ATSPI=ON"

%if 0%{?enable_streamline}
cmake_flags="$cmake_flags -DENABLE_TRACE_STREAMLINE=ON"
%else
cmake_flags="$cmake_flags -DENABLE_TRACE=ON"
%endif

# Tizen Wayland backend (see default at top of spec)
%if "%{tizen_wayland_backend}" == "TCORE"
cmake_flags="$cmake_flags -DTIZEN_WAYLAND_BACKEND=TCORE"
%else
cmake_flags="$cmake_flags -DTIZEN_WAYLAND_BACKEND=ECORE"
%endif

# Graphics backend (see default at top of spec)
cmake_flags="$cmake_flags -DENABLE_GRAPHICS_BACKEND=%{graphics_backend}"

# Enable preinitialized adaptor as default if not defined
%{!?enable_preinitialize_adaptor: %global enable_preinitialize_adaptor 1}

%if 0%{?enable_preinitialize_adaptor}
cmake_flags="$cmake_flags -DENABLE_PREINITIALIZE_ADAPTOR=ON"
%else
cmake_flags="$cmake_flags -DENABLE_PREINITIALIZE_ADAPTOR=OFF"
%endif

# Use this conditional when Tizen version is 7.x or greater
%if 0%{?tizen_version_major} >= 7
CXXFLAGS="$CXXFLAGS -DOVER_TIZEN_VERSION_7"
%endif

# Use this conditional when Tizen version is 8.x or greater
%if 0%{?tizen_version_major} >= 8
CXXFLAGS="$CXXFLAGS -DOVER_TIZEN_VERSION_8"
%endif

# Use this conditional when Tizen version is 9.x or greater
%if 0%{?tizen_version_major} >= 9
CXXFLAGS="$CXXFLAGS -DOVER_TIZEN_VERSION_9"
%endif

# Use this conditional when Tizen version is 10.x or greater
%if 0%{?tizen_version_major} >= 10
CXXFLAGS="$CXXFLAGS -DOVER_TIZEN_VERSION_10"
cmake_flags="$cmake_flags -DENABLE_WARNING_TO_ERROR=ON"
%else
cmake_flags="$cmake_flags -DENABLE_WARNING_TO_ERROR=OFF"
%endif

# Use this conditional when Tizen version is 11.x or greater
%if 0%{?tizen_version_major} >= 11
CXXFLAGS+=" -DOVER_TIZEN_VERSION_11"
%endif

%if "%{vd_asan}" == "1" || "%{asan}" == "1"
CFLAGS="$CFLAGS -fsanitize=address"
CXXFLAGS="$CXXFLAGS -fsanitize=address"
LDFLAGS="$LDFLAGS -fsanitize=address"
cmake_flags="$cmake_flags -DENABLE_ASAN=ON"
%endif

%if 0%{?enable_debug}
cmake_flags="$cmake_flags -DCMAKE_BUILD_TYPE=Debug"
%endif

%if 0%{?enable_logging}
cmake_flags="$cmake_flags -DENABLE_NETWORK_LOGGING=ON"
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

cmake_flags="$cmake_flags -DCMAKE_INSTALL_PREFIX=$PREFIX"
cmake_flags="$cmake_flags -DCMAKE_INSTALL_LIBDIR=%{_libdir}"
cmake_flags="$cmake_flags -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir}"
cmake_flags="$cmake_flags -DENABLE_TIZEN_MAJOR_VERSION=%{tizen_version_major}"
cmake_flags="$cmake_flags -DENABLE_FEEDBACK=YES"
cmake_flags="$cmake_flags -DENABLE_APPMODEL=ON"

%if "%{mv_prj}" != "1"
cmake_flags="$cmake_flags -DROBOT_PROFILE=NO"
%else
cmake_flags="$cmake_flags -DROBOT_PROFILE=YES"
%endif

cmake_flags="$cmake_flags -DENABLE_APPFW=YES"
cmake_flags="$cmake_flags -DCOMPONENT_APPLICATION_SUPPORT=YES"

# Set up the build via Cmake
#######################################################################

mkdir -p build
cd build

cmake -DENABLE_PROFILE=TIZEN $cmake_flags ..

# Build.
make %{?jobs:-j%jobs}
cd ..

%if %{with dali2}
# The same options as dali3 above; only DALI_API_MAJOR differs.
mkdir -p build-dali2
cd build-dali2

cmake -DENABLE_PROFILE=TIZEN $cmake_flags -DDALI_API_MAJOR=2 ..

make %{?jobs:-j%jobs}
cd ..
%endif

##############################
# Installation
##############################
%install
rm -rf %{buildroot}

cd %{_builddir}/%{name}-%{version}/build/tizen

cd build
%make_install
cd ..

%if %{with dali2}
cd build-dali2
%make_install
cd ..
%endif

# Create a symbolic link in integration-api to preserve legacy repo build
_dali_install_tizen_dir=$(pwd)
cd %{buildroot}%{_includedir}/dali/integration-api
ln -sf adaptor-framework adaptors
cd "$_dali_install_tizen_dir"

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

%if %{with dali2}
%post -n dali2-adaptor
/sbin/ldconfig
rm -rf %{system_cache_dir}shader/
exit 0

%postun -n dali2-adaptor
/sbin/ldconfig
exit 0
%endif

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
%{_libdir}/libdali3-adaptor.so
%{_libdir}/libdali3-adaptor.so.3
%{_libdir}/libdali3-adaptor.so.3.0.0

# Internal so files
# Separate graphics libraries exist only in the DYNAMIC configuration;
# GLES / VULKAN builds integrate the backend into libdali3-adaptor.so
%if "%{graphics_backend}" == "DYNAMIC"
%{_libdir}/libdali3-adaptor-gles.so
%endif
%{_libdir}/libdali3-adaptor-gl-window-addon.so
%{_libdir}/libdali3-adaptor-application-normal.so*
%{_libdir}/libdali3-file-download-plugin-curl.so
%{_libdir}/libdali3-file-download-plugin-download-api.so

%if "%{mv_prj}" != "1"
%{_libdir}/libdali3-adaptor-application-widget.so*
%{_libdir}/libdali3-adaptor-application-component-based.so*
%endif

#################################################

%if "%{graphics_backend}" == "DYNAMIC"
%files vulkan
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-adaptor-vulkan.so
%endif

#################################################

%files dali3-feedback-plugin
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali3-feedback-plugin.so*
%{dali_plugin_sound_files_install_dir}/*

#################################################

%files devel
%defattr(-,root,root,-)
%{_includedir}/dali/dali.h
%{_includedir}/dali/public-api/*
%{_includedir}/dali/extension-api/*
%{_includedir}/dali/doc/*
%{_libdir}/pkgconfig/dali3-adaptor.pc

%files integration-devel
%defattr(-,root,root,-)
%{_includedir}/dali/devel-api/*
%{_includedir}/dali/integration-api/adaptor-framework/*
%{_includedir}/dali/integration-api/system/*
%{_includedir}/dali/integration-api/adaptors
%{_libdir}/pkgconfig/dali3-adaptor-integration.pc

%if %{with dali2}
#################################################
# Both builds install the same headers, data and tools, so dali2 shares them with dali3.

%files -n dali2-adaptor
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
%if "%{graphics_backend}" == "DYNAMIC"
%{_libdir}/libdali2-adaptor-gles.so
%endif
%{_libdir}/libdali2-adaptor-gl-window-addon.so
%{_libdir}/libdali2-adaptor-application-normal.so*
%{_libdir}/libdali2-file-download-plugin-curl.so
%{_libdir}/libdali2-file-download-plugin-download-api.so
%if "%{mv_prj}" != "1"
%{_libdir}/libdali2-adaptor-application-widget.so*
%{_libdir}/libdali2-adaptor-application-component-based.so*
%endif

%if "%{graphics_backend}" == "DYNAMIC"
%files -n dali2-adaptor-vulkan
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-adaptor-vulkan.so
%endif

%files -n dali2-adaptor-dali2-feedback-plugin
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-feedback-plugin.so*
%{dali_plugin_sound_files_install_dir}/*

%files -n dali2-adaptor-devel
%defattr(-,root,root,-)
%{_includedir}/dali/dali.h
%{_includedir}/dali/public-api/*
%{_includedir}/dali/extension-api/*
%{_includedir}/dali/doc/*
%{_libdir}/pkgconfig/dali2-adaptor.pc

%files -n dali2-adaptor-integration-devel
%defattr(-,root,root,-)
%{_includedir}/dali/devel-api/*
%{_includedir}/dali/integration-api/adaptor-framework/*
%{_includedir}/dali/integration-api/system/*
%{_includedir}/dali/integration-api/adaptors
%{_libdir}/pkgconfig/dali2-adaptor-integration.pc
%endif
