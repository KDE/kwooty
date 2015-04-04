#!/bin/bash

echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> rc.cpp
echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> rc.cpp 

SOURCE_FILES=`find . -name \*.ui -o -name \*.rc -o -name \*.kcfg`
$EXTRACTRC $SOURCE_FILES >> rc.cpp

LIST=`find . -name \*.cpp -o -name \*.h`
$XGETTEXT $LIST -o $podir/kwooty.pot

rm -f rc.cpp

