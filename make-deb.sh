#! /bin/bash

# prepare tarball
. make-dist.sh

# prepare directories
DEB_DIR=$DIST_DIR-debianized
rm -rf $DEB_DIR nulloy_$VERSION*
tar -xzf $DIST_DIR.tar.gz
ln -s $DIST_DIR.tar.gz $DIST_DIR.orig.tar.gz
mv $DIST_DIR $DEB_DIR
cd $DEB_DIR
mv obs debian

# remove obs-specific stuff
rm -rf debian/nulloy* *.patch

# remove "debian." prefix
find debian/ -type f | awk -F/ '{print $NF}' | while read file; do mv debian/$file debian/${file#*.}; done

# add a new record to debian changelog
if ! grep -qw $VERSION debian/changelog; then
	CHANGELOG=debian/changelog
	CHANGELOG_PREP=debian/changelog_prepend
	echo "nulloy ($VERSION-1) unstable; urgency=low" > $CHANGELOG_PREP
	echo "" >> $CHANGELOG_PREP
	echo "  * $VERSION build" >> $CHANGELOG_PREP
	echo "" >> $CHANGELOG_PREP
	USERNAME=`whoami`
	USERNAME_FULL=`getent passwd $USERNAME | awk -F':' '{gsub(",", "",$5); print $5}'`
	echo " -- $USERNAME_FULL <$USERNAME@`hostname`>  `date +\"%a, %d %b %Y %T %z\"`" >> $CHANGELOG_PREP
	echo "" >> $CHANGELOG_PREP
	cat $CHANGELOG >> $CHANGELOG_PREP
	mv $CHANGELOG_PREP $CHANGELOG
fi

# build a deb
dpkg-buildpackage -sa -rfakeroot

cd -
rm -rf $DEB_DIR
