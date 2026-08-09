#pragma once
#include <string>

/**
 * @brief Initialise the uart connection of the bluetooth
 * @param device Bluetooth device
 */
int open_bluetooth_uart(const char *device);

/**
 * @brief Connect the two bluetooth devices together
 * @param fd Bluetooth device
 * @param address Address of the other device to connect
 */
bool connect_bluetooth(int fd, const std::string &address);

/**
 * @brief Send a bluetooth message
 * @param fd Bluetooth device
 * @param message Message to be sent
 */
void send_bluetooth_message(int fd, const std::string &message);

/**
 * @brief Test current connection status by sending and receiving a particular message
 * @param fd Bluetooth device
 */
bool test_connection(int fd);

/**
 * @brief Recieve and read a bluetooth message
 * @param fd Bluetooth device
 * @param result Bluetooth message received
 */
bool bluetooth_read_line(int fd, std::string &result);

/**
 * @brief Receive results (current score and if there is a win) from a bluetooth message
 * @param fd Bluetooth device
 */
void bluetooth_receive_results(int fd);