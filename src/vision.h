#pragma once

/**
 * @brief Find thresholded image of camera frame
 * @param frame Current frame
 * @param hsv_frame Where the new frame will be stored
 * @param iLowH Lowest Hue value
 * @param iLowS Lowest Saturation value
 * @param iLowV Lowest Value value
 * @param iHighH Highest Hue value
 * @param iHighS Highest Saturation value
 * @param iHighV Highest Value value
 * @param thresh_frame Where the thresholded data is stored
 * @param iOpen Size of Open kernal
 * @param iClose Size of Close kernal
 */
void find_threshold(cv::Mat &frame, cv::Mat &hsv_frame, int iLowH, int iLowS, int iLowV, int iHighH, int iHighS, int iHighV, cv::Mat &thresh_frame, int iOpen, int iClose);

/**
 * @brief Detect a bounce in the movement between frames
 * @param position Current position of the object detected
 * @param prev_position Previous position of the object detected
 * @param prev_velocity Previous velocity of the object detected
 * @param dt time difference between the detections
 * @param velocity Current velocity of the object detected
 */
bool detect_bounce(const cv::Point2f &position, const cv::Point2f &prev_position, const cv::Point2f &prev_velocity, double dt, cv::Point2f &velocity);

/**
 * @brief Detect if ball has changed from centre to edge side or vice versa
 * @param ball_position current ball position (center or edge)
 * @param x_position Current x position of the detected object
 */
bool is_ball_position_change(uint8_t &ball_position, int x_position);

/**
 * @brief Detect if ball has changed from left or right (or vice versa)
 * @param player_side Current ball location (left or right)
 * @param x_position Current x position of the detected object
 */
bool is_side_change(uint8_t &player_side, int x_position);