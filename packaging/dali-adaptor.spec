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

%bcond_with wayland

Name:       dali-adaptor
Summary:    The DALi Tizen Adaptor
Version:    1.2.59
Release:    1
Group:      System/Libraries
License:    Apache-2.0 and BSD-3-Clause and MIT
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-adaptor.git;a=summary
Source0:    %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:       giflib

#need libtzplatform-config for directory if tizen version is 3.x

%if 0%{?tizen_version_major} >= 3
%define tizen_platform_config_supported 1
BuildRequires:  pkgconfig(libtzplatform-config)
%endif

%if 0%{?tizen_version_major} < 4
%define disable_cxx03_build 1
%endif

# Get the profile from tizen_profile_name if tizen version is 2.x and tizen_profile_name exists.

%if "%{tizen_version_major}" == "2" && 0%{?tizen_profile_name:1}
%define profile %{tizen_profile_name}
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
BuildRequires:  pkgconfig(capi-appfw-watch-application)
BuildRequires:  pkgconfig(appcore-watch)
BuildRequires:  pkgconfig(screen_connector_provider)
%endif

BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(glesv2)

%if !0%{?disable_cxx03_build}
BuildRequires:  dali-devel-cxx03
BuildRequires:  dali-integration-devel-cxx03
%endif
BuildRequires:  dali-devel
BuildRequires:  dali-integration-devel

BuildRequires:  pkgconfig
BuildRequires:  gawk
BuildRequires:  giflib-devel
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  pkgconfig(vconf)
BuildRequires:  tts-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  libdrm-devel
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(egl)
BuildRequires:  libcurl-devel
BuildRequires:  pkgconfig(harfbuzz)
BuildRequires:  fribidi-devel

BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(capi-system-sensor)

%if %{with wayland}

####### BUILDING FOR WAYLAND #######
BuildRequires:  pkgconfig(wayland-egl)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  wayland-devel
BuildRequires:  wayland-extension-client-devel

# dali-adaptor uses ecore mainloop
BuildRequires:  pkgconfig(ecore-wayland)

# dali-adaptor needs tbm_surface in tizen 3.0 wayland
BuildRequires:  pkgconfig(libtbm)

# tpkp-curl (certificate pinning for libcurl functions) is only available in Tizen 3.0
%if !0%{?disable_cxx03_build}
BuildRequires:  pkgconfig(tpkp-curl-deprecated)
%endif
BuildRequires:  pkgconfig(tpkp-curl)

####### BUILDING FOR X11#######
%else
BuildRequires:  pkgconfig(egl)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(utilX)
%endif

# for dali-adaptor
BuildRequires:  pkgconfig(evas)

%if 0%{?tizen_version_major} == 3
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(elementary)
%else
BuildRequires:  pkgconfig(appcore-ui)
BuildRequires:  pkgconfig(capi-appfw-app-common)
BuildRequires:  pkgconfig(capi-appfw-app-control)
BuildRequires:  pkgconfig(ecore-imf)
%endif

BuildRequires:  pkgconfig(capi-system-system-settings)

# for feedback plugin
BuildRequires:  pkgconfig(mm-sound)
%if 0%{?tizen_version_major} >= 3
BuildRequires:  pkgconfig(feedback)
%endif

# for multiprofile
Requires:   %{name}-compat = %{version}-%{release}
Recommends: %{name}-profile_common = %{version}-%{release}

%description
The DALi Tizen Adaptor provides a Tizen specific implementation of the dali-core
platform abstraction and application shell

%if !0%{?disable_cxx03_build}
%package cxx03
Summary:	The DALi Tizen Adaptor with cxx03 abi
Provides:	%{name}-cxx03 = %{version}-%{release}

%description cxx03
The DALi Tizen Adaptor provides a Tizen specific implementation of the dali-core
platform abstraction and application shell
%endif

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
%description profile_ivi
The DALi Tizen Adaptor for ivi.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
# Currently Tizen Common we use does not have wayland extensions like xdg-shell
%package profile_common
%define tizen_2_2_compatibility 0
Summary:        The DALi Tizen Adaptor for common
Provides:       %{name}-compat = %{version}-%{release}
Conflicts:      %{name}-profile_mobile
Conflicts:      %{name}-profile_wearable
Conflicts:      %{name}-profile_tv
Conflicts:      %{name}-profile_ivi
%description profile_common
The DALi Tizen Adaptor for common.
%endif

%if !0%{?disable_cxx03_build}
###########################################
# Dali adapter for profiles for cxx03 ABI
###########################################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%package profile_mobile-cxx03
Summary:	The DALi Tizen Adaptor for mobile with cxx03 abi
Provides:	%{name}-cxx03-compat = %{version}-%{release}
Conflicts:	%{name}-profile_tv-cxx03
Conflicts:	%{name}-profile_wearable-cxx03
Conflicts:	%{name}-profile_ivi-cxx03
Conflicts:	%{name}-profile_common-cxx03
%description profile_mobile-cxx03
The DALi Tizen Adaptor for mobile.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%package profile_tv-cxx03
Summary:	The DALi Tizen Adaptor for tv with cxx03 abi
Provides:	%{name}-cxx03-compat = %{version}-%{release}
Conflicts:	%{name}-profile_mobile-cxx03
Conflicts:	%{name}-profile_wearable-cxx03
Conflicts:	%{name}-profile_ivi-cxx03
Conflicts:	%{name}-profile_common-cxx03
%description profile_tv-cxx03
The DALi Tizen Adaptor for tv.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%package profile_wearable-cxx03
Summary:	The DALi Tizen Adaptor for wearable with cxx03 abi
Provides:	%{name}-cxx03-compat = %{version}-%{release}
Conflicts:	%{name}-profile_mobile-cxx03
Conflicts:	%{name}-profile_tv-cxx03
Conflicts:	%{name}-profile_ivi-cxx03
Conflicts:	%{name}-profile_common-cxx03
%description profile_wearable-cxx03
The DALi Tizen Adaptor for wearable.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
%package profile_ivi-cxx03
Summary:	The DALi Tizen Adaptor for ivi with cxx03 abi
Provides:	%{name}-cxx03-compat = %{version}-%{release}
Conflicts:	%{name}-profile_mobile-cxx03
Conflicts:	%{name}-profile_wearable-cxx03
Conflicts:	%{name}-profile_tv-cxx03
Conflicts:	%{name}-profile_common-cxx03
%description profile_ivi-cxx03
The DALi Tizen Adaptor for ivi.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
# Currently Tizen Common we use does not have wayland extensions like xdg-shell
%package profile_common-cxx03
%define tizen_2_2_compatibility 0
Summary:  The DALi Tizen Adaptor for common with cxx03 abi
Provides:	%{name}-cxx03-compat = %{version}-%{release}
Conflicts:	%{name}-profile_mobile-cxx03
Conflicts:	%{name}-profile_wearable-cxx03
Conflicts:	%{name}-profile_tv-cxx03
Conflicts:	%{name}-profile_ivi-cxx03
%description profile_common-cxx03
The DALi Tizen Adaptor for common.
%endif
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

%if !0%{?disable_cxx03_build}
##############################
# devel cxx03
##############################
%package devel-cxx03
Summary:    Development components for the DALi Tizen Adaptor with cxx03 abi
Group:      Development/Building
Requires:   %{name}-cxx03 = %{version}-%{release}
Requires:   %{name}-integration-devel-cxx03 = %{version}-%{release}

%description devel-cxx03
Development components for the DALi Tizen Adaptor - public headers and package configs

##############################
# integration-devel cxx03
##############################
%package integration-devel-cxx03
Summary:    Integration development package for the Adaptor with cxx03 abi
Group:      Development/Building
Requires:   %{name}-cxx03 = %{version}-%{release}

%description integration-devel-cxx03
Integration development package for the Adaptor - headers for integrating with an adaptor library.
%endif

##############################
# Dali Feedback Plugin
##############################
%package dali-feedback-plugin
Summary:    Plugin to play haptic and audio feedback for Dali
Group:      System/Libraries
Requires:   %{name} = %{version}-%{release}
%description dali-feedback-plugin
Feedback plugin to play haptic and audio feedback for Dali

%if !0%{?disable_cxx03_build}
##############################
# Dali Feedback Plugin cxx03
##############################

%package dali-feedback-plugin-cxx03
Summary:    Plugin to play haptic and audio feedback for Dali with cxx03 abi
Group:      System/Libraries
Requires:   %{name}-cxx03 = %{version}-%{release}
%description dali-feedback-plugin-cxx03
Feedback plugin to play haptic and audio feedback for Dali

%endif

##############################
# Preparation
##############################
%prep
%setup -q

#Use TZ_PATH when tizen version is 3.x or greater

%if 0%{?tizen_version_major} >= 3
%define dali_data_rw_dir         %TZ_SYS_RO_SHARE/dali/
%define dali_data_ro_dir         %TZ_SYS_RO_SHARE/dali/
%define font_preloaded_path      %TZ_SYS_RO_SHARE/fonts/
%define font_downloaded_path     %TZ_SYS_SHARE/fonts/
%define font_application_path    %TZ_SYS_RO_SHARE/app_fonts/
%define font_configuration_file  %TZ_SYS_ETC/fonts/conf.avail/99-slp.conf
%else
%define dali_data_rw_dir         /usr/share/dali/
%define dali_data_ro_dir         /usr/share/dali/
%define font_preloaded_path      /usr/share/fonts/
%define font_downloaded_path     /opt/share/fonts/
%define font_application_path    /usr/share/app_fonts/
%define font_configuration_file  /opt/etc/fonts/conf.avail/99-slp.conf
%endif

%define user_shader_cache_dir    %{dali_data_ro_dir}/core/shaderbin/
%define dali_plugin_sound_files  /plugins/sounds/
%define dev_include_path %{_includedir}

##############################
# Build
##############################
%build
PREFIX+="/usr"
CXXFLAGS+=" -Wall -g -Os -fPIC -fvisibility-inlines-hidden -fdata-sections -ffunction-sections -DGL_GLEXT_PROTOTYPES"
LDFLAGS+=" -Wl,--rpath=%{_libdir} -Wl,--as-needed -Wl,--gc-sections -Wl,-Bsymbolic-functions "

%ifarch %{arm}
CXXFLAGS+=" -D_ARCH_ARM_ -lgcc"
%endif

%if %{with wayland}
CFLAGS+=" -DWAYLAND"
CXXFLAGS+=" -DWAYLAND"
configure_flags="--enable-wayland"
%endif

# Use this conditional when Tizen version is 4.x or greater
%if 0%{?tizen_version_major} >= 4
CXXFLAGS+=" -DOVER_TIZEN_VERSION_4"
%endif

%if 0%{?tizen_2_2_compatibility}
CFLAGS+=" -DTIZEN_SDK_2_2_COMPATIBILITY"
CXXFLAGS+=" -DTIZEN_SDK_2_2_COMPATIBILITY"
%endif

libtoolize --force
cd %{_builddir}/%{name}-%{version}/build/tizen
autoreconf --install

DALI_DATA_RW_DIR="%{dali_data_rw_dir}" ; export DALI_DATA_RW_DIR
DALI_DATA_RO_DIR="%{dali_data_ro_dir}"  ; export DALI_DATA_RO_DIR
FONT_PRELOADED_PATH="%{font_preloaded_path}" ; export FONT_PRELOADED_PATH
FONT_DOWNLOADED_PATH="%{font_downloaded_path}" ; export FONT_DOWNLOADED_PATH
FONT_APPLICATION_PATH="%{font_application_path}"  ; export FONT_APPLICATION_PATH
FONT_CONFIGURATION_FILE="%{font_configuration_file}" ; export FONT_CONFIGURATION_FILE
%if 0%{?tizen_platform_config_supported}
TIZEN_PLATFORM_CONFIG_SUPPORTED="%{tizen_platform_config_supported}" ; export TIZEN_PLATFORM_CONFIG_SUPPORTED
%endif

# Default to GLES 2.0 if not specified.
%if 0%{?target_gles_version} == 0
%define target_gles_version 20
%endif

# Set up the build via configure.
#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=MOBILE \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
popd

pushd %{buildroot}%{_libdir}
for FILE in libdali-adap*.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE.mobile"; done
for FILE in libdali-*plugin*-cxx11.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done 
mv pkgconfig/dali-adaptor*pc %{_builddir}/%{name}-%{version}/build/tizen/
popd

make clean

%endif

#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=TV \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
popd

pushd %{buildroot}%{_libdir}
for FILE in libdali-adap*.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE.tv"; done
for FILE in libdali-*plugin*-cxx11.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done 
mv pkgconfig/dali-adaptor*pc %{_builddir}/%{name}-%{version}/build/tizen/
popd

make clean
%endif

#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=WEARABLE \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
popd

pushd %{buildroot}%{_libdir}
for FILE in libdali-adap*.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE.wearable"; done
for FILE in libdali-*plugin*-cxx11.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done 
mv pkgconfig/dali-adaptor*pc %{_builddir}/%{name}-%{version}/build/tizen/
popd

make clean
%endif

#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=IVI \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
popd

pushd %{buildroot}%{_libdir}
for FILE in libdali-adaptor*.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE.ivi"; done
for FILE in libdali-*plugin*-cxx11.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done 

mv pkgconfig/dali-adaptor*pc %{_builddir}/%{name}-%{version}/build/tizen/
popd

make clean
%endif

#######################################################################
# common
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=COMMON \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
popd

pushd %{buildroot}%{_libdir}
for FILE in libdali-adap*.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done
for FILE in libdali-*plugin*-cxx11.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done 

mv pkgconfig/dali-adaptor*pc %{_builddir}/%{name}-%{version}/build/tizen/
popd

make clean

%endif

%if !0%{?disable_cxx03_build}
#######################################################################
#BUILD for cxx03 ABI
#######################################################################

# Set up the build via configure.
#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"

%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=MOBILE \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
           --enable-cxx03-abi=yes  \
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
popd

pushd %{buildroot}%{_libdir}
for FILE in libdali-adap*.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE.mobile"; done
for FILE in libdali-*plugin.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done 
popd

make clean

%endif

#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"

%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=TV \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
           --enable-cxx03-abi=yes  \
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
popd

pushd %{buildroot}%{_libdir}
for FILE in libdali-adap*.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE.tv"; done
for FILE in libdali-*plugin.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done 
popd

make clean
%endif

#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"

%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=WEARABLE \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
           --enable-cxx03-abi=yes \
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
popd

pushd %{buildroot}%{_libdir}
for FILE in libdali-adap*.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE.wearable"; done
for FILE in libdali-*plugin.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done 
popd

make clean
%endif

#######################################################################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"

%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=IVI \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
           --enable-cxx03-abi=yes  \
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"
popd

pushd %{buildroot}%{_libdir}
for FILE in libdali-adap*.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE.ivi"; done
for FILE in libdali-*plugin.so*; do mv "$FILE" "%{_builddir}/%{name}-%{version}/build/tizen/$FILE"; done 
popd

make clean
%endif

#######################################################################
# common
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"

%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=%{target_gles_version} \
           --enable-shaderbincache=DISABLE --enable-profile=COMMON \
           --enable-tizen-major-version=%{tizen_version_major} \
%if 0%{?tizen_version_major} >= 3
           --enable-feedback \
%endif
           --enable-cxx03-abi=yes  \
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if %{with wayland}
           --enable-efl=no \
%else
           --enable-efl=yes \
%endif
%if 0%{?enable_debug}
           --enable-debug \
%endif
           --enable-appfw=yes \
           $configure_flags --libdir=%{_libdir} \
           --enable-rename-so=no

# Build.
make %{?jobs:-j%jobs}

%endif
%endif

##############################
# Installation
##############################
%install
rm -rf %{buildroot}

pushd %{_builddir}/%{name}-%{version}/build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"


# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# !unified && (wearable || tv || ivi || mobile)
%if "%{?profile}" == "wearable" || "%{?profile}" == "tv" || "%{?profile}" == "ivi" || "%{?profile}" == "mobile"
rm -rf %{buildroot}%{_libdir}/libdali-adap*.so*
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# wearable || tv || ivi || mobile || unified
%if "%{?profile}" != "common"
for FILE in libdali-*.so*; do mv "$FILE" "%{buildroot}%{_libdir}/$FILE"; done
mv dali-adaptor*.pc %{buildroot}%{_libdir}/pkgconfig/
%endif
popd

################################################
#rename 
###############################################
pushd %{buildroot}%{_libdir}

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
#%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
rm -rf libdali-adaptor*.so
%if !0%{?disable_cxx03_build}
ln -s libdali-adaptor.so.0.0.0 libdali-adaptor-cxx03.so
%endif
ln -s libdali-adaptor-cxx11.so.0.0.0 libdali-adaptor.so
#%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
rm -rf libdali-adaptor*.so.wearable
%if !0%{?disable_cxx03_build}
ln -s libdali-adaptor.so.0.0.*.wearable libdali-adaptor-cxx03.so.wearable
%endif
ln -s libdali-adaptor-cxx11.so.0.0.*.wearable libdali-adaptor.so.wearable
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
rm -rf libdali-adaptor*.so.tv
%if !0%{?disable_cxx03_build}
ln -s libdali-adaptor.so.0.0.*.tv libdali-adaptor-cxx03.so.tv
%endif
ln -s libdali-adaptor-cxx11.so.0.0.*.tv libdali-adaptor.so.tv
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
rm -rf libdali-adaptor*.so.ivi
%if !0%{?disable_cxx03_build}
ln -s libdali-adaptor.so.0.0.*.ivi libdali-adaptor-cxx03.so.ivi
%endif
ln -s libdali-adaptor-cxx11.so.0.0.*.ivi libdali-adaptor.so.ivi
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
rm -rf libdali-adaptor*.so.mobile
%if !0%{?disable_cxx03_build}
ln -s libdali-adaptor.so.0.0.*.mobile libdali-adaptor-cxx03.so.mobile
%endif
ln -s libdali-adaptor-cxx11.so.0.0.*.mobile libdali-adaptor.so.mobile
%endif
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

%pre
exit 0

##############################
#  Post Install new package
##############################
%post
/sbin/ldconfig
exit 0

##############################
#  Pre Uninstall old package
##############################
%preun
exit 0

##############################
#  Post Uninstall old package
##############################
%postun
/sbin/ldconfig
exit 0

##############################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%post profile_mobile
pushd %{_libdir}
for FILE in libdali-adaptor-cxx11.so*.mobile; do ln -sf "$FILE" "${FILE%.mobile}"; done
popd
/sbin/ldconfig
exit 0

%postun profile_mobile
/sbin/ldconfig
exit 0
%endif

##############################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%post profile_tv
pushd %{_libdir}
for FILE in libdali-adaptor-cxx11.so*.tv; do ln -sf "$FILE" "${FILE%.tv}"; done
popd
/sbin/ldconfig
exit 0

%postun profile_tv
/sbin/ldconfig
exit 0
%endif

##############################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%post profile_wearable
pushd %{_libdir}
for FILE in libdali-adaptor-cxx11.so*.wearable; do ln -sf "$FILE" "${FILE%.wearable}"; done
popd
/sbin/ldconfig
exit 0

%postun profile_wearable
/sbin/ldconfig
exit 0
%endif

##############################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
%post profile_ivi
pushd %{_libdir}
for FILE in libdali-adaptor-cxx11.so*.ivi; do ln -sf "$FILE" "${FILE%.ivi}"; done
popd
/sbin/ldconfig
exit 0

%postun profile_ivi
/sbin/ldconfig
exit 0
%endif

%if !0%{?disable_cxx03_build}
##############################
#cxx03 ABI
#############################
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%post profile_mobile-cxx03
pushd %{_libdir}
for FILE in libdali-adaptor.so*.mobile; do ln -sf "$FILE" "${FILE%.mobile}"; done
popd
/sbin/ldconfig
exit 0

%postun profile_mobile-cxx03
/sbin/ldconfig
exit 0
%endif

##############################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%post profile_tv-cxx03
pushd %{_libdir}
for FILE in libdali-adaptor.so*.tv; do ln -sf "$FILE" "${FILE%.tv}"; done
popd
/sbin/ldconfig
exit 0

%postun profile_tv-cxx03
/sbin/ldconfig
exit 0
%endif

##############################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%post profile_wearable-cxx03
pushd %{_libdir}
for FILE in libdali-adaptor.so*.wearable; do ln -sf "$FILE" "${FILE%.wearable}"; done
popd
/sbin/ldconfig
exit 0

%postun profile_wearable-cxx03
/sbin/ldconfig
exit 0
%endif

##############################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
%post profile_ivi-cxx03
pushd %{_libdir}
for FILE in libdali-adaptor.so*.ivi; do ln -sf "$FILE" "${FILE%.ivi}"; done
popd
/sbin/ldconfig
exit 0

%postun profile_ivi-cxx03
/sbin/ldconfig
exit 0
%endif
%endif

##############################
# Files in Binary Packages
##############################

%files
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%defattr(-,app,app,-)
%dir %{user_shader_cache_dir}
%{_bindir}/*
%license LICENSE

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
#%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor-cxx11.so.0*
%{_libdir}/libdali-adaptor.so
%exclude %{_libdir}/libdali-adap*.so*.mobile
%exclude %{_libdir}/libdali-adap*.so*.wearable
%exclude %{_libdir}/libdali-adap*.so*.tv
%exclude %{_libdir}/libdali-adap*.so*.ivi
#%endif

%if !0%{?disable_cxx03_build}
%files cxx03
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%defattr(-,app,app,-)
%dir %{user_shader_cache_dir}
%{_bindir}/*
%license LICENSE
# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
#%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor.so.0*
%{_libdir}/libdali-adaptor-cxx03.so
%exclude %{_libdir}/libdali-adap*.so*.mobile
%exclude %{_libdir}/libdali-adap*.so*.wearable
%exclude %{_libdir}/libdali-adap*.so*.tv
%exclude %{_libdir}/libdali-adap*.so*.ivi
#%endif

%if 0%{?tizen_version_major} >= 3
%files dali-feedback-plugin-cxx03
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-feedback-plugin.so*
%{dali_plugin_sound_files}/*
%endif
%endif

#################################################

%if 0%{?tizen_version_major} >= 3
%files dali-feedback-plugin
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-feedback-plugin-cxx11.so*
%{dali_plugin_sound_files}/*
%endif

#################################################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%files profile_common
# default .so files are housed in the main pkg.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%files profile_mobile
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor.so.mobile
%{_libdir}/libdali-adaptor-cxx11.so.0*.mobile
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%files profile_tv
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor.so.tv
%{_libdir}/libdali-adaptor-cxx11.so.0*.tv
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%files profile_wearable
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor.so.wearable
%{_libdir}/libdali-adaptor-cxx11.so.0*.wearable
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
%files profile_ivi
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor.so.ivi
%{_libdir}/libdali-adaptor-cxx11.so.0*.ivi
%endif


%files devel
%defattr(-,root,root,-)
%{dev_include_path}/dali/dali.h
%{dev_include_path}/dali/public-api/*
%{dev_include_path}/dali/devel-api/*
%{dev_include_path}/dali/doc/*
%{_libdir}/pkgconfig/dali-adaptor.pc

%files integration-devel
%defattr(-,root,root,-)
%{dev_include_path}/dali/integration-api/adaptors/*
%{_libdir}/pkgconfig/dali-adaptor-integration.pc

%if !0%{?disable_cxx03_build}
################################################
# cxx03 ABI
################################################

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if common ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%files profile_common-cxx03
# default .so files are housed in the main pkg.
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if mobile || "undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%files profile_mobile-cxx03
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor-cxx03.so.mobile
%{_libdir}/libdali-adaptor.so.0*mobile
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if tv ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "common" && "%{?profile}" != "ivi" && "%{?profile}" != "mobile"
%files profile_tv-cxx03
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor-cxx03.so.tv
%{_libdir}/libdali-adaptor.so.0*.tv
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if wearable || "undefined"
%if "%{?profile}" != "mobile" && "%{?profile}" != "tv" && "%{?profile}" != "ivi" && "%{?profile}" != "common"
%files profile_wearable-cxx03
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor-cxx03.so.wearable
%{_libdir}/libdali-adaptor.so.0*.wearable
%endif

# This is for backward-compatibility. This does not deteriorate 4.0 Configurability
# if ivi ||"undefined"
%if "%{?profile}" != "wearable" && "%{?profile}" != "tv" && "%{?profile}" != "common" && "%{?profile}" != "mobile"
%files profile_ivi-cxx03
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adaptor-cxx03.so.ivi
%{_libdir}/libdali-adaptor.so.0*.ivi
%endif


%files devel-cxx03
%defattr(-,root,root,-)
%{dev_include_path}/dali/dali.h
%{dev_include_path}/dali/public-api/*
%{dev_include_path}/dali/devel-api/*
%{dev_include_path}/dali/doc/*
%{_libdir}/pkgconfig/dali-adaptor-cxx03.pc

%files integration-devel-cxx03
%defattr(-,root,root,-)
%{dev_include_path}/dali/integration-api/adaptors/*
%{_libdir}/pkgconfig/dali-adaptor-integration-cxx03.pc
%endif
