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

#ifndef NNTPCLIENT_H
#define NNTPCLIENT_H

#include <QObject>
#include <QSslSocket>
#include "data/segmentdata.h"
#include "utility.h"
using namespace UtilityNamespace;

class ClientManagerConn;

class NntpClient : public QObject
{

Q_OBJECT
Q_ENUMS(ServerAnswer)
Q_ENUMS(ServerStatus)

public:

    enum ServerAnswer { PasswordRequested = 381,
                        AuthenticationAccepted = 281,
                        BodyArticleFollows = 222,
                        GroupSeleted = 211,
                        QuitFromServer = 205,
                        ServerIsReadyPosting = 200,
                        ServerIsReadyNoPosting = 201,
                        AuthenticationDenied = 502,
                        IdleTimeout = 400,
                        AuthenticationRequired = 480,
                        AuthenticationRejected = 482,
                        NoSuchArticle = 430
                       };

    enum NntpClientStatus { ClientIdle,
                            ClientDownload,
                            ClientSegmentRequest
                       };


    NntpClient(ClientManagerConn* parent = 0);
    NntpClient();
    ~NntpClient();
    void downloadNextSegment(const SegmentData);
    void setConnectedClientStatus(const NntpClient::NntpClientStatus);

private:
    static const int MINUTES_TO_MILLISECONDS = 60000;

    ClientManagerConn* parent;
    SegmentData currentSegmentData;
    QSslSocket* tcpSocket;
    QByteArray segmentByteArray;
    QTimer* tryToReconnectTimer;
    QTimer* idleTimeOutTimer;
    QTimer* serverAnswerTimer;
    NntpClient::NntpClientStatus clientStatus;
    int nntpError;
    bool authenticationDenied;

    void connectToHost();
    void setupConnections();
    void getAnswerFromServer();
    void downloadSegmentFromServer();
    void postDownloadProcess(const UtilityNamespace::Article);
    void sendBodyCommandToServer();
    void sendQuitCommandToServer();
    void sendUserCommandToServer();
    void sendPasswordCommandToServer();
    void segmentDataRollBack();



signals:
    void getNextSegmentSignal(ClientManagerConn*);
    void updateDownloadSegmentSignal(SegmentData);
    void connectionStatusSignal(int);
    void encryptionStatusSignal(const bool, const QString = QString());
    void speedSignal(int);
    void saveFileErrorSignal(int);
    void nntpErrorSignal(const int);

public slots:
    void dataHasArrivedSlot();
    void answerTimeOutSlot();

private slots:
    void readyReadSlot();
    void connectedSlot();    
    void errorSlot(QAbstractSocket::SocketError);
    void idleTimeOutSlot();
    void disconnectedSlot();
    void tryToReconnectSlot();
    void socketEncryptedSlot();


};

#endif // NNTPCLIENT_H
