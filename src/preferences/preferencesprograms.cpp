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

#include "utilities/utility.h"
#include "kwootysettings.h"

PreferencesPrograms::PreferencesPrograms()
{
    setupUi(this);
    this->setupConnections();

    // init combobox process priority list :
    QStringList processPriorityList;
    processPriorityList.append(i18n("Low"));
    processPriorityList.append(i18n("Lowest"));
    processPriorityList.append(i18n("Custom"));

    kcfg_verifyProcessValues->addItems(processPriorityList);
    kcfg_verifyProcessValues->setToolTip(this->buildNicePriorityToolTip());
    kcfg_verifyNiceValue->setPrefix("+");

    kcfg_extractProcessValues->addItems(processPriorityList);
    kcfg_extractProcessValues->setToolTip(this->buildNicePriorityToolTip());
    kcfg_extractNiceValue->setPrefix("+");

    this->verifyProcessPriorityChangedSlot();
    this->extractProcessPriorityChangedSlot();


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
    QString programPath;
    bool isProgramFound = false;

    programPath = Utility::searchExternalPrograms(UtilityNamespace::repairProgram, isProgramFound);
    this->displayProgramInfo(isProgramFound, programPath, par2LabelIcon, par2LabelText, UtilityNamespace::repairProgram);

    // display information about unrar program :
    isProgramFound = false;
    programPath = Utility::searchExternalPrograms(UtilityNamespace::rarExtractProgram, isProgramFound);
    this->displayProgramInfo(isProgramFound, programPath, unrarLabelIcon, unrarLabelText, UtilityNamespace::rarExtractProgram);


    // display information about 7z program :
    isProgramFound = false;
    programPath = Utility::searchExternalPrograms(UtilityNamespace::sevenZipExtractProgram, isProgramFound);

    QString programName;

    if (!programPath.isEmpty()) {
        programName = programPath.split("/").takeLast();
    }
    else {
        programName = UtilityNamespace::sevenZipExtractProgram.split(";").takeFirst();
    }

    this->displayProgramInfo(isProgramFound, programPath, sevenZipLabelIcon, sevenZipLabelText, programName);

}


void PreferencesPrograms::setupConnections() {

    // show/hide text when priority method is changed :
    connect (kcfg_verifyProcessValues, SIGNAL(currentIndexChanged(int)), this, SLOT(verifyProcessPriorityChangedSlot()));
    connect (kcfg_extractProcessValues, SIGNAL(currentIndexChanged(int)), this, SLOT(extractProcessPriorityChangedSlot()));


}




void PreferencesPrograms::displayProgramInfo(const bool isProgramFound, const QString& path, QLabel* labelIcon, QLabel* labelText, const QString& program){

    // indicate path to binary file if program has been found :
    if (isProgramFound) {
        labelIcon->setPixmap(iconLoader->loadIcon("dialog-ok", KIconLoader::Small));
        labelText->setText(i18n("<b>%1</b> program found: %2", program, path));

        // enable group box if program found :
        this->enableGroupBox(true, program);

    }
    // indicate that program has not been found :
    else {
        labelIcon->setPixmap(iconLoader->loadIcon("dialog-close", KIconLoader::Small));
        labelText->setText(i18n("<b>%1</b> program not found", program));

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

        // enable extract group box is unrar or 7z program has been found :
        if (!kcfg_groupBoxAutoDecompress->isEnabled()) {

            kcfg_groupBoxAutoDecompress->setEnabled(isEnabled);

        }
    }

}





void PreferencesPrograms::verifyProcessPriorityChangedSlot() {

    // custom priority is not selected, hide nice process comboBox :
    if (kcfg_verifyProcessValues->currentIndex() != (kcfg_verifyProcessValues->count() - 1)) {
        verifyNiceValueText->hide();
        kcfg_verifyNiceValue->hide();
    }
    else if (verifyNiceValueText->isHidden()) {
        verifyNiceValueText->show();
        kcfg_verifyNiceValue->show();
    }


}

void PreferencesPrograms::extractProcessPriorityChangedSlot() {

    // custom priority is not selected, hide nice process comboBox :
    if (kcfg_extractProcessValues->currentIndex() != (kcfg_extractProcessValues->count() - 1)) {
        extractNiceValueText->hide();
        kcfg_extractNiceValue->hide();
    }
    else if (extractNiceValueText->isHidden()) {
        extractNiceValueText->show();
        kcfg_extractNiceValue->show();
    }

}



QString PreferencesPrograms::buildNicePriorityToolTip() {

    QString currentTip;
    currentTip.append("<table style='white-space: nowrap'>");
    currentTip.append(Utility::buildToolTipRow(i18n("Low:"), i18n("nice value set to +10")));
    currentTip.append(Utility::buildToolTipRow(i18n("Lowest:"), i18n("nice value set to +19")));
    currentTip.append(Utility::buildToolTipRow(i18n("Custom:"), i18n("choose nice value")));
    currentTip.append("</table>");
    return currentTip;
}

