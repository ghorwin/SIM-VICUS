#!/bin/bash

# Execute in top-level directory, i.e. 
#
# update_source_code_archive.sh

rm -rf simvicus-*/ > /dev/null

echo "Removed all previous files in directories 'simvicus-*'."
echo "Press any key to continue or CTRL+C to abort!"
read -t 10 -n 1

# clone/update current source repository
echo "*** STEP 1 : cloning SIM-VICUS.git ***" &&

if [ ! -d "SIM-VICUS-git" ]; then
  echo "Cloning github repo (creating a copy of the parent's git repo)" &&
  rsync -a --delete --exclude="debian*" ../ SIM-VICUS-git/
  # git clone https://github.com/ghorwin/SIM-VICUS.git SIM-VICUS-git
fi &&
echo "Reverting local changes and pulling newest revisions from github" && 
(cd SIM-VICUS-git && git reset --hard HEAD && git clean -fdx && git pull --rebase) &&
du -h --summarize SIM-VICUS-git/  && 

# extract version number
VERSION=$(python3 scripts/extractVersion.py SIM-VICUS-git/externals/Vicus/src/VICUS_Constants.cpp) &&
TARGETDIR=simvicus-$VERSION &&

# copy directory
echo "*** STEP 2 : Creating src directory $TARGETDIR ***" &&
if [ ! -d $TARGETDIR ]; then
	mkdir $TARGETDIR
fi &&
rsync -a --delete --exclude=".*" SIM-VICUS-git/ $TARGETDIR/ && 
du -h --summarize $TARGETDIR/ &&

# step 3 - remove unwanted files that should not go into the source code archive
#          end help reducing archive size

echo "*** STEP 3 : Cleaning out source directory ***" &&
rm -rf $TARGETDIR/third-party &&
rm -rf $TARGETDIR/doc &&
rm -rf $TARGETDIR/docs &&
rm -rf $TARGETDIR/data &&
rm -rf $TARGETDIR/debian &&
rm -rf $TARGETDIR/NandradDevTests &&
rm -rf $TARGETDIR/externals/clipper/doc &&
rm -rf $TARGETDIR/externals/qwt/doc/ &&
rm -rf $TARGETDIR/externals/qwt/playground/ &&
du -h --summarize $TARGETDIR/ &&

echo "*** STEP 4 : Copy top-level CMakeLists.txt file ***" &&

cp CMakeLists.txt $TARGETDIR/ &&

echo "*** STEP 5 : Creating source code archive ***" &&

# create tar.gz or tar.xz (the latter has better compression)
# tar -czvf simvicus_$VERSION.orig.tar.gz $TARGETDIR/
tar cf - $TARGETDIR/ | xz -z - > simvicus_$VERSION.orig.tar.xz &&
du -h --summarize simvicus_$VERSION.orig.tar.xz
