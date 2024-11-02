const fs = require("fs");
const https = require("https");
const io = require("socket.io");

// Read the SSL certificate and key files
const options = {
    key: fs.readFileSync("key.pem"),
    cert: fs.readFileSync("cert.pem")
};

// Create HTTPS server
const httpsServer = https.createServer(options);
const socketServer = io(httpsServer, {
    cors: {
        origin: "*", // Allow all origins for testing; specify origins in production
        methods: ["GET", "POST"]
    }
});

let clients = new Map();

const MessageType = Object.freeze({
    register: "register",
    offer: "offer",
    answer: "answer",
});

// Listen for client connections
socketServer.on('connection', (socket) => {
    console.log('A client connected:', socket.id); // Log connection

    socket.on('message', (message) => {
        const data = JSON.parse(message);
        console.log(data);

        switch (data.type) {
            case MessageType.register:
                clients.set(data.id, socket);
                break;

            case MessageType.offer:
                let offerer = clients.get(data.answererId);
                if (offerer) {
                    offerer.emit('message', JSON.stringify({
                        type: MessageType.offer,
                        MyId: data.MyId,
                        sdp: data.sdp
                    }));
                }
                console.log(data);
                break;

            case MessageType.answer:
                let answerer = clients.get(data.offererId);
                if (answerer) {
                    answerer.emit('message', JSON.stringify({
                        type: MessageType.answer,
                        MyId: data.MyId,
                        sdp: data.sdp
                    }));
                }
                break;

            default:
                console.log("ERROR: Unknown message type");
        }
    });

    // Handle client disconnection
    socket.on('disconnect', () => {
        console.log('A client disconnected:', socket.id);
        // Remove the client from the clients map
        clients.forEach((client, id) => {
            if (client === socket) {
                clients.delete(id);
                console.log(`Client ${id} removed from clients map`);
            }
        });
    });
});

// Start the HTTPS server
httpsServer.listen(8080, () => {
    console.log("Signaling server is running on https://localhost:8080");
});
