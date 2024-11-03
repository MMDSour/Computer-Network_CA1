// client.h
#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QRandomGenerator>
#include "AudioInput.h"
#include "AudioOutput.h"
#include <QMutex>
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


    AudioInput* audioInput;
    AudioOutput* audioOutput;
    QString peerId;
    QString id;
    bool isOfferer;
    WebRTC* webrtc;
    QMutex mutex;

private Q_SLOTS:
    void onOfferIsReady(const QString &peerID, const QString& description);
    void onAnswerIsReady(const QString &peerID, const QString& description);
    void onOpenedDataChannel(const QString &peerId);
    void onDataReady(QByteArray &data);
    void onIncommingPacket(const QString &peerId, const QByteArray &data, qint64 len);
};

#endif
