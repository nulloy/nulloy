#! /bin/bash

BASENAME=`basename $0`
TRY_HELP="Try \`$BASENAME --help' for more information"

FORCE_VERSION=no
VERSION=

echo_help()
{
	echo "Usage:  $BASENAME [options]"
	echo '   --force-version VERSION      overrides top most version in ChangeLog'
	echo '   -h, --help                   print this message'
	echo
}

args=`getopt -n$BASENAME -o h --longoptions="force-version: help" -- "$@"`
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

DIST_DIR=nulloy-$VERSION
rm -rf $DIST_DIR
mkdir $DIST_DIR
GIT_WORK_TREE=$DIST_DIR git checkout -f
cd $DIST_DIR &&
sed -i 's/\(.*N_VERSION = \)$$system.*/\1'$VERSION'/g' src/version.pri
find obs -type f -exec sed -i 's/_N_VERS_/'$VERSION'/g' {} +
rm -f .gitignore src/version-git.sh
src/changelog.sh -i ChangeLog -c "Sergey Vlasov <sergey@vlasov.me>" -p nulloy -r obs/nulloy.changes -d obs/debian.changelog -f $VERSION
cd -
tar zcpf $DIST_DIR.tar.gz $DIST_DIR
rm -rf $DIST_DIR
