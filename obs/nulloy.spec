Name:          nulloy
URL:           http://nulloy.com
License:       GPLv3
Summary:       Music player with a waveform progress bar
Version:       _N_VERS_
Release:       1
Source:        %{name}-%{version}.tar.gz
%if 0%{?fedora_version} || 0%{?suse_version}
BuildRequires: qt-devel gcc-c++
%else
BuildRequires: libqt-devel
%endif
BuildRequires: gstreamer-devel gstreamer-plugins-base-devel zip libX11-devel
BuildRoot:     %{_topdir}/%{name}-%{version}-root
Patch1: nulloy-fedora.patch

%description
Music player with a waveform progress bar.

%prep
%setup -q
%if 0%{?fedora_version}
%patch1 -p1
%endif

%build
%if 0%{?fedora_version}
%define QMAKE qmake-qt4
%else
%define QMAKE qmake
%endif
QMAKE=%{QMAKE} ./configure --prefix=%{buildroot}%{_prefix}
make

%install
%makeinstall

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%{_datadir}/%{name}/skins/*
%{_datadir}/icons/*
%{_datadir}/applications/%{name}.desktop

%package  gstreamer
Summary:  GStreamer backend for Nulloy
Requires: nulloy
%description gstreamer
GStreamer backend for Nulloy.
%files gstreamer
%defattr(-,root,root)
%{_libdir}/%{name}/plugins/libplugin_gstreamer.so

%changelog

