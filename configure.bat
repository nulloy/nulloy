@ECHO off

set BASENAME=%0
set QMAKE=qmake

set DEBUG=no
set CONSOLE=no

set HAVE_GSTREAMER=no
set HAVE_VLC=no

set FORCE_VERSION=no
set BUILD_GSTREAMER=yes
set BUILD_GSTREAMER_TAGREADER=no
set BUILD_TAGLIB=yes
set BUILD_VLC=no
set BUILD_PHONON=no
set SUPPORT_SKINS=yes

:getopt
	shift
	if "%0" == "--no-gstreamer" (
		set BUILD_GSTREAMER=no
		goto getopt
	)
	if "%0" == "--gstreamer-tagreader" (
		set BUILD_GSTREAMER_TAGREADER=yes
		goto getopt
	)
	if "%0" == "--vlc" (
		set BUILD_VLC=yes
		goto getopt
	)
	if "%0" == "--phonon" (
		set BUILD_PHONON=yes
		goto getopt
	)
	if "%0" == "--no-taglib" (
		set BUILD_TAGLIB=no
		goto getopt
	)
	if "%0" == "--no-skins" (
		set SUPPORT_SKINS=no
		goto getopt
	)
	if "%0" == "--force-version" (
		set FORCE_VERSION=%1
		shift
		goto getopt
	)
	if "%0" == "--debug" (
		set DEBUG=yes
		goto getopt
	)
	if "%0" == "--console" (
		set CONSOLE=yes
		goto getopt
	)
	if "%0" == ""       goto getopt_finished
	if "%0" == "-h"     goto help
	if "%0" == "--help" goto help
	if "%0" == "/?"     goto help
	echo %BASENAME%: invalid option '%0'
	goto try_help

:try_help
	echo.
	echo Try `%BASENAME% --help' for more information
	goto end

:help
	echo Usage:  %BASENAME% [options]
	echo     --no-gstreamer              do not build GStreamer plugins
	echo     --gstreamer-tagreader       build GStreamer TagReader plugin
	echo     --vlc                       build VLC plugins
	echo     --phonon                    build Phonon plugins
	echo     --no-taglib                 do not build with TagLib
	echo     --no-skins                  disable skins support
	echo     --console                   build with console output support
	echo     --force-version VERSION     overrides version.pri
	echo     --debug                     build in debug mode
	goto end


:getopt_finished


%QMAKE% -v > NUL 2>&1
if errorlevel 1 goto qmake_not_found
goto qmake_ok

:qmake_not_found
echo %BASENAME%: Unable to find 'qmake'. Check the PATH environment variable.
goto end

:qmake_ok

set QMAKE_CACHE=.qmake.cache
echo. > %QMAKE_CACHE%

if "%BUILD_GSTREAMER%" == "yes"           echo CONFIG += gstreamer>> %QMAKE_CACHE%
if "%BUILD_GSTREAMER_TAGREADER%" == "yes" echo CONFIG += gstreamer-tagreader>> %QMAKE_CACHE%
if "%BUILD_VLC%" == "yes"                 echo CONFIG += vlc>> %QMAKE_CACHE%
if "%BUILD_PHONON%" == "yes"              echo CONFIG += phonon>> %QMAKE_CACHE%
if "%BUILD_TAGLIB%" == "yes"              echo CONFIG += taglib>> %QMAKE_CACHE%
if "%SUPPORT_SKINS%" == "no"              echo CONFIG += no-skins>> %QMAKE_CACHE%
if "%CONSOLE%" == "yes"                   echo CONFIG += console>> %QMAKE_CACHE%
if "%DEBUG%" == "yes" (
	echo CONFIG += debug>> %QMAKE_CACHE%
) else (
	echo CONFIG += release>> %QMAKE_CACHE%
)

if not "%FORCE_VERSION%" == "no" (
	echo Forced version: %FORCE_VERSION%
	echo N_CONFIG_FORCE_VERSION = %FORCE_VERSION%>> %QMAKE_CACHE%
)

echo N_CONFIG_SUCCESS = yes>> %QMAKE_CACHE%

echo.
echo Running qmake...

%QMAKE%

echo Nulloy has been configured. Now run `make'.
echo.

:end

