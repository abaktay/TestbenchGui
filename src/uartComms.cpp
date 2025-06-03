#include "uartComms.hpp"
#include "display.hpp"
#include "throttlePacket.hpp"
#include <array>
#include <cstdint>
#include <cstdio>

UARTComms::UARTComms(const char* device) : running_RX(false), fd(-1) {
    fd = open_port(device);
}

UARTComms::~UARTComms() {
    stop();
    if (fd >= 0) close(fd);
}

void UARTComms::start() {
    if (running_RX) return;
    if (running_TX) return;
    running_RX = true;
    running_TX = true;
    thread_RX = std::thread(&UARTComms::getSerialInput, this);

    std::cout << "zart\n";
    thread_TX = std::thread(&UARTComms::sendThrottlePacket, this);
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
    throttlePacket.update(value);
    throttlePacket.update_checksum(display.checksum);
}

void UARTComms::arm() {
    throttlePacket.arm_status = 0b01; 
}

void UARTComms::disarm() {
    throttlePacket.arm_status = 0b00; 
}
void UARTComms::reset() {
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
            display.update(buf);
            display.checksum = calculate_checksum(buf);
            std::printf("Calculated checksum %d\n", display.checksum);
        } else {
            //std::printf("Received bytes: %d\n", n);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void UARTComms::sendThrottlePacket() {
    while(running_TX) {
        if (fd < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        int n = write(fd, throttlePacket.get_packet().data(), TX_PACKET_LEN*sizeof(uint8_t));

        if (n == TX_PACKET_LEN) {
            //std::cout << "SENT PACKET\n";
        }
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

