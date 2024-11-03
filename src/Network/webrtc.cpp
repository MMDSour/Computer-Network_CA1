#include "webrtc.h"
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static_assert(true);

#pragma pack(push, 1)
struct RtpHeader {
    uint8_t first;
    uint8_t marker:1;
    uint8_t payloadType:7;
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
};
#pragma pack(pop)


WebRTC::WebRTC(QObject *parent)
    : QObject{parent},
    m_audio("Audio")
{
    connect(this, &WebRTC::gatheringComplited, [this] (const QString &peerID) {

        m_localDescription = descriptionToJson(m_peerConnections[peerID]->localDescription().value());
        Q_EMIT localDescriptionGenerated(peerID, m_localDescription);

        if (m_isOfferer)
            Q_EMIT this->offerIsReady(peerID, m_localDescription);
        else
            Q_EMIT this->answerIsReady(peerID, m_localDescription);
    });
}

WebRTC::~WebRTC()
{}


/**
 * ====================================================
 * ================= public methods ===================
 * ====================================================
 **/

void WebRTC::init(const QString &id, bool isOfferer)
{
    m_localId = id;
    m_isOfferer = isOfferer;

    m_config = rtc::Configuration();
    m_config.iceServers.push_back(rtc::IceServer{"stun:stun.l.google.com:19302"});

    m_audio = rtc::Description::Audio("audio");
    m_audio.setBitrate(m_bitRate);

    Q_EMIT isOffererChanged();

    qDebug() << "WebRTC initialized with ID:" << id
             << ", as" << (isOfferer ? "offerer" : "answerer")
             << ", with payload type:" << m_payloadType
             << ", and bit rate:" << m_bitRate;
}

void WebRTC::addPeer(const QString &peerId)
{
    auto newPeer = std::make_shared<rtc::PeerConnection>(m_config);
    m_peerConnections.insert(peerId, newPeer);

    newPeer->onLocalDescription([this, peerId](const rtc::Description &description) {
        m_localDescription = descriptionToJson(description);
    });

    newPeer->onLocalCandidate([this, peerId](rtc::Candidate candidate) {
        QString candidateStr = QString::fromStdString(candidate.candidate());
        QString sdpMid = QString::fromStdString(candidate.mid());
        Q_EMIT localCandidateGenerated(peerId, candidateStr, sdpMid);
    });

    newPeer->onStateChange([this, peerId](rtc::PeerConnection::State state) {
        switch (state) {
        case rtc::PeerConnection::State::New:
            qDebug() << "Peer" << peerId << "connection state: New";
            break;
        case rtc::PeerConnection::State::Connecting:
            qDebug() << "Peer" << peerId << "connection state: Connecting";
            break;
        case rtc::PeerConnection::State::Connected:
            qDebug() << "Peer" << peerId << "connection state: Connected";
            Q_EMIT openedDataChannel(peerId);
            break;
        case rtc::PeerConnection::State::Disconnected:
            qDebug() << "Peer" << peerId << "connection state: Disconnected";
            break;
        case rtc::PeerConnection::State::Failed:
            qDebug() << "Peer" << peerId << "connection state: Failed";
            break;
        case rtc::PeerConnection::State::Closed:
            qDebug() << "Peer" << peerId << "connection state: Closed";
            break;
        }
    });

    newPeer->onGatheringStateChange([this, peerId](rtc::PeerConnection::GatheringState state) {
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            m_gatheringComplited = true;
            Q_EMIT gatheringComplited(peerId);
            qDebug() << "Gathering completed for peer" << peerId;
        }
    });

    newPeer->onTrack([this, peerId] (std::shared_ptr<rtc::Track> track) {
        m_peerTracks[peerId] = track;
        track->onMessage([this, peerId](rtc::message_variant data) {
            QByteArray packet = readVariant(data);
            Q_EMIT incommingPacket(peerId, packet, packet.size());
        });
        qDebug() << "Incoming track received for peer" << peerId;
    });

    addAudioTrack(peerId, "m=audio 49170 UDP/TLS/RTP/SAVPF 111");
}

void WebRTC::generateOfferSDP(const QString &peerId)
{
    auto it = m_peerConnections.find(peerId);
    if (it == m_peerConnections.end()) {
        qWarning() << "Peer connection not found for peerID:" << peerId;
        return;
    }

    auto peerConnection = it.value();
    peerConnection->setLocalDescription(rtc::Description::Type::Offer);

    qDebug() << "Generated SDP offer for peer ID:" << peerId;
}

void WebRTC::generateAnswerSDP(const QString &peerId)
{
    auto it = m_peerConnections.find(peerId);
    if (it == m_peerConnections.end()) {
        qWarning() << "Peer connection not found for peerID:" << peerId;
        return;
    }

    auto peerConnection = it.value();

    peerConnection->localDescription()->generateSdp();

    qDebug() << "Generated SDP answer for peer ID:" << peerId;
}

void WebRTC::addAudioTrack(const QString &peerId, const QString &trackName)
{
    if (!m_peerConnections.contains(peerId)) {
        qWarning() << "Peer connection for" << peerId << "does not exist.";
        return;
    }

    auto peerConnection = m_peerConnections[peerId];

    auto audioTrack = peerConnection->addTrack(rtc::Description::Media(trackName.toStdString(), "audio", rtc::Description::Direction::SendRecv));

    m_peerTracks[peerId] = audioTrack;

    audioTrack->onMessage([this, peerId](rtc::message_variant data) {
        QByteArray packet = readVariant(data);
        Q_EMIT incommingPacket(peerId, packet, packet.size());
        qDebug() << "Received audio message with timestamp:";
    });

    audioTrack->onFrame([this](rtc::binary frame, rtc::FrameInfo info) {
        qDebug() << "Received audio frame with timestamp:" << info.timestamp;
    });

    qDebug() << "Added audio track for peer:" << peerId << "with track name:" << trackName;
}

void WebRTC::sendTrack(const QString &peerId, const QByteArray &buffer)
{
    auto peerConnectionIt = m_peerConnections.find(peerId);
    auto peerTrackIt = m_peerTracks.find(peerId);
    if (peerConnectionIt == m_peerConnections.end() || peerTrackIt == m_peerTracks.end()) {
        qWarning() << "Peer connection or track not found for peerID:" << peerId;
        return;
    }

    RtpHeader rtpHeader;
    rtpHeader.first = 0x80;
    rtpHeader.marker = 0;
    rtpHeader.payloadType = m_payloadType;
    rtpHeader.sequenceNumber = qToBigEndian(m_sequenceNumber++);
    rtpHeader.timestamp = qToBigEndian(getCurrentTimestamp());
    rtpHeader.ssrc = qToBigEndian(m_ssrc);

    QByteArray rtpPacket;
    rtpPacket.append(reinterpret_cast<const char*>(&rtpHeader), sizeof(RtpHeader));
    rtpPacket.append(buffer);

    std::vector<std::byte> packetData(rtpPacket.size());
    std::transform(rtpPacket.begin(), rtpPacket.end(), packetData.begin(), [](char c) {
        return static_cast<std::byte>(c);
    });

    try {
        peerTrackIt.value()->send(packetData);
        qDebug() << "Sent RTP packet to peer" << peerId << "with size:" << rtpPacket.size();
    } catch (const std::exception &e) {
        qWarning() << "Error sending RTP packet to peer" << peerId << ":" << e.what();
    }
}


/**
 * ====================================================
 * ================= public slots =====================
 * ====================================================
 */

void WebRTC::setRemoteDescription(const QString &peerID, const QString &sdp)
{
    auto it = m_peerConnections.find(peerID);
    if (it == m_peerConnections.end()) {
        qWarning() << "Peer connection not found for peerID:" << peerID;
        return;
    }
    std::string sdpStr = sdp.toStdString();
    rtc::Description remoteDescription(sdpStr, m_isOfferer ? rtc::Description::Type::Answer : rtc::Description::Type::Offer);
    it.value()->setRemoteDescription(remoteDescription);

    qDebug() << "Set remote description for peerID:" << peerID;
}

void WebRTC::setRemoteCandidate(const QString &peerID, const QString &candidate, const QString &sdpMid)
{
    auto it = m_peerConnections.find(peerID);
    if (it == m_peerConnections.end()) {
        qWarning() << "Peer connection not found for peerID:" << peerID;
        return;
    }

    std::string candidateStr = candidate.toStdString();
    std::string sdpMidStr = sdpMid.toStdString();

    rtc::Candidate rtcCandidate(candidateStr, sdpMidStr);

    it.value()->addRemoteCandidate(rtcCandidate);

    qDebug() << "Added remote candidate for peerID:" << peerID
             << " with candidate:" << candidate
             << " and sdpMid:" << sdpMid;
}


/*
 * ====================================================
 * ================= private methods ==================
 * ====================================================
 */

QByteArray WebRTC::readVariant(const rtc::message_variant &data)
{
    QByteArray result;

    if (std::holds_alternative<std::string>(data)) {
        const auto &str = std::get<std::string>(data);
        result = QByteArray::fromStdString(str);
    }
    else if (std::holds_alternative<rtc::binary>(data)) {
        const rtc::binary &binaryData = std::get<rtc::binary>(data);
        result = QByteArray(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
    } else {
        qWarning() << "Unsupported data type in rtc::message_variant";
        return result;
    }

    const int rtpHeaderSize = sizeof(RtpHeader);

    if (result.size() >= rtpHeaderSize) {
        result = result.mid(rtpHeaderSize);
    } else {
        qWarning() << "Data size is smaller than RTP header size. Unable to remove header.";
    }

    return result;
}

QString WebRTC::descriptionToJson(const rtc::Description &description)
{
    QJsonObject json;

    json["type"] = QString::fromStdString(description.typeString());
    json["sdp"] = QString::fromStdString(description.generateSdp());

    QJsonDocument doc(json);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

int WebRTC::bitRate() const
{
    return m_bitRate;
}

void WebRTC::setBitRate(int newBitRate)
{
    m_bitRate = newBitRate;
    Q_EMIT bitRateChanged();
}

void WebRTC::resetBitRate()
{
    m_bitRate = 48000;
    Q_EMIT bitRateChanged();
}

void WebRTC::setPayloadType(int newPayloadType)
{
    m_payloadType = newPayloadType;
    Q_EMIT payloadTypeChanged();
}

void WebRTC::resetPayloadType()
{
    m_payloadType = 111;
    Q_EMIT payloadTypeChanged();
}

rtc::SSRC WebRTC::ssrc() const
{
    return m_ssrc;
}

void WebRTC::setSsrc(rtc::SSRC newSsrc)
{
    m_ssrc = newSsrc;
    Q_EMIT ssrcChanged();
}

void WebRTC::resetSsrc()
{
    m_ssrc = 2;
    Q_EMIT ssrcChanged();
}

int WebRTC::payloadType() const
{
    return m_payloadType;
}


/**
 * ====================================================
 * ================= getters setters ==================
 * ====================================================
 */

bool WebRTC::isOfferer() const
{
    return m_isOfferer;
}

void WebRTC::setIsOfferer(bool newIsOfferer)
{
    m_isOfferer = newIsOfferer;
    Q_EMIT isOffererChanged();
}

void WebRTC::resetIsOfferer()
{
    m_isOfferer = false;
    Q_EMIT isOffererChanged();
}


