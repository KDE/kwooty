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
#include <QAbstractSocket>

#include "data/segmentdata.h"
#include "data/segmentinfodata.h"
#include "utilities/utility.h"
using namespace UtilityNamespace;

class ClientManagerConn;
class NntpSocket;

class NntpClient : public QObject
{

    Q_OBJECT
    Q_ENUMS(ServerAnswer)
    Q_ENUMS(NntpClientStatus)
    Q_ENUMS(NewSegmentRequest)
    Q_ENUMS(TimerJob)

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
                        NoSuchArticleMessageId = 430,
                        NoSuchArticleNumber = 423,
                        CommandNotPerformed = 503,
                        TransfertFailed = 436,
                        AccessDenied = 481
                      };

    enum NntpClientStatus { ClientIdle,
                            ClientDownload,
                            ClientSegmentRequest
                          };

    enum NewSegmentRequest { RequestNewSegment,
                             DoNotRequestNewSegment
                           };

    enum TimerJob { StartStopTimers,
                    DoNotTouchTimers
                  };

    enum ServerAnswerStatus { ServerFirstAnswerNotSent,
                              ServerFirstAnswerSent,
                              ServerConnectedPostingOk,
                              ServerDisconnectedPostingOk,
                              ServerDisconnected
                            };


    NntpClient(ClientManagerConn*);
    ~NntpClient();
    void downloadNextSegment(const SegmentData&);
    void noSegmentAvailable();
    bool isClientReady();
    void disconnectRequestByManager();
    void connectRequestByManager();
    NntpSocket* getTcpSocket();


private:
    static const int MAX_CONNECTING_LOOP = 5;

    ClientManagerConn* parent;
    NntpSocket* tcpSocket;
    QByteArray segmentByteArray;
    SegmentData currentSegmentData;
    NntpClient::NntpClientStatus clientStatus;
    NntpClient::ServerAnswerStatus serverAnswerStatus;
    int nntpError;
    int connectingLoopCounter;
    bool authenticationDenied;
    bool segmentProcessed;

    int notifyDownloadHasFinished(const UtilityNamespace::Article);
    bool downloadSegmentWithBackupServer();
    void setConnectedClientStatus(const NntpClientStatus, const TimerJob = StartStopTimers);
    void setupConnections();
    void getAnswerFromServer();
    void postDownloadProcess(UtilityNamespace::Article);
    void segmentDataRollBack();
    void requestNewSegment();
    void postProcessIfBackupServer(NewSegmentRequest = RequestNewSegment);
    void updateServerAnswerStatus(const ServerAnswerStatus);
    void retryDownloadDelayed(const int&);


Q_SIGNALS:
    void getNextSegmentSignal(ClientManagerConn*);
    void updateDownloadSegmentSignal(SegmentData);
    void connectionStatusPerServerSignal(int);
    void speedPerServerSignal(const SegmentInfoData);
    void nntpErrorPerServerSignal(const int);


public Q_SLOTS:
    void dataHasArrivedSlot();


private Q_SLOTS:
    void connectToHostSlot();
    void readyReadSlot();
    void connectedSlot();
    void errorSlot(QAbstractSocket::SocketError);
    void disconnectedSlot();
    void downloadSegmentFromServerSlot();
    void answerTimeOutSlot();

};

#endif // NNTPCLIENT_H
