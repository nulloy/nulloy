# ![logo](src/icons/icon-32.png) Nulloy Music Player http://nulloy.com

[![Build Status](https://travis-ci.org/sergey-vlasov/nulloy.svg?branch=devel)](https://travis-ci.org/sergey-vlasov/nulloy)

**Testing PPA:** [ppa:sergey-vlasov/nulloy-testing](http://launchpad.net/~sergey-vlasov/+archive/nulloy-testing)



## Windows Build Instructions

### Prerequisites
* Qt 4.x MinGW libraries for Windows http://qt-project.org/downloads
* MinGW http://sourceforge.net/projects/mingw-w64/
* GStreamer Build & SDK http://code.google.com/p/ossbuild/
* CMake http://www.cmake.org/
* TagLib Source http://taglib.github.io/
* Zlib precompiled dlls http://zlib.net/
* 7zip http://www.7-zip.org/

### Environment Setup

Extract and/or install the downloads. Create ```vars.bat``` file with:

```bat
set QTDIR=C:\qt-libs-dir
set TAGLIB_DIR=C:\taglib-source-dir
set ZLIB_DIR=C:\zlib-dlls-dir
set PATH=%QTDIR%\bin;C:\mingw-dir\bin;C:\Program Files\7-Zip;%ZLIB_DIR%;%PATH%
```

Create a shortcut from ```vars.bat``` and set target as ```%COMSPEC% /k "C:\vars.bat"```. Open the shortcut.

### Build TagLib

```bat
cd %TAGLIB_DIR%
cmake -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=%TAGLIB_DIR% -DCMAKE_RELEASE_TYPE=Release -DENABLE_STATIC=ON -DENABLE_STATIC_RUNTIME=ON .
mingw32-make
mingw32-make install
```

### Build & Run Nulloy

```bat
cd C:\nulloy.git
configure --force-version 1-testing
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
sudo port install pkgconfig getopt qt4-mac gstreamer010{,-gst-plugins-base} zlib taglib
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
sudo port install gstreamer010-gst-plugins-{good,bad,ugly}
```



## Linux Build Instructions

### Installing Dependences

#### DEB-based distro

```sh
apt-get install g++ libqt4-dev qt4-qmake libgstreamer{-plugins-base,}0.10-dev zip libx11-dev libtag1-dev
```

#### RPM-based distro

```sh
yum install gcc-c++ qt-devel gstreamer{-plugins-base,}-devel zip libX11-devel taglib-devel
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
apt-get install gstreamer0.10{-ffmpeg,-plugins-{good,bad,ugly}}
```

#### RPM-based distro

```sh
yum install gstreamer{-ffmpeg,-plugins-{good,bad,ugly}}
```
