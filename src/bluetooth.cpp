#include "bluetooth.h"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <thread>
#include <chrono>
#include <sstream>
#include "board.h"
#include "scorestate.h"
#include "thingspeak.h"

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
    cfmakeraw(&options);                 // sets raw-mode flags for you
    options.c_cflag |= (CLOCAL | CREAD); // still needed, cfmakeraw doesn't set these
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
    send_bluetooth_message(fd, "PING\n");
    std::string line;
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(5))
    {
        if (bluetooth_read_line(fd, line) && line.find("PONG") != std::string::npos)
        {
            std::cout << "Bluetooth link verified\n";
            return true;
        }
    }
    std::cout << "No response from remote board. Trying again.\n";
    return false;
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

bool bluetooth_read_line(int fd, std::string &result)
{
    static std::string buffer; // holds any partial data between calls
    char chunk[32];
    int n = read(fd, chunk, sizeof(chunk));
    if (n > 0)
    {
        buffer.append(chunk, n);
    }
    size_t nl = buffer.find('\n');
    if (nl != std::string::npos)
    {
        result = buffer.substr(0, nl);
        buffer.erase(0, nl + 1);
        return true;
    }
    else
    {
        return false;
    }
}

void bluetooth_receive_results(int fd)
{
    std::string result;
    int player;

    if (bluetooth_read_line(fd, result))
    {
        if (std::sscanf(result.c_str(), "Player1: %d, Player2: %d", &scores[PLAYER_1], &scores[PLAYER_2]) == 2)
        {
            printf("Score updated: Player1: %d  Player2: %d", scores[0], scores[1]);
            upload_state(scores[PLAYER_1], scores[PLAYER_2],
                         session_wins[PLAYER_1], session_wins[PLAYER_2],
                         total_wins[PLAYER_1], total_wins[PLAYER_2]);
        }
        else if (std::sscanf(result.c_str(), "Won: Player%d", &player) == 1)
        {
            session_wins[player - 1]++;
            total_wins[player - 1]++;
            printf("Player %d has won!\r\n", player);
            upload_state(scores[PLAYER_1], scores[PLAYER_2],
                         session_wins[PLAYER_1], session_wins[PLAYER_2],
                         total_wins[PLAYER_1], total_wins[PLAYER_2]);
        }
    }
}