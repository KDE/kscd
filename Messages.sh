#! /bin/sh
$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp || exit 11
$XGETTEXT *.cpp gui/*.cpp -o $podir/kscd.pot
rm -f rc.cpp
