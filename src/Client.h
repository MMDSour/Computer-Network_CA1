// client.h
#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QRandomGenerator>
#include "SocketIO/sio_client.h"
#include "webrtc.h"
using namespace std;

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr, string serverUrl = "https://localhost:8080", QString id = "", bool isOfferer = false);
    void connectToServer(const std::string& url);
    void sendRegisterRequest();
    void sendSdp(const string &clientId);
    void startCall(QString peerId);

private:
    sio::client socket;

    void onConnected();
    void onMessageReceived(const std::string& message);


    QString id;
    bool isOfferer;
    WebRTC* webrtc;

private Q_SLOTS:
    void onOfferIsReady(const QString &peerID, const QString& description);
    void onAnswerIsReady(const QString &peerID, const QString& description);
    void onOpenedDataChannel(const QString &peerId);
};

#endif // CLIENT_H
