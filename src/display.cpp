#include "display.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>

void Display::update(const uint8_t* buffer) {
    if (!buffer) return;
    
    uint8_t msg_type;
    std::memcpy(&msg_type, buffer, sizeof(uint8_t));
    std::printf("Message type: %02b\n", msg_type);
    std::memcpy(&ADC1_CH1, &buffer[1], sizeof(float));
    std::memcpy(&ADC1_CH2, &buffer[5], sizeof(float));
    std::memcpy(&ADC1_CH3, &buffer[9], sizeof(float));
    std::memcpy(&ADC2, &buffer[13], sizeof(float));
    std::memcpy(&ADC3, &buffer[17], sizeof(float));
    std::memcpy(&checksum, &buffer[29], sizeof(uint16_t));
    std::printf("Checksum received: %u\n", checksum);
}
