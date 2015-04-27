/***************************************************************************
 *   Copyright (C) 2012 by Xavier Lefage                                   *
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


#ifndef SESSIONGNOME2_H
#define SESSIONGNOME2_H

#include <solid/powermanagement.h>
using namespace Solid::PowerManagement;



#include "sessionbase.h"

class SessionGnome2 : public SessionBase {


public:

    SessionGnome2(ShutdownManager*);
    QList<UtilityNamespace::SystemShutdownType> retrieveAvailableShutdownMethods();

private:

    void requestShutdown() Q_DECL_OVERRIDE;
    void requestSuspend() Q_DECL_OVERRIDE;

};

#endif // SESSIONGNOME2_H
