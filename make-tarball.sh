#!/bin/bash -exu

export LC_ALL=C
cd "$(dirname $0)"

if [[ $(head -n1 ChangeLog) =~ \*[[:space:]]([0-9\.]+),[[:space:]](.*) ]]; then
  VERSION=${BASH_REMATCH[1]}
else
  echo failed to parse ChangeLog
  exit 1
fi

TARBALL_NAME=nulloy-$VERSION
TMP_DIR=$(mktemp -d)
TARBALL_DIR=$TMP_DIR/$TARBALL_NAME
mkdir $TARBALL_DIR
git diff > $TARBALL_DIR/diff.patch
GIT_WORK_TREE=$TARBALL_DIR git checkout -f
pushd $TARBALL_DIR
patch -i diff.patch -p1
rm -f .gitignore make-tarball.sh .travis.yml README.md diff.patch
popd
tar -C $TMP_DIR -zcpf $TARBALL_NAME.tar.gz $TARBALL_NAME
rm -rf $TMP_DIR

readlink -f $TARBALL_NAME.tar.gz
