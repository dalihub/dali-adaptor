# This source builds DALi 3 (dali3-adaptor). On Tizen 10.1 and 11, where
# applications are still moving from DALi 2.x, it also produces the DALi 2.x
# package names (dali2-adaptor and its sub-packages) as alias packages: links
# to the dali3 files plus the Provides that 2.x binaries and specs depend on.
# On Tizen 10.0 and below the real DALi 2.x is installed from its own source,
# so the aliases must never be built there. --with/--without dali2_aliases
# overrides the default for a local build.
#
# Nested %if, not %elif: Tizen's rpm (4.14) predates %elif and silently
# skips such a branch.
%define legacy_tizen (0%{?tizen_version_major} < 10 || (0%{?tizen_version_major} == 10 && 0%{?tizen_version_minor} == 0))
%if %{legacy_tizen}
%bcond_with    dali2_aliases
%else
%bcond_without dali2_aliases
%endif
%if %{with dali2_aliases} && %{legacy_tizen}
%{error:Tizen <= 10.0 ships the real DALi 2.x; dali2 aliases must not be built there}
%endif

# Temporary, for the Tizen Studio rootstrap. Its generator copies only the
# files that the packages it names list themselves, and reaches a directory
# link as an empty directory, so while the public rootstrap still names the
# DALi 2.x packages and include paths the alias devel packages install real
# copies of the headers under include/dali, beside those of dali2-devel,
# which installs the directory in place of its link. Remove this once the
# rootstrap names the dali3 packages and include/dali3.
%if %{with dali2_aliases}
%bcond_without dali2_include_copy
%endif

# rpm derives no Provides from a symbolic link, so the alias packages declare
# the ELF ones themselves. 64-bit ELF provides carry a suffix.
%if "%{__isa_bits}" == "64"
%define elf_suffix ()(64bit)
%else
%define elf_suffix %{nil}
%endif

# NOTES
# This spec file builds one DALi Adaptor, used by every Tizen profile.

Name:       dali3-adaptor
Summary:    The DALi Tizen Adaptor
Version:    3.0.1
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
# BUILD_TCORE_ENABLE: 0 (default) = ECORE, 1 = TCORE.
# The backend is decided by this build config only (regardless of the Tizen version).
# Example: gbs build ... --define "BUILD_TCORE_ENABLE 1"
# Resolve the backend here for both dependencies and build options.
%{!?BUILD_TCORE_ENABLE:%global BUILD_TCORE_ENABLE 0}
%global tizen_wayland_backend ECORE
%if "%{BUILD_TCORE_ENABLE}" != "0" && "%{BUILD_TCORE_ENABLE}" != "1"
%{error:BUILD_TCORE_ENABLE must be 0 or 1}
%endif
%if "%{BUILD_TCORE_ENABLE}" == "1"
%global tizen_wayland_backend TCORE
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
BuildRequires:  pkgconfig(gio-2.0)

# for feedback plugin
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(feedback)

BuildRequires:  pkgconfig(thorvg)

# HEIF support only from Tizen 10 onwards
%if 0%{?tizen_version_major} >= 10
BuildRequires:  libheif
BuildRequires:  libheif-devel
BuildRequires:  pkgconfig(heif)
%endif

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

%if %{with dali2_aliases}
##############################
# DALi 2.x names (aliases)
##############################
%package -n dali2-adaptor
Summary:    DALi 2.x library names for the DALi Tizen Adaptor
Group:      System/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   dali2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Provides:   libdali2-adaptor.so.2%{elf_suffix}

# Keep the legacy profile names for existing image configurations and upgrades.
Provides:   dali2-adaptor-profile_common = %{version}-%{release}
Provides:   dali2-adaptor-profile_mobile = %{version}-%{release}
Provides:   dali2-adaptor-profile_tv = %{version}-%{release}
Obsoletes:  dali2-adaptor-profile_common < %{version}-%{release}
Obsoletes:  dali2-adaptor-profile_mobile < %{version}-%{release}
Obsoletes:  dali2-adaptor-profile_tv < %{version}-%{release}

%description -n dali2-adaptor
libdali2-adaptor.so.2, the internal libdali2-adaptor-*.so names, dalireslog.sh and share/dali as links to their dali3 counterparts, for binaries built against DALi 2.x. Goes away once the applications have moved to dali3.

%if "%{graphics_backend}" == "DYNAMIC"
%package -n dali2-adaptor-vulkan
Summary:        DALi 2.x name for the Vulkan library of the DALi Tizen Adaptor
Requires:       dali2-adaptor = %{version}-%{release}
Requires:       %{name}-vulkan = %{version}-%{release}
%description -n dali2-adaptor-vulkan
libdali2-adaptor-vulkan.so as a link to the dali3 library.
%endif

%package -n dali2-adaptor-devel
Summary:    DALi 2.x development names for the DALi Tizen Adaptor
Group:      Development/Building
Requires:   dali2-adaptor = %{version}-%{release}
Requires:   dali2-devel
Requires:   %{name}-devel = %{version}-%{release}

%description -n dali2-adaptor-devel
dali2-adaptor.pc and libdali2-adaptor.so as links to their dali3 counterparts; the headers are reached through the include/dali link of dali2-devel.

%package -n dali2-adaptor-integration-devel
Summary:    DALi 2.x integration development names for the DALi Tizen Adaptor
Group:      Development/Building
Requires:   dali2-adaptor-devel = %{version}-%{release}
Requires:   %{name}-integration-devel = %{version}-%{release}

%description -n dali2-adaptor-integration-devel
dali2-adaptor-integration.pc and the integration-api/adaptors link that 2.x shipped.

%package -n dali2-adaptor-dali2-feedback-plugin
Summary:    DALi 2.x name for the Dali feedback plugin
Group:      System/Libraries
Requires:   dali2-adaptor = %{version}-%{release}
Requires:   %{name}-dali3-feedback-plugin = %{version}-%{release}
%description -n dali2-adaptor-dali2-feedback-plugin
libdali2-feedback-plugin.so as a link to the dali3 plugin.
%endif

#Use TZ_PATH when tizen version is 3.x or greater
%define dali3_data_dir           %TZ_SYS_RO_SHARE/dali3/
%define font_preloaded_path      %TZ_SYS_RO_SHARE/fonts/
%define font_downloaded_path     %TZ_SYS_SHARE/fonts/
%define font_application_path    %TZ_SYS_RO_SHARE/app_fonts/
%define font_configuration_file  %TZ_SYS_ETC/fonts/conf.avail/99-slp.conf

# Paths below mirror what CMake derives from DALI_API_MAJOR.
%define dali3_shader_cache_dir %{dali3_data_dir}/core/shaderbin/
%define dali3_system_cache_dir /home/owner/.cache/dali3_common_caches/
%define dali3_sound_dir        %{dali3_data_dir}/plugins/sounds/

##############################
# Preparation
##############################
%prep
%setup -q


##############################
# Build
##############################
%build
# Keep shadow diagnostics enabled independently of the build server policy.
export CFLAGS="${CFLAGS:-%optflags} -Werror=shadow"
export CXXFLAGS="${CXXFLAGS:-%optflags} -Werror=shadow"

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

# Use this conditional when Tizen version is 10.1 or greater
%if ( 0%{?tizen_version_major} == 10 && 0%{?tizen_version_minor} >= 1 ) || 0%{?tizen_version_major} >= 11
CXXFLAGS+=" -DOVER_TIZEN_VERSION_10_1"
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

# The data directory goes in as a cache variable, not the environment, so
# that a re-run of cmake inside the build directory keeps it.
cmake -DENABLE_PROFILE=TIZEN \
      -DDALI_DATA_RW_DIR="%{dali3_data_dir}" -DDALI_DATA_RO_DIR="%{dali3_data_dir}" \
      -DDALI_LEGACY_ALIASES=%{?with_dali2_aliases:ON}%{!?with_dali2_aliases:OFF} \
      $cmake_flags .

make %{?jobs:-j%jobs}

##############################
# Installation
##############################
%install
rm -rf %{buildroot}
cd build/tizen
%make_install

%if %{with dali2_include_copy}
# include/dali: copies of this package's dali3 headers beside those of
# dali2-devel (see dali2_include_copy above)
mkdir -p %{buildroot}%{_includedir}/dali
cp -a %{buildroot}%{_includedir}/dali3/dali/. %{buildroot}%{_includedir}/dali
%endif

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
rm -rf %{dali3_system_cache_dir}shader/  # this code is used to clear all existing binaries when installing Tizen packages. see build/tizen/shader-cache-path.in.
exit 0

%preun
exit 0

%postun
/sbin/ldconfig
exit 0

%if %{with dali2_aliases}
%post -n dali2-adaptor
/sbin/ldconfig
exit 0

%postun -n dali2-adaptor
/sbin/ldconfig
exit 0
%endif

##############################
##############################
# Files in Binary Packages
##############################
# The Lua scriptlet runs up to the next section, so nothing but %files may
# follow it. Every earlier package (DALi 2.x and dali3 3.0.0 alike) installed
# share/dali as a real directory; rpm cannot replace a directory with a link,
# so the alias package clears it before the transaction unpacks.
%if %{with dali2_aliases}
%pretrans -n dali2-adaptor -p <lua>
local path = "%TZ_SYS_RO_SHARE/dali"
if posix.stat(path, "type") == "directory" then
  os.execute("rm -rf '" .. path .. "'")
end
%endif
%files
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%dir %{dali3_shader_cache_dir}
%{_bindir}/dali3-reslog.sh
%license LICENSE
%license LICENSE.BSD-3-Clause
%license LICENSE.CC0-1.0
%license LICENSE.MIT
%license LICENSE.Unlicense
%license LICENSE.Zlib
%defattr(-,root,root,-)
%{_libdir}/libdali3-adaptor.so*

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
%{dali3_sound_dir}/*

#################################################

%files devel
%defattr(-,root,root,-)
%{_includedir}/dali3/dali/dali.h
%{_includedir}/dali3/dali/public-api/*
%{_includedir}/dali3/dali/extension-api/*
%{_includedir}/dali3/dali/doc/*
%{_libdir}/pkgconfig/dali3-adaptor.pc

%files integration-devel
%defattr(-,root,root,-)
%{_includedir}/dali3/dali/devel-api/*
%{_includedir}/dali3/dali/integration-api/adaptor-framework/*
%{_includedir}/dali3/dali/integration-api/adaptors
%{_includedir}/dali3/dali/integration-api/system/*
%{_libdir}/pkgconfig/dali3-adaptor-integration.pc

%if %{with dali2_aliases}
#################################################

%files -n dali2-adaptor
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%TZ_SYS_RO_SHARE/dali
%{_bindir}/dalireslog.sh
%{_libdir}/libdali2-adaptor.so.2
%if "%{graphics_backend}" == "DYNAMIC"
%{_libdir}/libdali2-adaptor-gles.so
%endif
%{_libdir}/libdali2-adaptor-gl-window-addon.so
%{_libdir}/libdali2-adaptor-application-normal.so
%{_libdir}/libdali2-file-download-plugin-curl.so
%{_libdir}/libdali2-file-download-plugin-download-api.so
%if "%{mv_prj}" != "1"
%{_libdir}/libdali2-adaptor-application-widget.so
%{_libdir}/libdali2-adaptor-application-component-based.so
%endif
%license LICENSE

%if "%{graphics_backend}" == "DYNAMIC"
%files -n dali2-adaptor-vulkan
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-adaptor-vulkan.so
%endif

%files -n dali2-adaptor-dali2-feedback-plugin
%manifest dali-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/libdali2-feedback-plugin.so

%files -n dali2-adaptor-devel
%defattr(-,root,root,-)
%{_libdir}/libdali2-adaptor.so
%{_libdir}/pkgconfig/dali2-adaptor.pc
%if %{with dali2_include_copy}
%{_includedir}/dali/dali.h
%{_includedir}/dali/public-api/*
%{_includedir}/dali/extension-api/*
%{_includedir}/dali/doc/*
%endif

%files -n dali2-adaptor-integration-devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/dali2-adaptor-integration.pc
%if %{with dali2_include_copy}
%{_includedir}/dali/devel-api/*
%{_includedir}/dali/integration-api/adaptor-framework/*
%{_includedir}/dali/integration-api/system/*
%{_includedir}/dali/integration-api/adaptors
%endif
%endif
