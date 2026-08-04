#pragma once

#include <curl/curl.h>
#include <string>
#include <opencv2/opencv.hpp>

void upload_state(int player_1_score, int player_2_score, int player_1_wins, int player_2_wins);
void update_total_wins();


