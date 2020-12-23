# ![Icon](http://nulloy.com/files/github-icon.png) Nulloy Music Player http://nulloy.com

<img src="http://nulloy.com/files/screen.png" height="250">
<img src="http://nulloy.com/files/screen_1.png" height="250">
<img src="http://nulloy.com/files/screen_2.png" height="250">

[![Build Status](https://travis-ci.org/nulloy/nulloy.svg?branch=master)](https://travis-ci.org/nulloy/nulloy)

## Windows Build Instructions

### Prerequisites

* Qt 5 offline installer https://www.qt.io/offline-installers/
* GStreamer 1.0 MinGW 32-bit (runtime and development installers) http://gstreamer.freedesktop.org/download/
* pkg-config and its dependencies (glib and gettext-runtime) https://download.gnome.org/binaries/win32/dependencies/, https://download.gnome.org/binaries/win32/glib/
* CMake http://www.cmake.org/
* TagLib source code https://github.com/taglib/taglib/
* 7zip http://www.7-zip.org/

Disconnect from the Internet to skip creating Qt account. Run Qt 5 offline installer and select only the following components:
```
+ Qt
  + Qt 5
    - Qt Prebuilt Components for MinGW 32-bit
    - Qt Script
  + Developer and Designer Tools
    - MinGW 32-bit toolchain
```

Extract pkg-config and its dependencies into `C:\Downloads\pkg-config`.

Install GStreamer runtime and development packages. Open command prompt (Start menu -> type "cmd") and execute:
```bat
DEL C:\gstreamer\1.0\mingw_x86\bin\pkg-config.exe
DEL C:\gstreamer\1.0\mingw_x86\bin\libstdc++-6.dll
DEL C:\gstreamer\1.0\mingw_x86\bin\libtag.dll
DEL C:\gstreamer\1.0\mingw_x86\lib\libstdc++.a
DEL C:\gstreamer\1.0\mingw_x86\lib\libtag.a
DEL C:\gstreamer\1.0\mingw_x86\lib\pkgconfig\taglib.pc
```

Extract and / or install the rest of the prerequisites.

### Build TagLib

Run Qt MinGW terminal (Start menu -> `Qt` -> `Qt (MinGW 32-bit)`) and execute:

```bat
C:\Downloads\taglib
cmake.exe -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DZLIB_INCLUDE_DIR=C:\gstreamer\1.0\mingw_x86\include -DCMAKE_INSTALL_PREFIX="."
mingw32-make
mingw32-make install
```

### Build & Run Nulloy

In the same terminal execute:

```bat
set PATH=C:\Program Files\7-Zip;%PATH%
set PATH=C:\Downloads\pkg-config\bin;%PATH%
set PATH=C:\gstreamer\1.0\mingw_x86\bin;%PATH%
set PKG_CONFIG_PATH=C:\gstreamer\1.0\mingw_x86\lib\pkgconfig;%PKG_CONFIG_PATH%
set PKG_CONFIG_PATH=C:\Downloads\taglib\lib\pkgconfig;%PKG_CONFIG_PATH%

cd C:\Downloads\nulloy
configure.bat
mingw32-make
windeployqt Nulloy.exe
copy /B C:\gstreamer\1.0\mingw_x86\bin\*.dll . \Y
copy /B C:\Downloads\taglib\bin\libtag.dll . \Y
Nulloy.exe
```



## OSX Build Instructions

### Prerequisites
* Xcode Command Line Tools
* MacPorts http://www.macports.org/ or HomeBrew https://brew.sh/

### Environment Setup

Install Xcode Command Line Tools:

```sh
xcode-select --install
```

### Installing Dependences

First install either MacPorts or HomeBrew.

#### MacPorts

After installing MacPorts:

```sh
. ~/.profile
sudo port install pkgconfig qt4-mac gstreamer1{,-gst-plugins-base} taglib
```

#### HomeBrew

After installing HomeBrew:

```sh
brew install pkgconfig cartr/qt4/qt gstreamer gst-plugins-base taglib
```

### Build & Run Nulloy

```sh
cd nulloy.git
./configure
make
make install
./nulloy.app/Contents/MacOS/nulloy
```

### Optional

Install extra GStreamer plugins for more audio formats.

#### MacPorts

```sh
sudo port install gstreamer1-gst-plugins-{good,bad,ugly}
```

#### HomeBrew

```sh
brew install gst-plugins-{good,bad,ugly}
```



## Linux Build Instructions

### Installing Dependences

#### DEB-based distro

```sh
apt-get install g++ qt5-default qttools5-dev qtscript5-dev qtbase5-private-dev libqt5x11extras5-dev libgstreamer{-plugins-base,}1.0-dev zip libx11-dev libtag1-dev
```

#### RPM-based distro

```sh
yum install gcc-c++ qt5-qtbase-devel qt5-qttools-devel qt5-qttools-static qt5-qtscript-devel qt5-qtbase-private-devel qt5-linguist gstreamer1{-plugins-base,}-devel zip libX11-devel taglib-devel
```

### Build & Run Nulloy

```sh
cd nulloy.git
./configure
make
./nulloy
```

### Optional

Install extra GStreamer plugins for more audio formats

#### DEB-based distro

```sh
apt-get install gstreamer1.0-plugins-{good,bad,ugly}
```

#### RPM-based distro

```sh
yum install gstreamer1-plugins-{good,bad,ugly}}
```

## License
[GPL3](/LICENSE.GPL3)
