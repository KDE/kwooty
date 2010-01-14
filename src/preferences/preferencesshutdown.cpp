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

#include "shutdownmanager.h"
#include "centralwidget.h"
#include "utility.h"
using namespace UtilityNamespace;


PreferencesShutdown::PreferencesShutdown(CentralWidget* centralWidget) {

    setupUi(this);
    this->setupConnections();

    // add power management options :
    QMap<QString, QString> iconAvailableShutdownMap = centralWidget->getShutdownManager()->retrieveIconAvailableShutdownMap();
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
    kcfg_timerRadioButton->setText(kcfg_shutdownMethods->currentText() + ' ' + i18n("in:"));

    // set text jobsRadioButton text according to choosen shutdown method :
    kcfg_jobsRadioButton->setText(kcfg_shutdownMethods->currentText() + ' ' + i18n("when all jobs are finished"));


    // set text pausedShutdown text according to choosen shutdown method :
    QString shutdownMethodText = kcfg_shutdownMethods->currentText();
    kcfg_pausedShutdown->setText(i18n("Do not") + ' ' +
                                 shutdownMethodText.left(1).toLower() +
                                 shutdownMethodText.right(shutdownMethodText.size() - 1) + ' ' +
                                 i18n("if jobs are finished but paused files remain"));

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




