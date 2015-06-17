# Nulloy Music Player http://nulloy.com

[![Build Status](https://travis-ci.org/nulloy/nulloy.svg)](https://travis-ci.org/nulloy/nulloy)

**Testing PPA:** [ppa:nulloy/testing](https://launchpad.net/~nulloy/+archive/ubuntu/testing)



## Windows Build Instructions

### Prerequisites
* Qt 4.x MinGW build http://www.qt.io/download-open-source/
* MinGW-w64 http://mingw-w64.org/
* GStreamer 1.0 http://gstreamer.freedesktop.org/download/
* pkg-config http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/
* CMake http://www.cmake.org/
* TagLib https://github.com/taglib/taglib/
* 7zip http://www.7-zip.org/

### Environment Setup

Extract and/or install the downloads. Move ```pkg-config.exe``` to ```C:\mingw\bin```. Create ```vars.bat``` file with:

```bat
set QTDIR=C:\qt4
set TAGLIB_DIR=C:\taglib.git
set PKG_CONFIG_PATH=%GSTREAMER_1_0_ROOT_X86%\lib\pkgconfig;%TAGLIB_DIR%\lib\pkgconfig
set PATH=%QTDIR%\bin;%TAGLIB_DIR%\bin;C:\mingw\bin;C:\Program Files\7-Zip;%PATH%
```
Create a shortcut from ```vars.bat``` and set target as ```%COMSPEC% /k "C:\vars.bat"```. Open the shortcut.

### Build TagLib

```bat
cd %TAGLIB_DIR%
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DZLIB_INCLUDE_DIR=%GSTREAMER_1_0_ROOT_X86%\include -DCMAKE_INSTALL_PREFIX="."
mingw32-make
mingw32-make install
```

### Build & Run Nulloy

```bat
cd C:\nulloy.git
configure --taglib --force-version 1-testing
mingw32-make
Nulloy.exe
```



## OSX Build Instructions

### Prerequisites
* Xcode
* MacPorts Installer http://www.macports.org/

### Environment Setup

Install Xcode and then install Xcode Command Line Tools. Install MacPorts and open a terminal.

```sh
. ~/.profile
sudo port install pkgconfig getopt qt4-mac gstreamer1{,-gst-plugins-base} taglib
```

### Build & Run Nulloy

```sh
cd nulloy.git
./configure --taglib --no-app-bundle --force-version 1-testing
make
./nulloy
```

### Optional

Install extra GStreamer plugins for more audio formats

```sh
sudo port install gstreamer1-gst-plugins-{good,bad,ugly}
```



## Linux Build Instructions

### Installing Dependences

#### DEB-based distro

```sh
apt-get install g++ libqt4-dev qt4-qmake libgstreamer{-plugins-base,}1.0-dev zip libx11-dev libtag1-dev
```

#### RPM-based distro

```sh
yum install gcc-c++ qt-devel gstreamer1{-plugins-base,}-devel zip libX11-devel taglib-devel
```

### Build & Run Nulloy

```sh
cd nulloy.git
./configure --taglib --force-version 1-testing
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
