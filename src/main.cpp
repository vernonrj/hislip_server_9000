#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <optional>
#include <thread>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "4880"

#include "message.h"
#include "scpi_parser.h"

std::optional<hs::Message> read_message(SOCKET ClientSocket) {
    // Receive until the peer shuts down the connection
    char recvbuf[DEFAULT_BUFLEN];
    std::string message;
    message.reserve(DEFAULT_BUFLEN);

    int recvbuflen = DEFAULT_BUFLEN;
    int iResult = {};

    iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

    if (iResult > 0) {
        printf("Bytes received: %d\n", iResult);
        message.append(recvbuf, recvbuf + iResult);
        auto msg = hs::Message::from_bytes(message);
        std::cout << msg << std::endl;
        return msg;
    } else if (iResult == 0) {
        printf("Connection closing...\n");
        return {};
    } else  {
        printf("recv failed with error: %d\n", WSAGetLastError());
        return {};
    }
}

void serve(SOCKET sync) {
    // TODO
    std::cout << "SERVE" << std::endl;
    for (auto maybe_msg = read_message(sync); maybe_msg.has_value(); maybe_msg = read_message(sync)) {
        auto msg = *maybe_msg;
        std::cout << "Got message " << msg << std::endl;
        switch (msg.message_type) {
            case hs::MessageType::Initialize:
            case hs::MessageType::AsyncInitialize:
                std::cerr << "oh no, they sent an Initialize on the service threads" << std::endl;
                break;
            case hs::MessageType::AsyncMaximumMessageSize:
            {
                std::cout << "setting the max message size" << std::endl;
                // TODO
                hs::Message response = {
                    .message_type = hs::MessageType::AsyncMaximumMessageSizeResponse,
                    .control_code = 0,
                    .message_param = 0,
                    .data = msg.data,
                };
                auto response_str = std::move(response).to_bytes();
                send(sync, response_str.data(), response_str.size(), 0);
                break;
            }
            case hs::MessageType::Data:
            case hs::MessageType::DataEnd:
            {
                std::vector<hs::Message> responses;
                const auto message_id = msg.message_param;
                if (auto parsed = scpi::parse(msg.data); parsed.has_value()) {
                    scpi::Command cmd = *parsed;
                    std::cout << "DATA(" << msg.message_param << "): " << cmd << std::endl;
                    const auto header = cmd.norm_header();
                    std::cout << "header = " << header << std::endl;
                    if (header == "*IDN?") {
                        responses.push_back(hs::Message {
                            .message_type = hs::MessageType::DataEnd,
                            .message_param = message_id,
                            .data = "ayy lmao",
                        });
                    }
                } else {
                    std::cout << "got invalid command: " << msg.data << std::endl;
                }
                for (auto&& each_response: std::move(responses)) {
                    std::cout << "SEND response " << each_response << std::endl;
                    auto response = std::move(each_response).to_bytes();
                    send(sync, response.data(), response.size(), 0);
                }
                break;
            }
        }
    }
}

class HislipServer {
public:
    HislipServer() = default;
    HislipServer(SOCKET sync):
        m_sync(sync)
    {
        m_syncthread = std::thread([sync] { serve(sync); });
    }
    ~HislipServer() {
        // TODO
    }
    HislipServer(const HislipServer&) = delete;
    HislipServer& operator=(const HislipServer&) = delete;

    void init_async(SOCKET async) {
        m_async = async;
        m_asyncthread = std::thread([async] { serve(async); });
    }

private:
    SOCKET m_sync = INVALID_SOCKET;
    SOCKET m_async = INVALID_SOCKET;
    std::thread m_syncthread;
    std::thread m_asyncthread;
};


int main() {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    std::map<uint32_t, HislipServer> sessions;
    uint32_t session_number = 1;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Now taking connections" << std::endl;
    for (;;) {
        // Accept a client socket
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            continue;
        }
        std::cout << "accepted a connected socket" << std::endl;

        std::optional<hs::Message> maybe_msg = read_message(ClientSocket);
        if (!maybe_msg.has_value()) {
            continue;
        }
        const hs::Message& msg = *maybe_msg;
        switch (msg.message_type) {
            case hs::MessageType::Initialize:
            {
                // oh hey someone's trying to initialize with us!
                std::cout << "Initialize" << std::endl;
                hs::Message response = {
                    .message_type = hs::MessageType::InitializeResponse,
                    .control_code = 0,  // prefer synchronized
                    .message_param = 0x00010000 | session_number, // Negotiated Prot version / Session ID
                    .data = "",
                };
                std::cout << "SEND: " << response << std::endl;
                auto response_data = std::move(response).to_bytes();
                send(ClientSocket, response_data.data(), response_data.size(), 0);
                sessions.emplace(session_number++, ClientSocket);
                break;
            }
            case hs::MessageType::AsyncInitialize:
                // this is the last part that's needed for the setup
                std::cout << "Async Initialize" << std::endl;
                auto session = msg.message_param;
                if (auto found = sessions.find(session); found != sessions.end()) {
                    found->second.init_async(ClientSocket);
                }
                hs::Message response = {
                    .message_type = hs::MessageType::AsyncInitializeResponse,
                    .control_code = 0,  // prefer synchronized
                    .message_param = 0x1234, // Server-vendorID
                    .data = "",
                };
                std::cout << "SEND: " << response << std::endl;
                auto response_data = std::move(response).to_bytes();
                send(ClientSocket, response_data.data(), response_data.size(), 0);
                break;
        }
    }

    // shutdown the connection since we're done
    WSACleanup();

    return 0;
}