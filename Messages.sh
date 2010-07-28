#!/bin/bash

XGETTEXT="xgettext --from-code=UTF-8 -C --kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 "
EXTRACTRC="extractrc"

export XGETTEXT EXTRACTRC

echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> rc.cpp
echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> rc.cpp 

SOURCE_FILES=`find . -name \*.ui -o -name \*.rc -o -name \*.kcfg`
$EXTRACTRC $SOURCE_FILES >> rc.cpp

LIST=`find . -name \*.cpp -o -name \*.h`
$XGETTEXT $LIST -o po/kwooty.pot

rm -f rc.cpp
