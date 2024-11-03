# Distributed Live Voice Call using WebRTC

## Table of Contents

1. [Project Overview](#project-overview)
2. [Objective](#objective)
3. [Technologies Used](#technologies-used)
4. [Project Structure](#project-structure)
5. [Prerequisites](#prerequisites)
6. [Installation](#installation)
7. [Code Explanation](#code-explanation)
8. [WebRTC and Coturn Explanation](#webrtc-and-coturn-explanation)
9. [Usage](#usage)
10. [Running The App](#running-the-app)

---

## Project Overview
This project implements a peer-to-peer distributed voice call application using WebRTC technology, developed as part of a computer networks assignment. The system allows real-time audio communication between devices without the need for a central server.

## Objective
The main goal is to establish a voice communication channel between two devices over the network, using WebRTC for peer-to-peer connectivity and, optionally, Coturn as a TURN server for NAT traversal. This ensures that users can connect seamlessly even when they are behind firewalls or routers that typically block direct connections.


## Technologies Used
- **Programming Language**: C++
- **Framework**: Qt
- **Libraries**:
  - **WebRTC** (libdatachannel) for real-time peer-to-peer communication
  - **Opus codec** for audio encoding and decoding
  - **Coturn** (optional) as a TURN server for NAT traversal

## Project Structure
```plaintext
## Project Structure
```plaintext
📂 Project_Root
├── CN_CA_1.pdf                # Project Description
├── README.md                   # Project README for Documentation
├── 📂 Signaling Server
│   ├── SignalingServer.js      # Signaling server for WebRTC
│   ├── cert.pem                # SSL certificate for secure connections
│   ├── key.pem                 # Private key for SSL certificate
│   ├── package-lock.json       # Dependency lock file for Node.js
│   └── package.json            # Package configuration for Node.js dependencies
└── 📂 src
    ├── 📂 App
    │   ├── app.cpp             # Main application logic
    │   └── app.h               # Header file for application class
    ├── 📂 Audio
    │   ├── AudioInput.cpp      # Audio input and encoding
    │   ├── AudioInput.h        # Header file for AudioInput class
    │   ├── AudioOutput.cpp     # Audio output and decoding
    │   └── AudioOutput.h       # Header file for AudioOutput class
    ├── 📂 Network
    │   ├── Client.cpp          # Client-side logic for network communication
    │   ├── Client.h            # Header file for Client class
    │   ├── webrtc.cpp          # WebRTC related functionality
    │   └── webrtc.h            # Header file for WebRTC functions
    ├── 📂 SocketIO
    │   ├── 📂 internal
    │   │   ├── sio_client_impl.cpp    # Internal implementation of Socket.IO client
    │   │   ├── sio_client_impl.h      # Header file for Socket.IO client implementation
    │   │   ├── sio_packet.cpp       # Socket.IO packet handling
    │   │   └── sio_packet.h         # Header file for Socket.IO packet
    │   ├── sio_client.cpp         # Socket.IO client logic
    │   ├── sio_client.h           # Header file for Socket.IO client class
    │   ├── sio_message.h          # Header file for Socket.IO message handling
    │   ├── sio_socket.cpp         # Socket.IO socket handling
    │   └── sio_socket.h           # Header file for Socket.IO socket class
    ├── 📂 UI
    │   └── Main.qml               # QML file for the App interface
    ├── 📂 build                   # Build directory for compiled binaries
    ├── main.cpp                   # Main entry point for the application
    ├── src.pro                    # Qt project file
    └── src.pro.user               # User-specific Qt project file
```

## Prerequisites
1. **Qt framework**: [Installation Guide](https://doc.qt.io/qt-6/qtonline-installation.html)
2. **WebRTC libdatachannel**: [GitHub Repo](https://github.com/paullouisageneau/libdatachannel)
3. **Opus codec**: [GitHub Repo](https://github.com/xiph/opus)
4. **Coturn (optional)**: [GitHub Repo](https://github.com/coturn/coturn)
5. **Node.js**: Required for running the signaling server. [Node.js Website](https://nodejs.org/)
6. **Socket.IO**: Used for real-time signaling between clients:
   - **Node.js dependencies**: Located in the `Signaling Server` folder, install using `npm install`.
   - **C++ Socket.IO Client Library**: Clone and integrate with:
     ```bash
     git clone https://github.com/socketio/socket.io-client-cpp.git 
     cd socket.io-client-cpp 
     git submodule update --init --recursive
     ```
7. **CMake**: Required for building the C++ components. [CMake Download](https://cmake.org/download/)
8. **OpenSSL**: Required for secure connections. [OpenSSL Website](https://www.openssl.org/source/)
## Installation
1. **Clone this repository**:
   ```bash
   git clone https://github.com/MMDSour/CN_CA1.git
   cd CN_CA1
   ```
2. **Install dependencies** as listed in the Prerequisites section.
3. **Build the project** using QT Creator:

   - Create Project in QT Creator
   - Name the project src in the directory
   - Change dependencies paths in src.pro
   - Build & Run Using QT Creator

4. **(Optional) Configure TURN server**:
   - If using Coturn, configure it by modifying `turnserver.conf` in the `config` folder.

## Code Explanation

### WebRTC
This class handles WebRTC functionality, using audio tracks and RTP (Real-time Transport Protocol) packets. The `WebRTC` class offers methods for initializing connections, managing peers, generating SDP (Session Description Protocol) offers/answers, and sending/receiving audio data. Let's break down the core components.

#### 1. `RtpHeader` Structure
The `RtpHeader` structure is defined with `#pragma pack(push, 1)` to ensure no padding is added, preserving the exact layout required for RTP packets.

```cpp
struct RtpHeader {
    uint8_t first;
    uint8_t marker:1;
    uint8_t payloadType:7;
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint32_t ssrc;
};
```

- **`marker`** and **`payloadType`** represent RTP header fields, with `marker` being 1 bit and `payloadType` 7 bits.
- **`sequenceNumber`, `timestamp`, and `ssrc`** are RTP fields used for identifying and timing packets.

#### 2. Constructor and Destructor
The `WebRTC` constructor initializes the class and connects the `gatheringCompleted` signal to a lambda function, which triggers SDP generation for the offerer/answerer role.

```cpp
WebRTC::WebRTC(QObject *parent)
    : QObject{parent}, m_audio("Audio")
{
    connect(this, &WebRTC::gatheringComplited, [this] (const QString &peerID) {
        m_localDescription = descriptionToJson(m_peerConnections[peerID]->localDescription().value());
        // Emitting signals based on role
    });
}

WebRTC::~WebRTC()
{}
```

#### 3. Public Methods

##### `init`
Initializes WebRTC settings with an ID and role (offerer/answerer), sets up STUN servers, and emits signals.

```cpp
void WebRTC::init(const QString &id, bool isOfferer)
{
    m_localId = id;
    m_isOfferer = isOfferer;
    m_config.iceServers.push_back(rtc::IceServer{"stun:stun.l.google.com:19302"});
    m_audio.setBitrate(m_bitRate);
    Q_EMIT isOffererChanged();
}
```

##### `addPeer`
Adds a new peer connection, handles connection states, ICE candidate generation, and audio tracks.

```cpp
void WebRTC::addPeer(const QString &peerId)
{
    auto newPeer = std::make_shared<rtc::PeerConnection>(m_config);
    // Handling connection states, candidates, tracks
    addAudioTrack(peerId, "m=audio 49170 UDP/TLS/RTP/SAVPF 111");
}
```

##### `generateOfferSDP` and `generateAnswerSDP`
Generates SDP offers and answers, used to establish a WebRTC connection.

#### 4. Track Management

##### `addAudioTrack`
Adds an audio track for the peer and sets up message and frame handlers.

```cpp
void WebRTC::addAudioTrack(const QString &peerId, const QString &trackName)
{
    if (!m_peerConnections.contains(peerId)) { return; }
    auto audioTrack = peerConnection->addTrack(rtc::Description::Media(trackName.toStdString(), "audio"));
    // Handlers for messages and frames
}
```

##### `sendTrack`
Encodes audio data into RTP packets and sends it to the specified peer.

```cpp
void WebRTC::sendTrack(const QString &peerId, const QByteArray &buffer)
{
    RtpHeader rtpHeader;
    // Set RTP header fields, transform to packet, and send
}
```

#### 5. Slots (Public Methods)

##### `setRemoteDescription` and `setRemoteCandidate`
Sets the remote SDP and ICE candidates for peer connection.

```cpp
void WebRTC::setRemoteDescription(const QString &peerID, const QString &sdp)
{
    rtc::Description remoteDescription(sdp.toStdString(), m_isOfferer ? rtc::Description::Type::Answer : rtc::Description::Type::Offer);
    it.value()->setRemoteDescription(remoteDescription);
}
```

#### 6. Private Methods

##### `readVariant`
Reads data from a message variant, stripping the RTP header if present.

```cpp
QByteArray WebRTC::readVariant(const rtc::message_variant &data)
{
    QByteArray result;
    // Process data type and remove RTP header if present
}
```

### `descriptionToJson`
Converts SDP descriptions to JSON format.

```cpp
QString WebRTC::descriptionToJson(const rtc::Description &description)
{
    QJsonObject json;
    json["type"] = QString::fromStdString(description.typeString());
    json["sdp"] = QString::fromStdString(description.generateSdp());
    return QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));
}
```

#### 7. Getters and Setters

Provides access and modification to WebRTC configurations, such as bit rate, payload type, and SSRC.

```cpp
int WebRTC::bitRate() const { return m_bitRate; }
void WebRTC::setBitRate(int newBitRate) { m_bitRate = newBitRate; Q_EMIT bitRateChanged(); }
```

---

### AudioInput

The `AudioInput` class is responsible for capturing audio data from the microphone, encoding it with the Opus codec, and sending the encoded data to the `AudioOutput` class for playback.

1. **Inheritance and Initialization**:
   - `AudioInput` inherits from `QIODevice` to handle raw audio data as it arrives from the microphone.
   - In the constructor, an instance of `QAudioSource` is created to capture audio input with a specified sample rate (48,000 Hz) and channel count (mono).
   - The constructor initializes an Opus encoder (`opusEncoder`) to compress audio data into smaller packets suitable for transmission or storage.
   - If the encoder fails to initialize, an error message is logged.

2. **start() Method**:
   - The `start()` method activates audio capture by calling `start(this)` on `qAudioSource`. This routes incoming audio data to the `AudioInput` object.
   - The state of `qAudioSource` is checked to ensure it is active, and any issues are logged.

3. **writeData() Method**:
   - `writeData(const char *data, qint64 len)` is invoked whenever a new audio packet is ready.
   - The method receives raw audio data (`data`) of length `len`, encodes it using the `encodeData` function, and forwards the encoded data to `AudioOutput` for playback.
   - Encoding in `writeData` reduces the data size, which is essential for real-time transmission and storage.

4. **encodeData() Method**:
   - This method uses the Opus encoder to compress audio data.
   - The raw PCM data from the microphone is converted to an encoded format using `opus_encode`.
   - If encoding is successful, the compressed data is returned as a `QByteArray` for further processing. If an error occurs, an error message is logged.

---

### AudioOutput

The `AudioOutput` class handles the playback of encoded audio data. It receives encoded packets, decodes them, and plays them through the system’s audio output device.

1. **Inheritance and Initialization**:
   - `AudioOutput` inherits from `QObject` to manage audio playback and connect Qt signals and slots for real-time processing.
   - The class uses `QAudioSink` to set up audio output with a matching format (48,000 Hz, mono, 16-bit).
   - An instance of `QIODevice` (`audioDevice`) is created to write the decoded PCM audio data to the audio output device.
   - The constructor initializes an Opus decoder (`opusDecoder`) to decode incoming audio packets, logging any errors if the decoder fails to initialize.

2. **addData() Method**:
   - `addData(const QByteArray &data)` is called by `AudioInput` whenever a new encoded audio packet arrives.
   - This method locks the `audioQueue` using `QMutexLocker` to prevent race conditions and then enqueues the new data.
   - The `newPacket` signal is emitted to indicate that a new packet is ready for playback.

3. **play() Method**:
   - The `play()` slot is connected to the `newPacket` signal and is called whenever new data is added to `audioQueue`.
   - `play()` dequeues the next packet and decodes it using `opus_decode`, converting the encoded data back into PCM format.
   - The decoded PCM data is then written to `audioDevice` for immediate playback through the system’s speakers or headphones.
   - `QMutexLocker` ensures thread-safe access to `audioQueue` in both `addData` and `play`.

---

### Signaling Server
The **Signaling Server** enables the initial exchange of SDP offers/answers and ICE candidates. This server acts as a third party to help two peers establish a direct connection.

- **SSL Setup**: Reads SSL certificates to secure the server.
- **WebSocket Server**: Establishes a WebSocket server using Socket.io to handle messages.
- **Message Handling**:
  - **Register**: Adds the client to a `clients` map.
  - **Offer/Answer**: Forwards WebRTC offer/answer messages between clients.

---

### Client Application

#### File: `app.cpp`

This file initializes and starts the client connection based on its role (Offerer or Answerer).

```cpp
#include "app.h"

App::App(QObject *parent, bool isOfferer)
    : QObject{parent}
{
    if (isOfferer)
        m_verName = "Offerer";
    else
    {
        m_verName = "Answerer";
        client = new Client(nullptr, "https://localhost:8080", "peer2", false, "peer1");
    }
    this->isOfferer = isOfferer;
}

void App::start()
{
    if (isOfferer)
    {
        client = new Client(nullptr, "https://localhost:8080", "peer1", true, "peer2");
        client->startCall("peer2");
    }
}

void App::end()
{
    delete client;
    client = nullptr;
}
```

- **Role Assignment**: Configures the application as either the Offerer or Answerer.
- **Starting the Call**: The Offerer initiates the call, creating a WebRTC connection request to the Answerer.

---

#### File: `client.cpp`

This file manages the WebRTC connection, including setting up WebSocket communication, sending messages, and handling data transmission.

- **WebRTC and Audio Setup**: Initializes WebRTC and audio input/output.
- **Socket Events**: Listens for socket events (connect, message, disconnect) to handle signaling messages.
- **Signaling Workflow**:
  - **Offer**: Initiates a call by sending an offer to the server.
  - **Answer**: Responds to an offer from a peer.
  
## WebRTC and Coturn Explanation

![WebRTC](./docs/images/webrtc-comprehensive-workflow_large.png)

### WebRTC
**WebRTC** (Web Real-Time Communication) is a technology that enables browsers and devices to communicate directly in real-time, supporting peer-to-peer audio, video, and data sharing without needing intermediaries.

WebRTC uses several protocols and components:

- **SDP (Session Description Protocol)**: WebRTC relies on SDP for session negotiation. SDP defines the multimedia session parameters (codecs, encryption methods, and formats) for each peer, helping establish a compatible communication link.
  
- **ICE (Interactive Connectivity Establishment)**: ICE is a protocol that manages network path discovery to establish the best route between peers. ICE leverages **STUN** and **TURN** servers to help peers connect even if they are behind NATs or firewalls.

- **Signaling**: Although WebRTC doesn’t define its own signaling protocol, signaling is essential to start the connection process. It allows the exchange of necessary information like SDP offers and ICE candidates. Once the WebRTC connection is established, signaling is no longer required. 

**Advantages of WebRTC**:
- **Low Latency**: Enables real-time data and media exchange.
- **NAT Traversal**: Built-in support for traversing NATs and firewalls.

**Disadvantages of WebRTC**:
- **Complex Implementation**: WebRTC requires handling signaling, NAT traversal, and multimedia streaming.
- **Security Considerations**: Handling multimedia data securely is critical.

### STUN and TURN for NAT Traversal

- **STUN (Session Traversal Utilities for NAT)**: STUN servers help devices discover their public IP addresses and understand their NAT type. This process allows WebRTC to attempt a direct connection, which often works for less restrictive network environments.
  
- **TURN (Traversal Using Relays around NAT)**: TURN servers act as intermediaries, relaying data between peers when a direct connection is not possible. This process is more bandwidth-intensive and may introduce additional latency, but it ensures that peers can still connect even in restrictive network environments.

**NAT (Network Address Translation)**: NAT is commonly used in networks to share a single public IP address among multiple devices. WebRTC uses STUN and TURN to navigate NATs, ensuring that peer-to-peer connections can be established despite network restrictions.

### Coturn as a TURN Server

**Coturn** is an open-source TURN server implementation that provides a reliable fallback when direct connections between peers aren’t possible. It relays data for peers, which ensures connection stability across various network conditions.

- **When Coturn is Used**: Coturn is essential in restrictive environments where STUN alone cannot establish a direct connection. Adding a Coturn server improves the reliability of WebRTC connections.

**Advantages of Coturn**:
- **Reliable Connectivity**: Allows WebRTC connections even in restrictive networks.
- **Enhanced Network Compatibility**: Coturn ensures that peers can communicate regardless of NAT type.

**Disadvantages of Coturn**:
- **Increased Latency**: Data is relayed through the server, which can slow communication.
- **Higher Bandwidth Costs**: Using a TURN server adds to the server's bandwidth load.

## Usage
1. **Running the Application**:
   - Run the SignalingServer with
      ```bash
     cd Signaling\ Server/
     node SignalingServer.js
     ```
   - Open two instances of the application on the same or different devices.
   - Initiate a call on one instance to test the functionality.
2. **Test Audio Streaming**:
   - Use headphones or separate speakers/microphones on each instance to avoid feedback.

## Running The App

- **Run Two Instanses in QT**:
  - As shown in the image below run Offerer and Answerer intances by passing the arguments (0 for Answerer and 1 for Offerer)
  
  ![Projects Run](./docs/images/IMAGE%201403-08-13%2022:20:45.jpg)

- **Initiating Call**:
  
  ![Initiating Call](./docs/images/Screenshot%201403-08-13%20at%2022.19.30.png)

- **Ongoing Call**:
  
  ![Ongoing Call](./docs/images/Screenshot%201403-08-13%20at%2022.19.36.png)


- **Sample Logs**:

Here’s an example of log output during a WebRTC connection. The logs showcase various stages of the peer-to-peer connection, including the SDP exchange, connection state changes, and audio data transfer.

```plaintext
Register message sent for ID: "peer1"
WebRTC initialized with ID: "peer1" , as offerer , with payload type: 111 , and bit rate: 48000
[2024-11-03 20:34:19] [connect] Successful connection
Added audio track for peer: "peer2" with track name: "m=audio 49170 UDP/TLS/RTP/SAVPF 111"
[2024-11-03 20:34:19] [connect] WebSocket Connection [::1]:8080 v-2 "WebSocket++/0.8.2" /socket.io/?EIO=4&transport=websocket&t=1730653459 101
Connected to signaling server
Peer "peer2" connection state: Connecting
Generated SDP offer for peer ID: "peer2"
Gathering completed for peer "peer2"
Offer is ready for peer: "peer2"
SDP Description:
 "{\"sdp\":\"v=0\\r\\no=rtc 4145184004 0 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=group:BUNDLE audio\\r\\na=group:LS audio\\r\\na=msid-semantic:WMS *\\r\\na=ice-options:ice2,trickle\\r\\na=fingerprint:sha-256 7C:9D:1C:C6:28:E4:51:66:62:DE:6B:F2:30:55:EB:C9:A6:40:16:EB:EA:C8:9A:3E:E7:59:B8:E7:28:2F:19:DB\\r\\nm=audio 57319 UDP/TLS/RTP/SAVPF 111\\r\\nc=IN IP4 172.30.49.27\\r\\na=mid:audio\\r\\na=sendrecv\\r\\na=rtcp-mux\\r\\na=setup:actpass\\r\\na=ice-ufrag:LMov\\r\\na=ice-pwd:Y5DGPIVXrz/YS3JXgCWjTP\\r\\na=candidate:1 1 UDP 2122317823 172.30.49.27 57319 typ host\\r\\na=candidate:2 1 UDP 2122317567 172.26.32.1 57319 typ host\\r\\na=candidate:3 1 UDP 1686109695 188.118.92.98 57319 typ srflx raddr 0.0.0.0 rport 0\\r\\na=end-of-candidates\\r\\n\",\"type\":\"offer\"}"
SDP offer message sent for peer ID: "peer2"
Message received from server: "{\"type\":\"answer\",\"MyId\":\"peer2\",\"sdp\":\"{\\\"sdp\\\":\\\"v=0\\r\\no=rtc 3299663448 0 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=group:BUNDLE audio\\r\\na=group:LS audio\\r\\na=msid-semantic:WMS *\\r\\na=ice-options:ice2,trickle\\r\\na=fingerprint:sha-256 4A:A8:CB:84:9A:39:CC:B0:DA:41:5E:94:76:E0:95:30:2D:87:7D:6C:C2:76:F2:E2:19:02:91:94:3B:C4:92:D0\\r\\nm=audio 57422 UDP/TLS/RTP/SAVPF 111\\r\\nc=IN IP4 172.30.49.27\\r\\na=mid:audio\\r\\na=sendrecv\\r\\na=rtcp-mux\\r\\na=setup:active\\r\\na=ice-ufrag:HIym\\r\\na=ice-pwd:I5R9u67pNxWbLKg0hP3FFq\\r\\na=candidate:1 1 UDP 2122317823 172.30.49.27 57422 typ host\\r\\na=candidate:2 1 UDP 2122317567 172.26.32.1 57422 typ host\\r\\na=candidate:5 1 UDP 1686109183 188.118.92.98 57422 typ srflx raddr 0.0.0.0 rport 0\\r\\na=end-of-candidates\\r\\n\\\",\\\"type\\\":\\\"answer\\\"}\"}"
Set remote description for peerID: "peer2"
Peer "peer2" connection state: Connected
Received audio data of size: 960
Encoded data size: 104
Sent RTP packet to peer "peer2" with size: 116
Received audio message with timestamp:
packet for: "peer1" from peer: "peer2"
```

## Difficulties and Challenges Faced

During the development process, we encountered a few significant challenges that required investigation and adjustments. Here are some issues we faced:

1. **AudioOutput Instance Not Created**: 
   One of our primary issues was that the `audioOutput` instance in the client was never created at all. When we attempted to call methods from the `audioOutput` class, they appeared to execute normally. However, when any of these methods attempted to access certain private fields of the `audioOutput` class, the application crashed with a segmentation fault. This issue required thorough debugging to identify the failure in instance initialization, leading us to revise the logic to ensure proper creation of the `audioOutput` instance.


2. **Opus Linking Error**: 
   In the early stages, we faced difficulties with linking the Opus library due to a “check stack” error. We attempted to resolve this by adding the `-fstack-protector` flag to `QMAKE_CXXFLAGS` in our `.pro` file, as recommended. However, this solution initially failed because we had placed the flag in an incorrect location within the `.pro` file. After some experimentation, we discovered that the correct placement was directly before the `CONFIG` line. This adjustment resolved the stack-check error and allowed the linking process to complete successfully.
