#pragma once

#include <curl/curl.h>
#include <string>
#include <opencv2/opencv.hpp>

/**
 * @brief Uploads player scores, session wins, and total wins to thingspeak server via http request
 */
void upload_state(int player_1_score, int player_2_score, int player_1_session_wins, int player_2_session_wins, int player_1_total_wins, int player_2_total_wins);

/**
 * @brief reads total wins from thingspeak server via http request
 */
void read_total_wins();