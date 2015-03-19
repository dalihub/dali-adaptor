Name:       dali-adaptor
Summary:    The DALi Tizen Adaptor
Version:    1.0.34
Release:    1
Group:      System/Libraries
License:    Apache-2.0
URL:        https://review.tizen.org/git/?p=platform/core/uifw/dali-adaptor.git;a=summary
Source0:    %{name}-%{version}.tar.gz

%define dali_profile WEARABLE
%define dali_wearable_profile 1
%define dali_feedback_plugin 0
%define dali_assimp_plugin 0

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:       boost-thread
Requires:       giflib
BuildRequires:  pkgconfig
BuildRequires:  gawk
BuildRequires:  pkgconfig(sensor)
BuildRequires:  pkgconfig(aul)
BuildRequires:  boost-devel
BuildRequires:  giflib-devel
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  dali-devel
BuildRequires:  dali-integration-devel
BuildRequires:  libxml2-devel
BuildRequires:  pkgconfig(vconf)
BuildRequires:  tts-devel
BuildRequires:  pkgconfig(dlog)
BuildRequires:  libdrm-devel
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(gles20)
BuildRequires:  pkgconfig(efl-assist)
BuildRequires:  libcurl-devel

%if 0%{?dali_assimp_plugin}
BuildRequires:  pkgconfig(assimp)
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

%description devel
Development components for the DALi Tizen Adaptor - public headers and package configs

##############################
# Dali Feedback Plugin
##############################
%package dali-feedback-plugin
Summary:    Plugin to play haptic and audio feedback for Dali
Group:      System/Libraries
%if 0%{?dali_feedback_plugin}
Requires:       libdeviced
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(deviced)
BuildRequires:  libfeedback-devel
%endif

%description dali-feedback-plugin
Feedback plugin to play haptic and audio feedback for Dali

##############################
# Preparation
##############################
%prep
%setup -q
%define dali_data_rw_dir         /opt/usr/share/dali/
%define dali_data_ro_dir         /usr/share/dali/
%define user_font_cache_dir      %{dali_data_rw_dir}/glyphcache/
%define user_shader_cache_dir    %{dali_data_ro_dir}/core/shaderbin/
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
LDFLAGS+=" -Wl,--rpath=%{_libdir} -Wl,--as-needed -Wl,--gc-sections -Wl,-Bsymbolic-functions "

%ifarch %{arm}
CXXFLAGS+=" -D_ARCH_ARM_ -lgcc"
%endif

libtoolize --force
cd %{_builddir}/%{name}-%{version}/build/tizen
autoreconf --install
DALI_DATA_RW_DIR="%{dali_data_rw_dir}" ; export DALI_DATA_RW_DIR
DALI_DATA_RO_DIR="%{dali_data_ro_dir}" ; export DALI_DATA_RO_DIR
FONT_PRELOADED_PATH="%{font_preloaded_path}" ; export FONT_PRELOADED_PATH
FONT_DOWNLOADED_PATH="%{font_downloaded_path}" ; export FONT_DOWNLOADED_PATH
FONT_APPLICATION_PATH="%{font_application_path}" ; export FONT_APPLICATION_PATH
FONT_CONFIGURATION_FILE="%{font_configuration_file}" ; export FONT_CONFIGURATION_FILE

%configure --with-jpeg-turbo --enable-gles=20 --enable-profile=%{dali_profile} \
%if 0%{?dali_feedback_plugin}
           --enable-feedback \
%endif
%if 0%{?dali_assimp_plugin}
           --enable-assimp \
%endif
           --libdir=%{_libdir}

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
exit 0

##############################
#  Post Install new package
##############################
%post
/sbin/ldconfig
chown 5000:5000 %{user_font_cache_dir}
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
rm -f %{user_font_cache_dir}/*
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
%manifest dali-adaptor.manifest-mobile
%defattr(-,root,root,-)
%{_libdir}/libdali-adap*.so*
%defattr(-,app,app,-)
%dir %{user_font_cache_dir}
%dir %{user_shader_cache_dir}
%{_bindir}/*
%{_datadir}/license/%{name}

%files devel
%defattr(-,root,root,-)
%{dev_include_path}/dali/*
%{_libdir}/pkgconfig/dali*.pc

%if 0%{?dali_feedback_plugin}
%files dali-feedback-plugin
%defattr(-,root,root,-)
%{_libdir}/libdali-feedback-plugin.so*
%{dali_plugin_sound_files}/*
%{dali_plugin_theme_files}/*
%endif

