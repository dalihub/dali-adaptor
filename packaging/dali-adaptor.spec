Name:       dali-adaptor
Summary:    The DALi Tizen Adaptor
Version:    0.9.15
Release:    1
Group:      System/Libraries
License:    Flora
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-adaptor.git;a=summary
Source0:    %{name}-%{version}.tar.gz

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:       boost-thread
#Requires:       giflib
BuildRequires:  gettext
BuildRequires:  pkgconfig
BuildRequires:  gawk
BuildRequires:  pkgconfig(sensor)
BuildRequires:  pkgconfig(aul)
BuildRequires:  boost-devel
BuildRequires:  giflib-devel
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(isf)
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  dali-devel
BuildRequires:  dali-integration-devel
BuildRequires:  libxml2-devel
BuildRequires:  vconf-devel
BuildRequires:  tts-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  libdrm-devel
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(gles20)
ExclusiveArch:  armv7l

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

%description devel
Development components for the DALi Tizen Adaptor - public headers and package configs

##############################
# Dali Feedback Plugin
##############################
%package dali-feedback-plugin
Summary:    Plugin to play haptic and audio feedback for Dali
Group:      System/Libraries
#Requires:       libdeviced
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(haptic)
BuildRequires:  libfeedback-devel

%description dali-feedback-plugin
Feedback plugin to play haptic and audio feedback for Dali

##############################
# Dali Dynamics/Bullet Plugin
##############################
%package dali-bullet-plugin
Summary:    Plugin to provide physics
Group:      System/Libraries
BuildRequires:  bullet-devel

%description dali-bullet-plugin
Dynamics plugin to wrap the libBulletDynamics libraries

##############################
# Preparation
##############################
%prep
%setup -q
%define dali_data_rw_dir         /opt/usr/share/dali/
%define dali_data_ro_dir         /usr/share/dali/
%define user_font_cache_dir      %{dali_data_rw_dir}/glyphcache/
%define user_shader_cache_dir    %{dali_data_rw_dir}/core/shaderbin/
%define font_preloaded_path      /usr/share/fonts/
%define font_downloaded_path     /opt/share/fonts/
%define font_application_path    /usr/share/app_fonts/
%define font_configuration_file  /opt/etc/fonts/conf.avail/99-slp.conf
%define dali_plugin_sound_files  %{dali_data_ro_dir}/plugins/sounds/
%define dali_plugin_theme_files  %{dali_data_ro_dir}/themes/feedback-themes/

%define dev_include_path %{_includedir}

##############################
# Build
##############################
%build
PREFIX+="/usr"
CXXFLAGS+=" -Wall -g -Os -fPIC -fvisibility-inlines-hidden -fdata-sections -ffunction-sections "
LDFLAGS+=" -Wl,--rpath=$PREFIX/lib -Wl,--as-needed -Wl,--gc-sections "

%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

%ifarch %{arm}
CXXFLAGS+=" -D_ARCH_ARM_ -lgcc"
%endif

libtoolize --force
cd %{_builddir}/%{name}-%{version}/build/tizen && autoreconf --install
cd %{_builddir}/%{name}-%{version}/build/tizen && CXXFLAGS=$CXXFLAGS LDFLAGS=$LDFLAGS DALI_DATA_RW_DIR="%{dali_data_rw_dir}" DALI_DATA_RO_DIR="%{dali_data_ro_dir}" FONT_PRELOADED_PATH="%{font_preloaded_path}" FONT_DOWNLOADED_PATH="%{font_downloaded_path}" FONT_APPLICATION_PATH="%{font_application_path}" FONT_CONFIGURATION_FILE="%{font_configuration_file}" ./configure --prefix=$PREFIX --with-jpeg-turbo --enable-gles=20 --enable-profile=COMMON


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
rm -f %{user_font_cache_dir}/*
rm -f %{user_shader_cache_dir}/*
exit 0

##############################
#  Post Install new package
##############################
%post
/sbin/ldconfig
chown 5000:5000 %{user_font_cache_dir}
chown 5000:5000 %{user_shader_cache_dir}
exit 0

%post dali-feedback-plugin
/sbin/ldconfig
exit 0

%post dali-bullet-plugin
/sbin/ldconfig
exit 0

##############################
#   Pre Uninstall old package
##############################
%preun
rm -f %{user_font_cache_dir}/*
rm -f %{user_shader_cache_dir}/*
exit 0

##############################
#   Post Uninstall old package
##############################
%postun
/sbin/ldconfig
exit 0

%postun dali-feedback-plugin
/sbin/ldconfig
exit 0

%postun dali-bullet-plugin
/sbin/ldconfig
exit 0


##############################
# Files in Binary Packages
##############################

%files
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali-adap*.so*
%{_libdir}/libdali-appl*.so*
%defattr(-,app,app,-)
%dir %{user_font_cache_dir}
%dir %{user_shader_cache_dir}
%{_bindir}/*
%{_datadir}/license/%{name}

%files devel
%defattr(-,root,root,-)
%{dev_include_path}/dali/*
%{_libdir}/pkgconfig/dali*.pc

%files dali-feedback-plugin
%defattr(-,root,root,-)
%{_libdir}/libdali-feedback-plugin.so*
%{dali_plugin_sound_files}/*
%{dali_plugin_theme_files}/*

%files dali-bullet-plugin
%defattr(-,root,root,-)
%{_libdir}/libdali-bullet-plugin.so*
