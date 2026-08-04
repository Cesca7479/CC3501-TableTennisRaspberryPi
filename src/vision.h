#pragma once

void find_threshold(cv::Mat &frame, cv::Mat &hsv_frame, int iLowH, int iLowS, int iLowV, int iHighH, int iHighS, int iHighV, cv::Mat &thresh_frame, int iOpen, int iClose);

bool detect_bounce(const cv::Point2f &position, const cv::Point2f &prev_position, const cv::Point2f &prev_velocity, double dt, cv::Point2f &velocity);

bool is_ball_position_change(uint8_t &ball_position, int x_position);

bool is_side_change(uint8_t &player_side, int x_position);