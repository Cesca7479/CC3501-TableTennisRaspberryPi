#include <opencv2/opencv.hpp>
#include "board.h"

void find_threshold(cv::Mat &frame, cv::Mat &hsv_frame, int iLowH, int iLowS, int iLowV, int iHighH, int iHighS, int iHighV, cv::Mat &thresh_frame, int iOpen, int iClose)
{
    // Convert to HSV and threshold
    cv::cvtColor(frame, hsv_frame, cv::COLOR_BGR2HSV);
    cv::inRange(hsv_frame, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), thresh_frame);
    if (iOpen > 0)
    {
        cv::morphologyEx(thresh_frame, thresh_frame, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(iOpen, iOpen)));
    }
    if (iClose > 0)
    {
        cv::morphologyEx(thresh_frame, thresh_frame, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(iClose, iClose)));
    }
}

bool detect_bounce(const cv::Point2f &position, const cv::Point2f &prev_position, const cv::Point2f &prev_velocity, double dt, cv::Point2f &velocity)
{
    cv::Point2f raw_velocity = (position - prev_position) * (1.0 / dt);         // Raw velocity (pixels/s)
    velocity = prev_velocity * 0.8f + raw_velocity * 0.2f;                      // Simple low-pass filter to reduce jitter
    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y); // Speeds
    float prev_speed = std::sqrt(prev_velocity.x * prev_velocity.x + prev_velocity.y * prev_velocity.y);
    bool y_reversed = (velocity.y * prev_velocity.y < 0);                    // Vertical reversal
    float dot = velocity.x * prev_velocity.x + velocity.y * prev_velocity.y; // Angle between velocity vectors
    float cos_theta = 1.0f;
    if (speed > 1 && prev_speed > 1)
    {
        cos_theta = dot / (speed * prev_speed);
        cos_theta = std::max(-1.0f, std::min(1.0f, cos_theta));
    }
    float angle_degrees = std::acos(cos_theta) * 180.0f / CV_PI;
    return (speed > 50 && prev_speed > 50) && (y_reversed || angle_degrees > 20.0f);
}

bool is_ball_position_change(uint8_t &ball_position, int x_position)
{
    if (x_position < 100 || x_position > 300)
    {
        if (ball_position != EDGE)
        {
            ball_position = EDGE;
            return 1;
        }
    }
    else
    {
        if (ball_position != CENTER)
        {
            ball_position = CENTER;
            return 1;
        }
    }
    return 0;
}

bool is_side_change(uint8_t &player_side, int x_position)
{
    if (x_position <= 200)
    {
        if (player_side != LEFT)
        {
            player_side = LEFT;
            return 1;
        }
    }
    else
    {
        if (player_side != RIGHT)
        {
            player_side = RIGHT;
            return 1;
        }
    }
    return 0;
}