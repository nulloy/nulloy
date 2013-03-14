#! /bin/bash

BASENAME=`basename $0`
TRY_HELP="Try \`$BASENAME --help' for more information"

FORCE_VERSION=no
BUILD_PHONON=no
VERSION=

echo_help()
{
	echo "Usage:  $BASENAME [options]"
	echo '    --force-version VERSION      overrides top most version in ChangeLog'
	echo '    --phonon                     build Phonon plugins'
	echo '    -h, --help                   print this message'
	echo
}

args=`getopt -n$BASENAME -o h --longoptions="force-version: help phonon" -- "$@"`
if test $? != 0; then
	echo $TRY_HELP
	exit 1
fi

eval set -- $args
while [ $# -gt 0 ]; do
	if [ $1 == "--force-version" ]; then
		shift; FORCE_VERSION=$1
	elif [[ $1 == "-h"  || $1 == "--help" ]]; then
		echo_help; exit 0
	elif [ $1 == "--phonon" ]; then
		BUILD_PHONON=yes
	elif [ $1 == "--" ]; then
		shift
		if [ $# -ne 0 ]; then
			echo "$BASENAME: invalid arguments"
			echo $TRY_HELP; exit 1
		fi
		break
	fi
	shift
done

if [ $FORCE_VERSION != "no" ]; then
	VERSION=$FORCE_VERSION
else
	VERSION=`src/version-git.sh`
fi

# prepare directories
DIST_DIR=nulloy-$VERSION
rm -rf $DIST_DIR
mkdir $DIST_DIR
GIT_WORK_TREE=$DIST_DIR git checkout -f
cd $DIST_DIR &&

# replace version
sed -i 's/\(.*N_VERSION = \)$$system.*/\1'$VERSION'/g' src/version.pri
find obs -type f -exec sed -i 's/_N_VERS_/'$VERSION'/g' {} +
rm -f .gitignore src/version-git.sh

# generate debian changelog
src/changelog.sh -i ChangeLog -c "Sergey Vlasov <sergey@vlasov.me>" -p nulloy -r obs/nulloy.changes -d obs/debian.changelog -f $VERSION

# disable phonon plugins
if [ $BUILD_PHONON == "no" ]; then
	rm obs/debian.nulloy-phonon.install
	sed -i 's/--phonon //' obs/debian.rules
	sed -i 's/libphonon-dev, //' obs/debian.control
	# remove phonon package section
	sed -i '/Package: nulloy-phonon/,/^$/d' obs/debian.control
fi

cd -
tar zcpf $DIST_DIR.tar.gz $DIST_DIR
rm -rf $DIST_DIR
