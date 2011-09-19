#! /bin/bash

if [ $# != 1 ]; then
	echo "`basename $0`: wrong arguments"
	echo
	echo "Usage:  `basename $0` PREFIX"
	echo
	exit 1
fi

PREFIX=$1
PWD=`dirname "$0"`

for file in $PWD/icon-*; do
	if [[ "$file" =~ icon-([0-9]+).png ]]; then
		SIZE=${BASH_REMATCH[1]}
		DIR=$PREFIX/share/icons/hicolor/$SIZE"x"$SIZE/apps
		if [[ ! -d "$DIR" ]]; then
			mkdir -p $DIR
		fi
		cp $file $DIR/nulloy.png
		echo "cp $file $DIR/nulloy.png"
	fi
done

exit 0
