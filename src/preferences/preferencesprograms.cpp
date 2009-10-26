/***************************************************************************
 *   Copyright (C) 2009 by Xavier Lefage                                   *
 *   xavier.kwooty@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "preferencesprograms.h"

#include <KDebug>
#include <QFile>

#include "utility.h"
#include "settings.h"

PreferencesPrograms::PreferencesPrograms()
{
    setupUi(this);
    iconLoader = new KIconLoader();
}

PreferencesPrograms::~PreferencesPrograms()
{
    delete iconLoader;
}



void PreferencesPrograms::aboutToShowSettingsSlot(){

    QString labelIcon;
    QString labelText;

    // display information about par2 program :
    bool isProgramFound = false;
    QString programPath = Utility::searchExternalPrograms(UtilityNamespace::repairProgram, isProgramFound);
    this->displayProgramInfo(isProgramFound, programPath, par2LabelIcon, par2LabelText, UtilityNamespace::repairProgram);

    // display information about unrar program :
    isProgramFound = false;
    programPath = Utility::searchExternalPrograms(UtilityNamespace::extractProgram, isProgramFound);
    this->displayProgramInfo(isProgramFound, programPath, unrarLabelIcon, unrarLabelText, UtilityNamespace::extractProgram);

}


void PreferencesPrograms::displayProgramInfo(const bool isProgramFound, const QString& path, QLabel* labelIcon, QLabel* labelText, const QString& program){

    // indicate path to binary file if program has been found :
    if (isProgramFound) {
        labelIcon->setPixmap(iconLoader->loadIcon("dialog-ok", KIconLoader::Small));
        labelText->setText("<b>" + program + "</b> " +  i18n("program found in ") + path);

        // enable group box if program found :
        this->enableGroupBox(true, program);

    }
    // indicate that program has not been found :
    else {
        labelIcon->setPixmap(iconLoader->loadIcon("dialog-close", KIconLoader::Small));
        labelText->setText("<b>" + program + "</b> " +  i18n("program not found"));

        // disable group box if program not found :
        this->enableGroupBox(false, program);

    }

}


void PreferencesPrograms::enableGroupBox(bool isEnabled, const QString& program) {

     // enable/disable auto repair settings if par2 program found/not found :
    if (program == UtilityNamespace::repairProgram) {
        kcfg_groupBoxAutoRepair->setEnabled(isEnabled);
    }
    // enable/ disable auto extract settings if unrar program found/not found :
    else {
        kcfg_groupBoxAutoDecompress->setEnabled(isEnabled);
    }

}



