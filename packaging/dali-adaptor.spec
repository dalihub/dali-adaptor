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


%bcond_with wayland

Name:       dali-adaptor
Summary:    The DALi Tizen Adaptor
Version:    1.1.15
Release:    1
Group:      System/Libraries
License:    Apache-2.0, BSD-2.0, MIT
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-adaptor.git;a=summary
Source0:    %{name}-%{version}.tar.gz

# Get the profile from tizen_profile_name if it exists.
%if 0%{?tizen_profile_name:1}
%define profile %{tizen_profile_name}
%endif

%if "%{profile}" == "mobile"
%define dali_profile MOBILE
%define dali_feedback_plugin 0
%define shaderbincache_flag DISABLE
%endif

%if "%{profile}" == "tv"
%define dali_profile TV
%define dali_feedback_plugin 0
%define shaderbincache_flag ENABLE
%endif

%if "%{profile}" == "wearable"
%define dali_profile WEARABLE
%define dali_feedback_plugin 0
%define shaderbincache_flag DISABLE
%endif

%if "%{profile}" == "common"
%define dali_profile COMMON
%define dali_feedback_plugin 0
%define tizen_2_2_compatibility 1
%define shaderbincache_flag DISABLE
%endif

# macro is enabled by passing gbs build --define "with_libuv 1"
%define build_with_libuv 0%{?with_libuv:1}

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:       giflib
BuildRequires:  pkgconfig
BuildRequires:  gawk
BuildRequires:  pkgconfig(sensor)
BuildRequires:  pkgconfig(aul)
BuildRequires:  giflib-devel
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  dali-devel
BuildRequires:  dali-integration-devel
BuildRequires:  pkgconfig(vconf)
BuildRequires:  tts-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  libdrm-devel
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(glesv2)
BuildRequires:  libcurl-devel
BuildRequires:  pkgconfig(harfbuzz)

BuildRequires:  fribidi-devel


%if 0%{?tizen_2_2_compatibility} != 1
BuildRequires:  pkgconfig(capi-system-info)
%endif

%if 0%{?build_with_libuv}
# Tizen currently does not have libuv as a separate libuv package
# So we have to look into the uv headers bundled inside node-js
BuildRequires:  nodejs-devel
%endif



%define dali_needs_efl_libraries 1
%define dali_needs_appfw_libraries 1
%if %{with wayland}

####### BUILDING FOR WAYLAND #######
BuildRequires:  pkgconfig(wayland-egl)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  wayland-devel
# Currently Tizen Common we use does not have wayland extensions like xdg-shell
%if "%{profile}" != "common"
BuildRequires:  wayland-extension-client-devel
%endif
%if %{build_with_libuv}
####### BUILDING FOR PURE WAYLAND #######
# if we're building with wayland and runnning under node.js then we are using libuv mainloop
# and DALis own wayland client (it needs wayland-client headers).
# EFL libraries and APP Framework libraries are not required in this case
%define dali_needs_efl_libraries 0
%define dali_needs_appfw_libraries 0
BuildRequires:  libxkbcommon-devel
%else
####### BUILDING FOR ECORE WAYLAND #######
BuildRequires:  pkgconfig(ecore-wayland)
%endif
####### BUILDING FOR X11#######
%else
BuildRequires:  pkgconfig(egl)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(utilX)
%endif

###### Building with EFL libraries
%if 0%{?dali_needs_efl_libraries}
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(elementary)
%endif

###### Build with APP Framework
%if 0%{?dali_needs_appfw_libraries}
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-system-system-settings)
%endif

%if 0%{?over_tizen_2_2}
BuildRequires:  pkgconfig(capi-system-info)
%endif





%description
The DALi Tizen Adaptor provides a Tizen specific implementation of the dali-core
platform abstraction and application shell

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
%package dali-feedback-plugin
Summary:    Plugin to play haptic and audio feedback for Dali
Group:      System/Libraries
%if 0%{?dali_feedback_plugin}
#Requires:       libdeviced
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(haptic)
BuildRequires:  libfeedback-devel
%endif

%description dali-feedback-plugin
Feedback plugin to play haptic and audio feedback for Dali

##############################
# Preparation
##############################
%prep
%setup -q
%define dali_data_rw_dir         /usr/share/dali/
%define dali_data_ro_dir         /usr/share/dali/
%define user_shader_cache_dir    %{dali_data_ro_dir}/core/shaderbin/
%define font_preloaded_path      /usr/share/fonts/
%define font_downloaded_path     /opt/share/fonts/
%define font_application_path    /usr/share/app_fonts/
%define font_configuration_file  /opt/etc/fonts/conf.avail/99-slp.conf
%define dali_plugin_sound_files  %{dali_data_ro_dir}/plugins/sounds/

%define dev_include_path %{_includedir}

##############################
# Build
##############################
%build
PREFIX+="/usr"
CXXFLAGS+=" -Wall -g -Os -fPIC -fvisibility-inlines-hidden -fdata-sections -ffunction-sections "
LDFLAGS+=" -Wl,--rpath=%{_libdir} -Wl,--as-needed -Wl,--gc-sections -Wl,-Bsymbolic-functions "

%ifarch %{arm}
CXXFLAGS+=" -D_ARCH_ARM_ -lgcc"
%endif

%if %{with wayland}
CFLAGS+=" -DWAYLAND"
CXXFLAGS+=" -DWAYLAND"
configure_flags="--enable-wayland"
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

%configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=20 --enable-shaderbincache=%{shaderbincache_flag} --enable-profile=%{dali_profile} \
%if 0%{?dali_feedback_plugin}
           --enable-feedback \
%endif
%if 0%{?tizen_2_2_compatibility}
           --with-tizen-2-2-compatibility \
%endif
%if 0%{?dali_needs_efl_libraries}
            --enable-efl=yes \
%else
            --enable-efl=no \
%endif
%if 0%{?dali_needs_appfw_libraries}
            --enable-appfw=yes \
%else
            --enable-appfw=no \
%endif
%if %{?build_with_libuv}
           --with-libuv=/usr/include/node/ \
%endif
           $configure_flags --libdir=%{_libdir}

make %{?jobs:-j%jobs}

##############################
# Installation
##############################
%install
rm -rf %{buildroot}
cd build/tizen
%make_install DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}"

# LICENSE
mkdir -p %{buildroot}/usr/share/license
cp -af %{_builddir}/%{name}-%{version}/LICENSE %{buildroot}/usr/share/license/%{name}

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

%if 0%{?dali_feedback_plugin}
%post dali-feedback-plugin
/sbin/ldconfig
exit 0
%endif

##############################
#   Pre Uninstall old package
##############################
%preun
exit 0

##############################
#   Post Uninstall old package
##############################
%postun
/sbin/ldconfig
exit 0

%if 0%{?dali_feedback_plugin}
%postun dali-feedback-plugin
/sbin/ldconfig
exit 0
%endif

##############################
# Files in Binary Packages
##############################

%files
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adap*.so*
%defattr(-,app,app,-)
%dir %{user_shader_cache_dir}
%{_bindir}/*
%{_datadir}/license/%{name}

%files devel
%defattr(-,root,root,-)
%{dev_include_path}/dali/dali.h
%{dev_include_path}/dali/public-api/*
%{dev_include_path}/dali/devel-api/*
%{dev_include_path}/dali/doc/*
%{_libdir}/pkgconfig/dali.pc

%files integration-devel
%defattr(-,root,root,-)
%{dev_include_path}/dali/integration-api/adaptors/*
%{_libdir}/pkgconfig/dali-adaptor-integration.pc

%if 0%{?dali_feedback_plugin}
%files dali-feedback-plugin
%defattr(-,root,root,-)
%{_libdir}/libdali-feedback-plugin.so*
%{dali_plugin_sound_files}/*
%endif

