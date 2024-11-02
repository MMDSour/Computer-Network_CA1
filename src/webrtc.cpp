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
    // Initialize WebRTC using libdatachannel library
    // Create an instance of rtc::Configuration to Set up ICE configuration
    // Add a STUN server to help peers find their public IP addresses
    // Add a TURN server for relaying media if a direct connection can't be established
    // Set up the audio stream configuration


    m_localId = id;
    m_isOfferer = isOfferer;

    m_config = rtc::Configuration();

    // Optionally, add a TURN server (replace with actual TURN server credentials if you have one)
    // m_config.iceServers.push_back(rtc::IceServer{"turn:turn.example.com", "username", "password"});
    m_config.iceServers.push_back(rtc::IceServer{"stun:stun.l.google.com:19302"});

    m_audio = rtc::Description::Audio("audio");
    m_audio.setBitrate(m_bitRate); // Set the bit rate

    Q_EMIT isOffererChanged();

    qDebug() << "WebRTC initialized with ID:" << id
             << ", as" << (isOfferer ? "offerer" : "answerer")
             << ", with payload type:" << m_payloadType
             << ", and bit rate:" << m_bitRate;
}

void WebRTC::addPeer(const QString &peerId)
{
    // Create and add a new peer connection
    auto newPeer = std::make_shared<rtc::PeerConnection>(m_config);
    m_peerConnections.insert(peerId, newPeer);
    // newPeer->setLocalDescription();

    // Set up a callback for when the local description is generated

    newPeer->onLocalDescription([this, peerId](const rtc::Description &description) {
        // The local description should be emitted using the appropriate signals based on the peer's role (offerer or answerer)

        // Emit the appropriate signal based on whether this is an offer or an answer
        m_localDescription = descriptionToJson(description);
        // Q_EMIT localDescriptionGenerated(peerId, m_localDescription);

        // if (m_isOfferer)
        //     Q_EMIT offerIsReady(peerId, m_localDescription);
        // else
        //     Q_EMIT answerIsReady(peerId, m_localDescription);
    });

    // Set up a callback for handling local ICE candidates
    newPeer->onLocalCandidate([this, peerId](rtc::Candidate candidate) {
        // Emit the local candidates using the localCandidateGenerated signal
        QString candidateStr = QString::fromStdString(candidate.candidate());
        QString sdpMid = QString::fromStdString(candidate.mid());
        Q_EMIT localCandidateGenerated(peerId, candidateStr, sdpMid);
    });

    // Set up a callback for when the state of the peer connection changes
    newPeer->onStateChange([this, peerId](rtc::PeerConnection::State state) {
        // Handle different states like New, Connecting, Connected, Disconnected, etc.
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
            // Q_EMIT closedDataChannel(peerId);
            break;
        }
    });


    // Set up a callback for monitoring the gathering state
    newPeer->onGatheringStateChange([this, peerId](rtc::PeerConnection::GatheringState state) {
        // When the gathering is complete, emit the gatheringComplited signal
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            m_gatheringComplited = true;
            Q_EMIT gatheringComplited(peerId);
            qDebug() << "Gathering completed for peer" << peerId;
        }
    });


    // Set up a callback for handling incoming tracks
    newPeer->onTrack([this, peerId] (std::shared_ptr<rtc::Track> track) {
        // handle the incoming media stream, emitting the incommingPacket signal if a stream is received

        // Store track reference
        m_peerTracks[peerId] = track;

        // If audio track received, monitor incoming packets
        track->onMessage([this, peerId](rtc::message_variant data) {
            QByteArray packet = readVariant(data);
            Q_EMIT incommingPacket(peerId, packet, packet.size());
        });

        qDebug() << "Incoming track received for peer" << peerId;
    });

    // Add an audio track to the peer connection
    addAudioTrack(peerId, "m=audio 49170 UDP/TLS/RTP/SAVPF 111");
}

// Set the local description for the peer's connection
void WebRTC::generateOfferSDP(const QString &peerId)
{
    // Retrieve the peer connection for the specified peer ID
    auto it = m_peerConnections.find(peerId);
    if (it == m_peerConnections.end()) {
        qWarning() << "Peer connection not found for peerID:" << peerId;
        return;
    }

    auto peerConnection = it.value();

    // // Set up a callback for when the local description (SDP offer) is generated
    // peerConnection->onLocalDescription([this, peerId](const rtc::Description &description) {
    //     // Convert the SDP offer to JSON format
    //     m_localDescription = descriptionToJson(description);

    //     // Emit the localDescriptionGenerated signal to notify listeners
    //     Q_EMIT localDescriptionGenerated(peerId, m_localDescription);

    //     // Emit the offerIsReady signal if this instance is acting as the offerer
    //     // if (m_isOfferer) {
    //     //     Q_EMIT offerIsReady(peerId, m_localDescription);
    //     // }
    // });

    // Create and set an SDP offer as the local description
    peerConnection->setLocalDescription(rtc::Description::Type::Offer);

    qDebug() << "Generated SDP offer for peer ID:" << peerId;
}

// Generate an answer SDP for the peer
void WebRTC::generateAnswerSDP(const QString &peerId)
{
    // Retrieve the peer connection for the given peerId
    auto it = m_peerConnections.find(peerId);
    if (it == m_peerConnections.end()) {
        qWarning() << "Peer connection not found for peerID:" << peerId;
        return;
    }

    auto peerConnection = it.value();

    // // Set a callback for when the local description (SDP answer) is generated
    // peerConnection->onLocalDescription([this, peerId](const rtc::Description &description) {
    //     // Convert the SDP answer to JSON format
    //     m_localDescription = descriptionToJson(description);

    //     // Emit the localDescriptionGenerated signal
    //     Q_EMIT localDescriptionGenerated(peerId, m_localDescription);

    //     // Emit the answerIsReady signal since this instance is generating an answer
    //     Q_EMIT answerIsReady(peerId, m_localDescription);
    // });

    // Create and set an SDP answer as the local description
    peerConnection->localDescription()->generateSdp();

    qDebug() << "Generated SDP answer for peer ID:" << peerId;
}


// Add an audio track to the peer connection
void WebRTC::addAudioTrack(const QString &peerId, const QString &trackName)
{
    if (!m_peerConnections.contains(peerId)) {
        qWarning() << "Peer connection for" << peerId << "does not exist.";
        return;
    }

    auto peerConnection = m_peerConnections[peerId];

    // Create an audio track description and add it to the peer connection
    auto audioTrack = peerConnection->addTrack(rtc::Description::Media(trackName.toStdString(), "audio", rtc::Description::Direction::SendOnly));

    // Store the track to allow later management or removal if needed
    m_peerTracks[peerId] = audioTrack;

    // Handle track message events
    audioTrack->onMessage([this, peerId](rtc::message_variant data) {
        QByteArray packet = readVariant(data);  // Convert the rtc::message_variant data to QByteArray
        Q_EMIT incommingPacket(peerId, packet, packet.size());
        qDebug() << "Received audio message with timestamp:";
    });

    // Handle track frame events
    audioTrack->onFrame([this](rtc::binary frame, rtc::FrameInfo info) {
        // Process frame data as needed
        qDebug() << "Received audio frame with timestamp:" << info.timestamp;
    });

    qDebug() << "Added audio track for peer:" << peerId << "with track name:" << trackName;
}



// Sends audio track data to the peer
void WebRTC::sendTrack(const QString &peerId, const QByteArray &buffer)
{
    // Check if the peer connection and audio track exist
    auto peerConnectionIt = m_peerConnections.find(peerId);
    auto peerTrackIt = m_peerTracks.find(peerId);
    if (peerConnectionIt == m_peerConnections.end() || peerTrackIt == m_peerTracks.end()) {
        qWarning() << "Peer connection or track not found for peerID:" << peerId;
        return;
    }

    // Create the RTP header and initialize an RtpHeader struct
    RtpHeader rtpHeader;
    rtpHeader.first = 0x80;  // RTP version 2
    rtpHeader.marker = 0;    // No marker, could set for specific packets like end of frame
    rtpHeader.payloadType = m_payloadType;
    rtpHeader.sequenceNumber = qToBigEndian(m_sequenceNumber++);
    rtpHeader.timestamp = qToBigEndian(getCurrentTimestamp());
    rtpHeader.ssrc = qToBigEndian(m_ssrc);

    // Create the RTP packet by appending the RTP header and the payload buffer
    QByteArray rtpPacket;
    rtpPacket.append(reinterpret_cast<const char*>(&rtpHeader), sizeof(RtpHeader));  // Append the RTP header
    rtpPacket.append(buffer);  // Append the payload (audio data)

    // Send the packet, catch and handle any errors that occur during sending

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

// Set the remote SDP description for the peer that contains metadata about the media being transmitted
void WebRTC::setRemoteDescription(const QString &peerID, const QString &sdp)
{
    // Check if the peer connection exists
    auto it = m_peerConnections.find(peerID);
    if (it == m_peerConnections.end()) {
        qWarning() << "Peer connection not found for peerID:" << peerID;
        return;
    }

    // Convert the QString SDP to std::string for compatibility with libdatachannel
    std::string sdpStr = sdp.toStdString();

    // Parse the SDP string to create a rtc::Description
    rtc::Description remoteDescription(sdpStr, m_isOfferer ? rtc::Description::Type::Answer : rtc::Description::Type::Offer);

    // Set the remote description on the peer connection
    it.value()->setRemoteDescription(remoteDescription);

    qDebug() << "Set remote description for peerID:" << peerID;
}

// Add remote ICE candidates to the peer connection
void WebRTC::setRemoteCandidate(const QString &peerID, const QString &candidate, const QString &sdpMid)
{
    // Check if the peer connection exists
    auto it = m_peerConnections.find(peerID);
    if (it == m_peerConnections.end()) {
        qWarning() << "Peer connection not found for peerID:" << peerID;
        return;
    }

    // Convert QString parameters to std::string for libdatachannel compatibility
    std::string candidateStr = candidate.toStdString();
    std::string sdpMidStr = sdpMid.toStdString();

    // Create an rtc::Candidate object
    rtc::Candidate rtcCandidate(candidateStr, sdpMidStr);

    // Add the candidate to the peer connection
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

// Utility function to read the rtc::message_variant into a QByteArray
QByteArray WebRTC::readVariant(const rtc::message_variant &data)
{
    // Check the type of the variant and convert accordingly
    if (std::holds_alternative<rtc::binary>(data)) {
        // Handle binary data
        const rtc::binary &binaryData = std::get<rtc::binary>(data);
        return QByteArray(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
    } else if (std::holds_alternative<std::string>(data)) {
        // Handle UTF-8 string data
        const std::string &textData = std::get<std::string>(data);
        return QByteArray::fromStdString(textData);
    } else {
        // Unsupported type, return an empty QByteArray
        return QByteArray();
    }
}

// Utility function to convert rtc::Description to JSON format
QString WebRTC::descriptionToJson(const rtc::Description &description)
{
    QJsonObject json;

    // Add the type of the SDP description (either "offer" or "answer")
    json["type"] = QString::fromStdString(description.typeString());

    // Add the SDP string itself
    json["sdp"] = QString::fromStdString(description.generateSdp());

    // Convert the QJsonObject to a QJsonDocument, then to a QString
    QJsonDocument doc(json);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

// Retrieves the current bit rate
int WebRTC::bitRate() const
{
    return m_bitRate;
}

// Set a new bit rate and emit the bitRateChanged signal
void WebRTC::setBitRate(int newBitRate)
{
    // TODO check if it actually changes
    m_bitRate = newBitRate;
    Q_EMIT bitRateChanged();
}

// Reset the bit rate to its default value
void WebRTC::resetBitRate()
{
    // TODO check if it actually changes
    m_bitRate = 48000;
    Q_EMIT bitRateChanged();
}

// Sets a new payload type and emit the payloadTypeChanged signal
void WebRTC::setPayloadType(int newPayloadType)
{
    // TODO check if it actually changes
    m_payloadType = newPayloadType;
    Q_EMIT payloadTypeChanged();
}

// Resets the payload type to its default value
void WebRTC::resetPayloadType()
{
    // TODO check if it actually changes
    m_payloadType = 111;
    Q_EMIT payloadTypeChanged();
}

// Retrieve the current SSRC value
rtc::SSRC WebRTC::ssrc() const
{
    return m_ssrc;
}

// Set a new SSRC and emit the ssrcChanged signal
void WebRTC::setSsrc(rtc::SSRC newSsrc)
{
    // TODO check if it actually changes
    m_ssrc = newSsrc;
    Q_EMIT ssrcChanged();
}

// Reset the SSRC to its default value
void WebRTC::resetSsrc()
{
    // TODO check if it actually changes
    m_ssrc = 2;
    Q_EMIT ssrcChanged();
}

// Retrieve the current payload type
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
    // TODO check if it actually changes
    m_isOfferer = newIsOfferer;
    Q_EMIT isOffererChanged();
}

void WebRTC::resetIsOfferer()
{
    // TODO check if it actually changes
    m_isOfferer = false;
    Q_EMIT isOffererChanged();
}


