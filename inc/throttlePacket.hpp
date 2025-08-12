#pragma once
#include <array>
#include <cstdint>

constexpr int TX_PACKET_LEN = 5;
constexpr int RX_PACKET_LEN = 30;

struct ThrottlePacket {
    uint8_t arm_status = 0b10;
    uint16_t header[2] = {0x28, 0x11};
    uint16_t throttle_value = 0;
    uint16_t checksum = 0;
    uint16_t header_cksum = 0x28 ^ 0x11;  // Calculate directly with constants
    
    // Default constructor
    ThrottlePacket() = default;
    
    // Constructor with value
    ThrottlePacket(uint16_t value) ;    
    void update(uint16_t new_value);
    void update_checksum(uint16_t new_cksum);
    std::array<uint8_t, 5> get_packet() const;
};
