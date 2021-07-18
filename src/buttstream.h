#pragma once
#include <iostream>
#include <string_view>

/**
 * Reads a network-ordered bitstream into integers, in host order. or at least, host order on little-endian systems.
 */
class ButtstreamReader {
public:
    ButtstreamReader(std::string_view buffer): m_buffer(buffer) {};

    // stream into a byte
    ButtstreamReader& operator>>(uint8_t& value) {
        if (m_buffer.size() < sizeof(value)) {
            throw std::range_error("buffer is empty");
        }
        value = m_buffer[0];
        m_buffer = m_buffer.substr(sizeof(value));
        return *this;
    }

    // stream into a 16-bit type
    ButtstreamReader& operator>>(uint16_t& value) {
        if (m_buffer.size() < sizeof(value)) {
            throw std::range_error("too few bytes to fill value");
        }
        value = ((uint16_t)(uint8_t)m_buffer[0] << 8)
            | ((uint16_t)(uint8_t)m_buffer[1] << 0);
        m_buffer = m_buffer.substr(sizeof(value));
        return *this;
    }

    // stream into a 32-bit type
    ButtstreamReader& operator>>(uint32_t& value) {
        if (m_buffer.size() < sizeof(value)) {
            throw std::range_error("too few bytes to fill value");
        }
        value = ((uint32_t)(uint8_t)m_buffer[0] << 24)
            | ((uint32_t)(uint8_t)m_buffer[1] << 16)
            | ((uint32_t)(uint8_t)m_buffer[2] << 8)
            | ((uint32_t)(uint8_t)m_buffer[3] << 0);
        m_buffer = m_buffer.substr(sizeof(value));
        return *this;
    }

    // stream into a 64-bit type
    ButtstreamReader& operator>>(uint64_t& value) {
        if (m_buffer.size() < sizeof(value)) {
            throw std::range_error("too few bytes to fill value");
        }
        value = ((uint64_t)(uint8_t)m_buffer[0] << 56)
            | ((uint64_t)(uint8_t)m_buffer[1] << 48)
            | ((uint64_t)(uint8_t)m_buffer[2] << 40)
            | ((uint64_t)(uint8_t)m_buffer[3] << 32)
            | ((uint64_t)(uint8_t)m_buffer[4] << 24)
            | ((uint64_t)(uint8_t)m_buffer[5] << 16)
            | ((uint64_t)(uint8_t)m_buffer[6] << 8)
            | ((uint64_t)(uint8_t)m_buffer[7] << 0);
        m_buffer = m_buffer.substr(sizeof(value));
        return *this;
    }

    // return the rest of the bytes from the buffer
    std::string_view rest_of_bytes()&& {
        auto view = std::move(m_buffer);
        m_buffer = m_buffer.substr(0, 0);
        return view;
    }
private:
    // borrowed bytestream.
    std::string_view m_buffer;
};