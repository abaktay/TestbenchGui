#pragma once
#include <cstdint>

struct Display {
    float ADC1_CH1 = 0;
    float ADC1_CH2 = 0;
    float ADC1_CH3 = 0;
    float ADC2 = 0;
    float ADC3 = 0;
    uint16_t checksum = 0;

    void update(const uint8_t* buffer);
};
