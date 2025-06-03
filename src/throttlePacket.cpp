#include "throttlePacket.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>

ThrottlePacket::ThrottlePacket(uint8_t value) : throttle_value(value) {
}

void ThrottlePacket::update(uint8_t new_value) {
    throttle_value = new_value;
    //std::printf("updated throttle: %d\n", throttle_value);
}

// something causes a problem
void ThrottlePacket::update_checksum(uint16_t new_cksum) {
    std::printf("Checksum before throttle packet %d\n", checksum);
    checksum = new_cksum;
    std::printf("Checksum from throttle packet %d\n", checksum);
}

std::array<uint8_t, 5> ThrottlePacket::get_packet() const {
    std::array<uint8_t, 5> packet = {0,0,0,0,0};
    packet[0] = arm_status;
    std::memcpy(&packet[1], &throttle_value, sizeof(uint16_t));
    std::memcpy(&packet[3], &checksum, sizeof(uint16_t));
    //std::printf("Throttle value %d\n", throttle_value);
    return packet;
}
