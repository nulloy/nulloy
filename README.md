# ![Icon](http://nulloy.com/files/github-icon.png) Nulloy Music Player http://nulloy.com

<img src="http://nulloy.com/files/screen.png" height="250">
<img src="http://nulloy.com/files/screen_1.png" height="250">
<img src="http://nulloy.com/files/screen_2.png" height="250">

[![Build Status](https://travis-ci.org/nulloy/nulloy.svg?branch=master)](https://travis-ci.org/nulloy/nulloy)

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
configure
mingw32-make
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
yum install gcc-c++ qt-devel gstreamer1{-plugins-base,}-devel zip libX11-devel taglib-devel
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
