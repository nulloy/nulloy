#! /bin/bash
. make-dist.sh
DEB_DIR=$DIST_DIR-debianized
rm -rf $DEB_DIR nulloy_$VERSION*
tar -xzf $DIST_DIR.tar.gz
ln -s $DIST_DIR.tar.gz $DIST_DIR.orig.tar.gz
mv $DIST_DIR $DEB_DIR
cd $DEB_DIR
mv obs debian
rm -rf debian/nulloy* *.patch
find debian/ -type f | awk -F/ '{print $NF}' | while read file; do mv debian/$file debian/${file#*.}; done
dpkg-buildpackage -sa -rfakeroot
cd -
rm -rf $DEB_DIR
