#include "uartComms.hpp"
#include "display.hpp"
#include "throttlePacket.hpp"
#include <array>
#include <cstdint>
#include <cstdio>

UARTComms::UARTComms(const char* device) : running_RX(false), fd(-1) {
    fd = open_port(device);
    initialized = true;
}

UARTComms::~UARTComms() {
    stop();
    if (fd >= 0) close(fd);
}

void UARTComms::start() {
    running_RX = true;
    running_TX = true;
    thread_RX = std::thread(&UARTComms::getSerialInput, this);

    thread_TX = std::thread(&UARTComms::sendThrottlePacket, this);
    std::cout << "zart\n";
}

void UARTComms::stop() {
    running_RX = false;
    running_TX = false;
    if (thread_RX.joinable()) thread_RX.join();
    if (thread_TX.joinable()) thread_TX.join();
}

void UARTComms::set_throttle(uint16_t value) {
    std::lock_guard<std::mutex> lock(throttle_mutex);
    throttlePacket.arm_status = 0b10;
    throttlePacket.throttle_value = value;
    throttlePacket.update_checksum(display.checksum);
}

void UARTComms::arm() {
    std::lock_guard<std::mutex> lock(throttle_mutex);
    throttlePacket.arm_status = 0b01; 
}

void UARTComms::disarm() {
    std::lock_guard<std::mutex> lock(throttle_mutex);
    throttlePacket.arm_status = 0b00; 
}
void UARTComms::reset() {
    std::lock_guard<std::mutex> lock(throttle_mutex);
    throttlePacket.arm_status = 0b11; 
}

void UARTComms::get_log(float* logs) {
    std::lock_guard<std::mutex> lock(log_mutex);
    logs[0] = (display.ADC1_CH1);
    logs[1] = (display.ADC1_CH2);
    logs[2] = (display.ADC1_CH3);
    logs[3] = (display.ADC2);
    logs[4] = (display.ADC3);
}

bool UARTComms::is_connected() const { 
    return fd >= 0; 
}

uint16_t UARTComms::calculate_checksum(uint8_t* buf) {
    uint16_t cksum = 0; 
    for (int i = 0; i < RX_PACKET_LEN - 2; i++) {
        cksum ^= buf[i];
    }

    return cksum;
}

void UARTComms::getSerialInput() {
    while (!initialized) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    while (running_RX) {
      uint8_t buf[RX_PACKET_LEN] = {0};
    
        if (fd < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        int n = read(fd, buf, sizeof(buf));

        /*for (int i = 0; i<RX_PACKET_LEN; i++) {
            std::printf("buf[%d]: %u\n", i, buf[i]);
        }*/

        if (n == RX_PACKET_LEN) {
            std::lock_guard<std::mutex> lock(log_mutex);
            display.update(buf);
            display.checksum = calculate_checksum(buf);
            // std::printf("Calculated checksum %d\n", display.checksum);
        } else {
            //std::printf("Received bytes: %d\n", n);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void UARTComms::sendThrottlePacket() {
    while (!initialized) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    while(running_TX) {
        if (fd < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        std::array<uint8_t, 5> packet_data;
        {
            std::lock_guard<std::mutex> lock(throttle_mutex);
            packet_data = throttlePacket.get_packet();
        }
        uint16_t temp; 
        std::memcpy(&packet_data[1], &temp, sizeof(uint16_t));
        
        int n = write(fd, packet_data.data(), 5);
        std::cout << "Write completed, n = " << n << "\n";
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
int open_port(const char* device) {
    int fd = open(device, O_RDWR | O_NOCTTY);
    if (fd < 0) return -1;

    struct termios tty {};
    if (tcgetattr(fd, &tty) != 0) return -1;

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // TEST
    cfmakeraw(&tty);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_lflag = 0;
    tty.c_oflag = (ONOCR);
    tty.c_cc[VMIN] = RX_PACKET_LEN;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) return -1;

    //""sleep(2);
    tcflush(fd, TCIOFLUSH);
    return fd;
}

