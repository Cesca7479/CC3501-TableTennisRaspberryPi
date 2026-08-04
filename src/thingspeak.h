#pragma once

#include <curl/curl.h>
#include <string>
#include <opencv2/opencv.hpp>

void upload_state(int player_1_score, int player_2_score, int player_1_session_wins, int player_2_session_wins, int player_1_total_wins, int player_2_total_wins);
void read_total_wins();


