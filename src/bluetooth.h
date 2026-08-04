#pragma once
#include <string>

int open_bluetooth_uart(const char *device);
bool connect_bluetooth(int fd, const std::string &address);
void send_bluetooth_message(int fd, const std::string &message);
bool test_connection(int fd);
bool bluetooth_read_line(int fd, std::string &result);
void bluetooth_receive_results(int fd);