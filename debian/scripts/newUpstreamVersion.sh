#!/bin/bash

# run this script after changelog was updated
# set upstream version and target version below

DEB_VERSION=$(dpkg-parsechangelog --show-field Version)
VERSION=${DEB_VERSION%%-*}
TARGETDIR="simvicus-$VERSION"

echo "Building source package $DEB_VERSION for upstream version $VERSION" && 
if [ ! -d ../simvicus-$VERSION ]; then
	echo "Missing source directory ../simvicus-$VERSION"
	exit 1
fi &&

rm -rf simvicus-*
rm -rf simvicus_*
echo "Was the changelog updated correctly? Press any key or CTRL+C to abort!" &&
read -t 10 &&


# copy src tarball to this directory
cp ../simvicus_$VERSION.orig.tar.xz . && 
# copy src directory
cp -R ../simvicus-$VERSION/ .  &&
cp -R debian simvicus-$VERSION/ &&
echo "***  copied sources and debian directory, creating source deb ***" && 

cd $TARGETDIR && 
dpkg-buildpackage -S && 

echo "*** running lintian ***" &&

lintian -EvI --pedantic --show-overrides --color=auto ../simvicus_${DEB_VERSION}_source.changes && 

echo "Upload source archive to Launchpad?" &&
echo "Press any key to continue or CTRL+C to abort!" &&
read -t 10 -n 1 &&
dput ppa:ghorwin/sim ../simvicus_${DEB_VERSION}_source.changes
