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

#ifndef CLIENTMANAGERCONN_H
#define CLIENTMANAGERCONN_H

#include "data/segmentdata.h"

class NntpClient;
class CentralWidget;


class ClientManagerConn : public QObject {

    Q_OBJECT

public:
    ClientManagerConn(CentralWidget*, int, int);
    ClientManagerConn();
    ~ClientManagerConn();
    int clientStatus;
    void processNextSegment(SegmentData);
    void noSegmentAvailable();
    int getClientId() const;


private:
    NntpClient* nntpClient;
    CentralWidget* parent;
    int clientId;
    QString hostName;
    QString login;
    QString password;
    int port;
    int disconnectTimeout;
    int connectionDelay;
    bool authentication;
    bool enableSSL;

    void updateSettings();


signals:

public slots:
    void settingsChangedSlot();

private slots:
    void initSlot();


};

#endif // CLIENTMANAGERCONN_H

