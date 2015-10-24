Name:       libmm-utility
Summary:    Multimedia Framework Utility Library
Version:    0.53
Release:    0
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post):  /sbin/ldconfig
Requires(postun):  /sbin/ldconfig
BuildRequires:  pkgconfig(mm-common)
BuildRequires:  pkgconfig(mm-log)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gmodule-2.0)
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  pkgconfig(capi-media-tool)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(ttrace)
BuildRoot:  %{_tmppath}/%{name}-%{version}-build

%description


%package devel
Summary:    Multimedia Framework Utility Library (DEV)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel

%package tool
Summary:    Multimedia Framework Utility Library
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description tool

%prep
%setup -q

%build
./autogen.sh
export CFLAGS+=" -Wextra -Wno-array-bounds"
export CFLAGS+=" -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow"
export CFLAGS+=" -Wwrite-strings -Wswitch-default"
export CFLAGS+=" -Werror"
CFLAGS="$CFLAGS -DENABLE_TTRACE -DEXPORT_API=\"__attribute__((visibility(\\\"default\\\")))\" -D_MM_PROJECT_FLOATER" \
LDFLAGS+="-Wl,--rpath=%{_prefix}/lib -Wl,--hash-style=both -Wl,--as-needed" \
./configure --prefix=%{_prefix}

make %{?jobs:-j%jobs}

sed -i -e "s#@IMGP_REQPKG@#$IMGP_REQPKG#g" imgp/mmutil-imgp.pc
sed -i -e "s#@JPEG_REQPKG@#$JPEG_REQPKG#g" jpeg/mmutil-jpeg.pc

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig


%files
/usr/share/license/%{name}
%manifest libmm-utility.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so*
#%exclude %{_bindir}/*_testsuite

%files devel
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/pkgconfig/*

%files tool
%defattr(-,root,root,-)
%{_bindir}/*_testsuite