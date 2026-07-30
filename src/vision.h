#pragma once

void find_threshold(cv::Mat &frame, cv::Mat &hsv_frame, int iLowH, int iLowS, int iLowV, int iHighH, int iHighS, int iHighV, cv::Mat &thresh_frame, int iOpen, int iClose);

bool detect_bounce(const cv::Point2f &position, const cv::Point2f &prev_position, const cv::Point2f &prev_velocity, double dt, cv::Point2f &velocity);