#pragma once

#include <iostream>
#include <span>
#include <string>
#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "buttstream.h"
#include "error.h"

namespace hs {

    enum class MessageType: uint8_t {
        Initialize = 0,
        InitializeResponse = 1,
        FatalError = 2,
        Error = 3,
        AsyncLock = 4,
        AsyncLockResponse = 5,
        Data = 6,
        DataEnd = 7,
        DeviceClearComplete = 8,
        DeviceClearAcknowledge = 9,
        AsyncRemoteLocalControl = 10,
        AsyncRemoteLocalResponse = 11,
        Trigger = 12,
        Interrupted = 13,
        AsyncInterrupted = 14,
        AsyncMaximumMessageSize = 15,
        AsyncMaximumMessageSizeResponse = 16,
        AsyncInitialize = 17,
        AsyncInitializeResponse = 18,
        AsyncDeviceClear = 19,
        AsyncServiceRequest = 20,
        AsyncStatusQuery = 21,
        AsyncStatusResponse = 22,
        AsyncDeviceClearAcknowledge = 23,
        AsyncLockInfo = 24,
        AsyncLockInfoResponse = 25,
        GetDescriptors = 26,
        GetDescriptorsResponse = 27,
        StartTLS = 28,
        AsyncStartTLS = 29,
        AsyncStartTLSResponse = 30,
        EndTLS = 31,
        AsyncEndTLS = 32,
        AsyncEndTLSResponse = 33,
        GetSaslMechanismList = 34,
        GetSaslMechanismListResponse = 35,
        AuthenticationStart = 36,
        AuthenticationExchange = 37,
        AuthenticationResult = 38,
    };

    std::ostream& operator<<(std::ostream& os, const MessageType& mt) {
        os << "MessageType(" << static_cast<uint8_t>(mt) << ")";
        return os;
    }

    struct Message {
        MessageType message_type{};
        uint8_t control_code{};
        uint32_t message_param{};
        std::string data;

        static Message from_bytes(std::string_view bytes) {
            if (bytes.size() < 16) {
                throw std::range_error("not enough bytes to parse header");
            }
            if (bytes[0] != 'H' && bytes[1] != 'S') {
                throw "prologue not HS";
            }

            Message m = {
                .message_type = static_cast<MessageType>(bytes[2]),
            };

            ButtstreamReader(bytes.substr(3, 1)) >> m.control_code;
            ButtstreamReader(bytes.substr(4, 4)) >> m.message_param;
            uint64_t payload_length{};
            ButtstreamReader(bytes.substr(8, 8)) >> payload_length;

            const auto data = bytes.substr(16);
            if (payload_length > 0 && !data.empty()) {
                if (data.size() < payload_length) {
                    throw std::range_error("not enough data to hold payload");
                }
                m.data = std::string(data.begin(), data.begin() + payload_length);
            }
            return m;
        }
        std::string to_bytes()&& {
            uint64_t payload_length = data.length();
            std::string b = {
                static_cast<char>('H'),
                static_cast<char>('S'),
                static_cast<char>(message_type),
                static_cast<char>(control_code),
                static_cast<char>(message_param >> 24),
                static_cast<char>((message_param >> 16) & 0xff),
                static_cast<char>((message_param >> 8) & 0xff),
                static_cast<char>((message_param >> 0) & 0xff),
                static_cast<char>((payload_length >> 56) & 0xff),
                static_cast<char>((payload_length >> 48) & 0xff),
                static_cast<char>((payload_length >> 40) & 0xff),
                static_cast<char>((payload_length >> 32) & 0xff),
                static_cast<char>((payload_length >> 24) & 0xff),
                static_cast<char>((payload_length >> 16) & 0xff),
                static_cast<char>((payload_length >> 8) & 0xff),
                static_cast<char>((payload_length >> 0) & 0xff),
            };

            b.insert(b.end(), data.begin(), data.end());
            return b;
        }

        bool operator==(const Message& rhs) const = default;
    };
    std::ostream& operator<<(std::ostream& os, const Message& ms) {
        os << "Message{"
            << "message_type: " << uint32_t(ms.message_type) << ", "
            << "control_code: " << uint32_t(ms.control_code) << ", "
            << "message_param: " << ms.message_param << ", "
            << "payload_length: " << ms.data.size() << ", "
            << "data: \"" << ms.data << "\""
            << "}";
        return os;
    }

};