#! /bin/bash
VERSION=`src/version.sh`
DIST_DIR=nulloy-$VERSION
rm -rf $DIST_DIR
mkdir $DIST_DIR
GIT_WORK_TREE=$DIST_DIR git checkout -f
cd $DIST_DIR &&
sed -i 's/#\(VERSION=\)_N_VERS_/\1'$VERSION'/g' src/version.sh
find obs -type f -exec sed -i 's/_N_VERS_/'$VERSION'/g' {} +
rm -f .gitignore
src/changelog.sh -i ChangeLog -c "Sergey Vlasov <sergey@vlasov.me>" -p nulloy -r obs/nulloy.changes -d obs/debian.changelog -u $VERSION
cd -
tar zcpf $DIST_DIR.tar.gz $DIST_DIR
rm -rf $DIST_DIR
