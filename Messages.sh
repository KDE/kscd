#! /bin/sh
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT *.cpp gui/*.cpp -o $podir/kscd.pot
rm -f rc.cpp
