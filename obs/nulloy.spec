Name:          nulloy
URL:           http://nulloy.com
License:       GPLv3
Summary:       Music player with a waveform progress bar
Version:       _N_VERS_
Release:       1
Source:        tarball.tar.gz
%if 0%{?fedora_version} || 0%{?suse_version}
BuildRequires: qt-devel gcc-c++
%else
BuildRequires: libqt-devel
%endif
BuildRequires: gstreamer1-devel gstreamer1-plugins-base-devel zip libX11-devel taglib-devel
BuildRoot:     %{_topdir}/%{name}-%{version}-root

%description
Music player with a waveform progress bar.

%prep
%setup -q

%build
%if 0%{?fedora_version}
%define QMAKE qmake-qt4
%define LRELEASE lrelease-qt4
%else
%define QMAKE qmake
%define LRELEASE lrelease
%endif
QMAKE=%{QMAKE} LRELEASE=%{LRELEASE} ./configure --taglib --no-update-check --prefix=%{buildroot}%{_prefix}
make

%install
make install

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%{_datadir}/%{name}/skins/*
%{_datadir}/%{name}/i18n/*
%{_datadir}/icons/*
%{_datadir}/applications/%{name}.desktop

%package  gstreamer
Summary:  GStreamer backend for Nulloy
Requires: nulloy
%description gstreamer
GStreamer backend for Nulloy.
%files gstreamer
%defattr(-,root,root)
%{_prefix}/lib/%{name}/plugins/libplugin_gstreamer.so

%package  taglib
Summary:  TagLib backend for Nulloy
Requires: nulloy
%description taglib
TagLib backend for Nulloy.
%files taglib
%defattr(-,root,root)
%{_prefix}/lib/%{name}/plugins/libplugin_taglib.so

%changelog

