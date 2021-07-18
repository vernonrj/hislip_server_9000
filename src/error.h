#pragma once

#include <stdint.h>

namespace hs {
    enum class FatalErrorCode: uint8_t {
        UnidentifiedError = 0,
        PoorlyFormedHeader = 1,
        ChannelsNotEstablished = 2,
        InvalidInitSeq = 3,
        MaxClientsExceeded = 4,
        SecureConnectionFailed = 5,
    };

    std::ostream& operator<<(std::ostream& os, const FatalErrorCode& ec) {
        os << "ErrorCode(";
        switch (ec) {
            case FatalErrorCode::UnidentifiedError:
                os << "Unidentified Error";
                break;
            case FatalErrorCode::PoorlyFormedHeader:
                os << "Poorly formed message header";
                break;
            case FatalErrorCode::ChannelsNotEstablished:
                os << "Attempt to use connection without both channels established";
                break;
            case FatalErrorCode::InvalidInitSeq:
                os << "Invalid Initialization Sequence";
                break;
            case FatalErrorCode::MaxClientsExceeded:
                os << "Server refused connection due to maximum number of clients exceeded";
                break;
            case FatalErrorCode::SecureConnectionFailed:
                os << "Secure connection failed";
                break;
            default:
                if (static_cast<uint8_t>(ec) <= 127) {
                    os << "Reserved for HiSLIP extensions";
                } else {
                    os << "Device defined errors";
                }
                break;
        }
        os << " = " << static_cast<uint32_t>(ec) << ")";
        return os;
    }

    enum class ErrorCode {
        UnidentifiedError = 0,
        UnrecognizedMessageType = 1,
        UnrecognizedControlCode = 2,
        UnrecognizedVendorMsg = 3,
        MessageTooLarge = 4,
        AuthFailed = 5,
    };

    std::ostream &operator<<(std::ostream &os, const ErrorCode &ec){
        os << "ErrorCode(";
        switch (ec) {
            case ErrorCode::UnidentifiedError:
                os << "Unidentified Error";
                break;
            case ErrorCode::UnrecognizedMessageType:
                os << "Unrecognized Message Type";
                break;
            case ErrorCode::UnrecognizedControlCode:
                os << "Unrecognized control code";
                break;
            case ErrorCode::UnrecognizedVendorMsg:
                os << "Unrecognized Vendor Defined Message";
                break;
            case ErrorCode::MessageTooLarge:
                os << "Message too large";
                break;
            case ErrorCode::AuthFailed:
                os << "Authentication failed";
                break;
            default:
                if (static_cast<uint8_t>(ec) <= 127) {
                    os << "Reserved for HiSLIP extensions";
                } else {
                    os << "Device defined errors";
                }
                break;
        }
        os << " = " << static_cast<uint32_t>(ec) << ")";
        return os;
    }
}