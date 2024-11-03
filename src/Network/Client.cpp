#include "client.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

Client::Client(QObject *parent, string serverUrl_, QString id_, bool isOfferer_, QString peerId) : QObject(parent)
{
    webrtc = new WebRTC (this);
    audioInput = new AudioInput(this);
    audioOutput = new AudioOutput();
    id = id_;
    isOfferer = isOfferer_;
    peerId_ = peerId;

    connect(webrtc, &WebRTC::offerIsReady, this, &Client::onOfferIsReady);
    connect(webrtc, &WebRTC::answerIsReady, this, &Client::onAnswerIsReady);
    connect(webrtc, &WebRTC::openedDataChannel, this, &Client::onOpenedDataChannel);
    connect(audioInput, &AudioInput::dataReady, this, &Client::onDataReady);
    connect(webrtc, &WebRTC::incommingPacket, this, &Client::onIncommingPacket);

    socket.set_open_listener([this]() { onConnected(); });
    socket.set_close_listener([](sio::client::close_reason const& reason) {
        qDebug() << "Disconnected from server";
    });

    socket.set_fail_listener([]() {
        qDebug() << "Connection failed";
    });

    socket.socket()->on("message", sio::socket::event_listener_aux(
                                        [this](const std::string& name, const std::shared_ptr<sio::message>& data, bool hasAck, sio::message::list &ack_resp) {
                                            if (data) {
                                                onMessageReceived(data->get_string());
                                            }
                                        }
                                        ));

    sendRegisterRequest();
    connectToServer(serverUrl_);    
}

void Client::connectToServer(const std::string& url)
{
    socket.connect(url);
    webrtc->init(id, isOfferer);
}

void Client::sendRegisterRequest()
{
    QString registerMessage = QString("{ \"type\": \"register\", \"id\": \"%1\" }").arg(id);

    socket.socket()->emit("message", registerMessage.toStdString());
    qDebug() << "Register message sent for ID:" << id;
}

void Client::onConnected()
{
    qDebug() << "Connected to signaling server";
}

void Client::startCall (QString peerId)
{
    webrtc->addPeer(peerId);
    webrtc->generateOfferSDP(peerId);
}

void Client::onOfferIsReady (const QString &peerID, const QString& description)
{
    qDebug() << "Offer is ready for peer:" << peerID << "\nSDP Description:\n" << description;

    QString escapedDescription = description;
    escapedDescription.replace("\"", "\\\"");

    QString offerMessage = QString("{ \"type\": \"offer\", \"MyId\": \"%1\", \"answererId\": \"%2\", \"sdp\": \"%3\" }")
                               .arg(id)
                               .arg(peerID)
                               .arg(escapedDescription);


    socket.socket()->emit("message", offerMessage.toStdString());
    qDebug() << "SDP offer message sent for peer ID:" << peerID;
}

void Client::onAnswerIsReady(const QString &peerID, const QString& description)
{
    qDebug() << "Answer is ready for peer:" << peerID << "\nSDP Description:\n" << description;

    QString escapedDescription = description;
    escapedDescription.replace("\"", "\\\"");

    QString answerMessage = QString("{ \"type\": \"answer\", \"MyId\": \"%1\", \"offererId\": \"%2\", \"sdp\": \"%3\" }")
                                .arg(id)
                                .arg(peerID)
                                .arg(escapedDescription);

    socket.socket()->emit("message", answerMessage.toStdString());
    qDebug() << "SDP answer message sent for peer ID:" << peerID;
}


void Client::onMessageReceived(const std::string& message)
{
    qDebug() << "Message received from server:" << QString::fromStdString(message);

    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(message).toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid message format";
        return;
    }

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    QString peerID = obj["MyId"].toString();
    QString sdp = obj["sdp"].toString();

    if (type == "offer") {

        webrtc->addPeer(peerID);
        webrtc->setRemoteDescription(peerID, sdp);
        webrtc->generateAnswerSDP(peerID);
    }
    else if (type == "answer"){
        webrtc->setRemoteDescription(peerID, sdp);
        qDebug() << "Message received on offerer:" << QString::fromStdString(message);
    }
}

void Client::onOpenedDataChannel(const QString &peerId)
{
    audioInput->start();
}

void Client::onDataReady(QByteArray &data)
{
    webrtc->sendTrack(peerId_, data);
}

void Client::onIncommingPacket(const QString &peerId, const QByteArray &data, qint64 len)
{
    qDebug() << "packet for:" << id <<"from peer:"<< peerId;
    audioOutput->addData(data);
}
