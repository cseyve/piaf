#!/bin/bash

echo "Building all sources..."
echo -n "For Qt, using qmake="
which qmake

SYSTEMUNAME=$(uname)
if [ $SYSTEMUNAME == "Darwin" ]; then
        QMAKEOPT="-spec macx-clang"
else
        QMAKEOPT=
fi

for src in opencv_*.cpp ;
do
	export SRCNAME=${src%.cpp}
	echo " + building OpenCV based $src..."
	qmake $QMAKEOPT opencv_template.pro && make $@ || exit 0
done

export SRCNAME=example
echo " + building $SRCNAME"
qmake fftw_template.pro && make $@ 
#|| exit 0

export SRCNAME=outoffocus
echo " + building $SRCNAME"
qmake fftw_template.pro && make $@ 
#|| exit 0

echo "Done"
