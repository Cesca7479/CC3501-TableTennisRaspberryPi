#include "bluetooth.h"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <thread>
#include <chrono>

int open_bluetooth_uart(const char *device)
{
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        std::cerr << "Failed to open UART\n";
        return -1;
    }
    termios options{};
    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_lflag = 0;
    options.c_iflag = 0;
    options.c_oflag = 0;
    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

bool connect_bluetooth(int fd, const std::string &address)
{
    char buffer[128];
    std::string received;
    std::cout << "Entering command mode..." << std::endl;
    write(fd, "$$$", 3);
    tcdrain(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    std::string command = "C,0," + address + "\r";
    std::cout << "Connecting..." << std::endl;
    write(fd, command.c_str(), command.size());
    tcdrain(fd);
    auto start = std::chrono::steady_clock::now();
    while (true)
    {
        if (test_connection(fd))
        {
            std::cout << "Connected" << std::endl;
            return true;
        }
        if (std::chrono::steady_clock::now() - start >
            std::chrono::seconds(20))
        {
            std::cout << "\nBluetooth connection timeout\n";
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool test_connection(int fd)
{
    std::string message = "PING\n";
    send_bluetooth_message(fd, message);
    char buffer[128];
    std::string received;
    auto start = std::chrono::steady_clock::now();
    while (true)
    {
        int n = read(fd, buffer, sizeof(buffer));
        if (n > 0)
        {
            received.append(buffer, n);
            if (received.find("PONG") != std::string::npos)
            {
                std::cout << "Bluetooth link verified\n";
                return true;
            }
        }
        if (std::chrono::steady_clock::now() - start >
            std::chrono::seconds(5))
        {
            std::cout << "No response from remote board. Trying again.\n";
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void send_bluetooth_message(int bt_device, const std::string &message)
{
    if (bt_device < 0)
    {
        return;
    }
    write(bt_device, message.c_str(), message.size());
    tcdrain(bt_device);
}