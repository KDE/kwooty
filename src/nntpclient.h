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
#include "data/segmentinfodata.h"
#include "utility.h"
using namespace UtilityNamespace;

class ClientManagerConn;

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

    enum SegmentDownload { SegmentDownloading,
                           SegmentDownloadFinished
                         };

    NntpClient(ClientManagerConn* parent = 0);
    ~NntpClient();
    void downloadNextSegment(const SegmentData&);
    void noSegmentAvailable();
    bool isClientReady();
    void disconnectRequestByManager();
    void connectRequestByManager();


private:
    static const int MAX_CONNECTING_LOOP = 5;

    ClientManagerConn* parent;
    QSslSocket* tcpSocket;
    QByteArray segmentByteArray;
    QTimer* tryToReconnectTimer;
    QTimer* idleTimeOutTimer;
    QTimer* serverAnswerTimer;
    QTimer* rateControlTimer;
    SegmentData currentSegmentData;
    NntpClient::NntpClientStatus clientStatus;
    NntpClient::ServerAnswerStatus serverAnswerStatus;
    int nntpError;
    int connectingLoopCounter;
    bool authenticationDenied;
    bool certificateVerified;
    bool segmentProcessed;
    int missingBytes;

    int notifyDownloadHasFinished(const UtilityNamespace::Article);
    bool downloadSegmentWithBackupServer();
    bool isSocketUnconnected() const;
    bool isSocketConnected() const;
    void setConnectedClientStatus(const NntpClientStatus, const TimerJob = StartStopTimers);
    void connectToHost();
    void setupConnections();
    void getAnswerFromServer();
    void downloadSegmentFromServer();
    void postDownloadProcess(UtilityNamespace::Article);
    void sendBodyCommandToServer();
    void sendQuitCommandToServer();
    void sendUserCommandToServer();
    void sendPasswordCommandToServer();
    void sendCommand(const QString&);
    void segmentDataRollBack();
    void requestNewSegment();
    void postProcessIfBackupServer(NewSegmentRequest = RequestNewSegment);
    void updateServerAnswerStatus(const ServerAnswerStatus);
    void retryDownloadDelayed(const int&);
    void checkRateControlTimer();
    void manageSocketBuffer(const SegmentDownload&);


signals:
    void getNextSegmentSignal(ClientManagerConn*);
    void updateDownloadSegmentSignal(SegmentData);
    void connectionStatusPerServerSignal(int);
    void encryptionStatusPerServerSignal(const bool, const QString = QString(), const bool = false, const QString = QString(), const QStringList = QStringList());
    void speedPerServerSignal(const SegmentInfoData);
    void saveFileErrorSignal(const int);
    void nntpErrorPerServerSignal(const int);


public slots:
    void dataHasArrivedSlot();
    void answerTimeOutSlot();
    void idleTimeOutSlot();


private slots:
    void readyReadSlot();
    void connectedSlot();    
    void errorSlot(QAbstractSocket::SocketError);
    void disconnectedSlot();
    void tryToReconnectSlot();
    void socketEncryptedSlot();
    void peerVerifyErrorSlot();
    void rateControlSlot();

};

#endif // NNTPCLIENT_H
