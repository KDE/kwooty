/***************************************************************************
 *   Copyright (C) 2010 by Xavier Lefage                                   *
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

#include "preferencesshutdown.h"

#include <KDebug>

#include "core.h"
#include "shutdown/shutdownmanager.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;


PreferencesShutdown::PreferencesShutdown(Core* core) {

    setupUi(this);
    this->setupConnections();

    // add power management options :
    QMap<QString, QString> iconAvailableShutdownMap = core->getShutdownManager()->retrieveIconAvailableShutdownMap();
    QMapIterator<QString, QString> mapIterator(iconAvailableShutdownMap);

    // build comboBox :
    while (mapIterator.hasNext()) {

        mapIterator.next();
        kcfg_shutdownMethods->addItem(KIcon(mapIterator.key()), mapIterator.value());

    }

    // disable pausedShutdown if timerRadioButton is not checked :
    this->radioButtonToggledSlot();

}


void PreferencesShutdown::setupConnections() {

    // update text when shutdown method is changed :
    connect (kcfg_shutdownMethods, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChangedSlot()));

    // enable/disable pausedShutdown option :
    connect (kcfg_jobsRadioButton, SIGNAL(toggled(bool)), this, SLOT(radioButtonToggledSlot()));
    connect (kcfg_timerRadioButton, SIGNAL(toggled(bool)), this, SLOT(radioButtonToggledSlot()));
}



void PreferencesShutdown::currentIndexChangedSlot() {

    // set text timerRadioButton text according to choosen shutdown method :
    kcfg_timerRadioButton->setText(i18nc("%1 = Shutdown/Suspend to RAM/Suspend to disk",
                                         "%1 in:", kcfg_shutdownMethods->currentText()));

    // set text jobsRadioButton text according to choosen shutdown method :
    kcfg_jobsRadioButton->setText(i18nc("%1 = Shutdown/Suspend to RAM/Suspend to disk",
                                        "%1 when all jobs are finished", kcfg_shutdownMethods->currentText()));


    // set text pausedShutdown text according to choosen shutdown method :
    QString shutdownMethodText = kcfg_shutdownMethods->currentText();
    kcfg_pausedShutdown->setText(i18nc("%1%2 = shutdown/suspend to RAM/suspend",
                                       "Do not %1%2 if jobs are finished but paused files remain",
                                       shutdownMethodText.left(1).toLower(),
                                       shutdownMethodText.right(shutdownMethodText.size() - 1)));

}


void PreferencesShutdown::radioButtonToggledSlot() {

    // enable adavanced option if jobsRadioButton is checked :
    if (kcfg_jobsRadioButton->isChecked()) {
        kcfg_pausedShutdown->setEnabled(true);
    }
    else {
        kcfg_pausedShutdown->setEnabled(false);
    }

}





