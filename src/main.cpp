#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "bluetooth.h"
#include "vision.h"
#include "board.h"
#include "thingspeak.h"

#include <iostream>
#include <chrono>
#include <cmath>

#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <thread>
#include <csignal>

int bt = -1;

using namespace cv;

/**
 * @brief Send updated position over Bluetooth
 * @param player_side Left or Right, depending on which side the ball is on
 * @param ball_position Center or Edge of the table
 * @param x_position Current x position
 */
void update_position(uint8_t &player_side, uint8_t &ball_position, uint16_t x_position)
{
    if (is_side_change(player_side, x_position))
    {
        const char *msg = (player_side == LEFT) ? "Left\n" : "Right\n";
        send_bluetooth_message(bt, msg);
    }
    if (is_ball_position_change(ball_position, x_position))
    {
        const char *msg = (ball_position == EDGE) ? "Edge\n" : "Center\n";
        send_bluetooth_message(bt, msg);
    }
}

/**
 * @brief Initiate shutdown sequence, including breaking bluetooth connection
 * @param signum Signal for the shutdown sequence
 */
void shutdown_handler(int signum)
{
    std::cout << "\nShutting down...\n";
    if (bt >= 0)
    {
        const char *msg = "Reboot\n";
        send_bluetooth_message(bt, msg);
        // Give the Pico time to act on it
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        close(bt);
    }
    std::_Exit(0);
}

int main()
{
    read_total_wins();
    signal(SIGINT, shutdown_handler);
    // Open the video camera
    std::string pipeline = "libcamerasrc"
                           " ! video/x-raw, width=800, height=600, framerate=60/1"
                           " ! videoconvert"
                           " ! videoscale"
                           " ! video/x-raw, width=400, height=300"
                           " ! appsink drop=true max_buffers=2";
    // " ! videoflip method=rotate-180"
    VideoCapture cap(pipeline, CAP_GSTREAMER);
    if (!cap.isOpened())
    {
        std::cout << "Could not open camera." << std::endl;
        return 1;
    }
    namedWindow("Camera", WINDOW_AUTOSIZE);
    namedWindow("Thresholded", WINDOW_AUTOSIZE);
    namedWindow("Control", WINDOW_AUTOSIZE);
    // HSV controls
    int iLowH = 137;
    int iHighH = 166;
    int iLowS = 59;
    int iHighS = 255;
    int iLowV = 0;
    int iHighV = 255;
    int iOpen = 5;
    int iClose = 0;
    int iMinThresh = 10;
    int iMaxThresh = 27;
    int iMaxSpeed = 100;
    // int iLowH = 18;
    // int iHighH = 79;
    // int iLowS = 4;
    // int iHighS = 255;
    // int iLowV = 0;
    // int iHighV = 255;
    // int iOpen = 4;
    // int iClose = 1;
    createTrackbar("LowH", "Control", &iLowH, 179);
    createTrackbar("HighH", "Control", &iHighH, 179);
    createTrackbar("LowS", "Control", &iLowS, 255);
    createTrackbar("HighS", "Control", &iHighS, 255);
    createTrackbar("LowV", "Control", &iLowV, 255);
    createTrackbar("HighV", "Control", &iHighV, 255);
    createTrackbar("Open", "Control", &iOpen, 10);
    createTrackbar("Close", "Control", &iClose, 10);
    createTrackbar("MinimumSize", "Control", &iMinThresh, 100);
    createTrackbar("MaximumSize", "Control", &iMaxThresh, 100);
    createTrackbar("MaximumSpeed", "Control", &iMaxSpeed, 200);
    Mat frame, hsv_frame, thresh_frame;
    // Tracking state
    Point2f prev_position(0, 0);
    Point2f prev_velocity(0, 0);
    bool has_previous = false;
    using Clock = std::chrono::steady_clock;
    auto prev_time = Clock::now();
    auto fps_start = Clock::now();
    int fps_frames = 0;
    // Bluetooth
    bt = open_bluetooth_uart("/dev/serial0");
    if (bt >= 0)
    {
        connect_bluetooth(bt, "FC0FE7BFE75B");
    }

    uint8_t ball_position = NUM_POSITIONS;
    uint8_t player_side = NUM_SIDES;

    while (true)
    {
        if (!cap.read(frame))
        {
            std::cout << "Could not read a frame." << std::endl;
            break;
        }
        find_threshold(frame, hsv_frame, iLowH, iLowS, iLowV, iHighH, iHighS, iHighV, thresh_frame, iOpen, iClose);
        // Find contours
        std::vector<std::vector<Point>> contours;
        findContours(thresh_frame, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        // Find largest contour
        double largest_area = 0;
        int largest_index = -1;
        for (size_t i = 0; i < contours.size(); ++i)
        {
            double area = contourArea(contours[i]);
            if (area < iMinThresh || area > iMaxThresh)
            {
                continue;
            }
            if (area > largest_area)
            {
                largest_area = area;
                largest_index = static_cast<int>(i);
            }
        }
        // std::cout << "Largest area = " << largest_area << std::endl;
        if (largest_index >= 0)
        {
            Moments m = moments(contours[largest_index]);

            if (m.m00 > 0)
            {
                Point2f position(m.m10 / m.m00, m.m01 / m.m00);

                auto now = Clock::now();
                double dt = std::chrono::duration<double>(now - prev_time).count();

                Point2f velocity(0, 0);

                // Assume valid unless proven otherwise
                bool valid_motion = true;

                if (has_previous)
                {
                    double dx = std::abs(position.x - prev_position.x);
                    double dy = std::abs(position.y - prev_position.y);

                    valid_motion = (dx < iMaxSpeed) &&
                                   (dy < iMaxSpeed);
                }

                // Only process if movement is reasonable
                if (valid_motion)
                {
                    // ----- Draw object -----
                    drawContours(frame, contours, largest_index, Scalar(0, 255, 0), 2);
                    circle(frame, position, 5, Scalar(0, 0, 255), FILLED);

                    // ----- Bounce detection -----
                    if (has_previous && dt > 1e-6)
                    {
                        bool bounced = detect_bounce(position, prev_position,
                                                     prev_velocity, dt, velocity);

                        if (bounced)
                        {
                            putText(frame, "BOUNCE DETECTED", Point(10, 30),
                                    FONT_HERSHEY_SIMPLEX, 0.8,
                                    Scalar(0, 0, 255), 2);

                            send_bluetooth_message(bt, "Bounce\n");
                        }
                    }

                    // ----- Draw coordinates -----
                    double display_y = frame.rows - position.y;

                    putText(frame,
                            "(" + std::to_string((int)position.x) + ", " +
                                std::to_string((int)display_y) + ")",
                            Point(position.x + 10, position.y - 10),
                            FONT_HERSHEY_SIMPLEX, 0.5,
                            Scalar(0, 255, 0), 2);

                    // ----- Bluetooth update -----
                    update_position(player_side, ball_position, (int)position.x);

                    // ----- Update history -----
                    prev_position = position;
                    prev_velocity = velocity;
                    prev_time = now;
                    has_previous = true;
                }
                // else
                // {
                //     // Ignore this detection completely
                //     // Do not draw it, do not send Bluetooth messages,
                //     // and do not update prev_position.

                //     std::cout << "Ignored jump: dx="
                //               << std::abs(position.x - prev_position.x)
                //               << " dy="
                //               << std::abs(position.y - prev_position.y)
                //               << std::endl;
                // }
            }
        }
        else
        {
            has_previous = false;
        }
        imshow("Thresholded", thresh_frame);
        imshow("Camera", frame);
        fps_frames++;
        auto fps_now = Clock::now();
        double elapsed = std::chrono::duration<double>(fps_now - fps_start).count();
        if (waitKey(1) == 27)
        {
            break;
        }
        bluetooth_receive_results(bt);
    }
    cap.release();
    destroyAllWindows();
    return 0;
}
