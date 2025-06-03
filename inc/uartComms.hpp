#pragma once
#include "display.hpp"
#include "throttlePacket.hpp"
#include <array>
#include <atomic>
#include <bit>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <string>
#include <sys/types.h>
#include <termios.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <vector>


class UARTComms {
public:
    UARTComms(const char* device);
    ~UARTComms();

    void start();
    void stop();
    void set_throttle(uint16_t value);
    void get_log(float* logs);
    void arm();
    void disarm();
    void reset();
    bool is_connected() const;
    uint16_t calculate_checksum(uint8_t* buf);

private:
    std::atomic<bool> running_RX{false}, running_TX{false};
    int fd;
    std::thread thread_RX;
    std::thread thread_TX;
    ThrottlePacket throttlePacket{0};
    std::mutex log_mutex, throttle_mutex;
    Display display;

    void getSerialInput();
    void sendThrottlePacket();
};

int open_port(const char* device);

