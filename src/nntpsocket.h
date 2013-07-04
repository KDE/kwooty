/***************************************************************************
 *   Copyright (C) 2013 by Xavier Lefage                                   *
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


#ifndef NNTPSOCKET_H
#define NNTPSOCKET_H

#include <QObject>
#include <QTimer>
#include <QSslSocket>
#include <QIODevice>

#include "nntpclient.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class ClientManagerConn;

class NntpSocket : public QSslSocket {


    Q_OBJECT

public:

    enum SegmentDownload { SegmentDownloading,
                           SegmentDownloadFinished
                       };

    NntpSocket(ClientManagerConn* parent);
    ~NntpSocket();
    bool isSocketUnconnected() const;
    bool isSocketConnected() const;
    void sendBodyCommandToServer(const QString&);
    void sendQuitCommandToServer();
    void sendUserCommandToServer(const QString&);
    void sendPasswordCommandToServer(const QString&);
    void connectToHostEncrypted(const QString&, quint16, OpenMode = ReadWrite);
    void connectToHost(const QString&, quint16, OpenMode = ReadWrite);
    void tryToReconnect();
    void connected();
    void abort();
    void retryDownloadDelayed();
    void checkRateControlTimer();
    QByteArray readChunck(const qint64&, const int&);
    QByteArray readAll();
    void manageBuffer(const SegmentDownload&);
    void notifyClientStatus(NntpClient::NntpClientStatus, NntpClient::TimerJob);
    int readAnswer();
    void quitAndReconnectInMs(const int&);
    void dataReadPending();
    void dataReadComplete();


private:

    ClientManagerConn* parent;

    QTimer* tryToReconnectTimer;
    QTimer* idleTimeOutTimer;
    QTimer* serverAnswerTimer;
    QTimer* rateControlTimer;
    bool certificateVerified;
    void setupConnections();
    void stopAllTimers();
    void sendCommand(const QString&);
    void dataReadArrived();
    int missingBytes;


signals:
    void downloadSegmentFromServerSignal();
    void answerTimeOutSignal();
    void socketEncryptedInfoSignal(bool, QString, QString, QStringList);
    void tryToReconnectSignal();


public slots:
    void answerTimeOutSlot();
    void idleTimeOutSlot();
    void rateControlSlot();
    void tryToReconnectSlot();

private slots:
    void socketEncryptedSlot();
    void peerVerifyErrorSlot();
};

#endif // NNTPSOCKET_H
