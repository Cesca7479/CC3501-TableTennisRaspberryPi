#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <chrono>
#include <cmath>

using namespace cv;

int main()
{
    // Open the video camera
    std::string pipeline = "libcamerasrc"
                           " ! video/x-raw, width=800, height=600, framerate=60/1"
                           " ! videoconvert"
                           " ! videoscale"
                           " ! video/x-raw, width=400, height=300"
                           " ! videoflip method=rotate-180"
                           " ! appsink drop=true max_buffers=2";

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
    int iLowH = 40;
    int iHighH = 79;
    int iLowS = 2;
    int iHighS = 255;
    int iLowV = 66;
    int iHighV = 255;
    int iOpen = 9;
    int iClose = 3;

    createTrackbar("LowH", "Control", &iLowH, 179);
    createTrackbar("HighH", "Control", &iHighH, 179);
    createTrackbar("LowS", "Control", &iLowS, 255);
    createTrackbar("HighS", "Control", &iHighS, 255);
    createTrackbar("LowV", "Control", &iLowV, 255);
    createTrackbar("HighV", "Control", &iHighV, 255);
    createTrackbar("Open", "Control", &iOpen, 10);
    createTrackbar("Close", "Control", &iClose, 10);

    Mat frame, hsv_frame, thresh_frame;

    // Tracking state
    Point2f prevPos;
    Point2f prevVel(0, 0);
    bool hasPrev = false;

    using Clock = std::chrono::steady_clock;
    auto prevTime = Clock::now();
    // auto fpsStart = Clock::now();
    // int fpsFrames = 0;

    for (;;)
    {
        if (!cap.read(frame))
        {
            std::cout << "Could not read a frame." << std::endl;
            break;
        }

        // Convert to HSV and threshold
        cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

        inRange(hsv_frame,
                Scalar(iLowH, iLowS, iLowV),
                Scalar(iHighH, iHighS, iHighV),
                thresh_frame);

        if (iOpen > 0)
        {
            morphologyEx(thresh_frame, thresh_frame, MORPH_OPEN,
                         getStructuringElement(MORPH_ELLIPSE, Size(iOpen, iOpen)));
        }

        if (iClose > 0)
        {
            morphologyEx(thresh_frame, thresh_frame, MORPH_CLOSE,
                         getStructuringElement(MORPH_ELLIPSE, Size(iClose, iClose)));
        }

        // Find contours
        std::vector<std::vector<Point>> contours;
        findContours(thresh_frame, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // Find largest contour
        double largestArea = 0;
        int largestIndex = -1;

        for (size_t i = 0; i < contours.size(); ++i)
        {
            double area = contourArea(contours[i]);
            if (area > largestArea)
            {
                largestArea = area;
                largestIndex = static_cast<int>(i);
            }
        }

        if (largestIndex >= 0)
        {
            Moments m = moments(contours[largestIndex]);

            if (m.m00 > 0)
            {
                Point2f pos(m.m10 / m.m00, m.m01 / m.m00);

                // Draw object
                drawContours(frame, contours, largestIndex, Scalar(0, 255, 0), 2);
                circle(frame, pos, 5, Scalar(0, 0, 255), FILLED);

                auto now = Clock::now();
                double dt = std::chrono::duration<double>(now - prevTime).count();

                Point2f vel(0, 0);
                Point2f accel(0, 0);

                if (hasPrev && dt > 1e-6)
                {
                    // Raw velocity (pixels/s)
                    Point2f rawVel = (pos - prevPos) * (1.0 / dt);

                    // Simple low-pass filter to reduce jitter
                    vel = prevVel * 0.8f + rawVel * 0.2f;

                    // Acceleration (pixels/s^2)
                    accel = (vel - prevVel) * (1.0 / dt);

                    // Speeds
                    float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
                    float prevSpeed = std::sqrt(prevVel.x * prevVel.x + prevVel.y * prevVel.y);

                    // --- Bounce detection ---
                    bool yReversed = (vel.y * prevVel.y < 0);

                    // Angle between velocity vectors
                    float dot = vel.x * prevVel.x + vel.y * prevVel.y;
                    float cosTheta = 1.0f;

                    if (speed > 1 && prevSpeed > 1)
                    {
                        cosTheta = dot / (speed * prevSpeed);
                        cosTheta = std::max(-1.0f, std::min(1.0f, cosTheta));
                    }

                    float angleDeg = std::acos(cosTheta) * 180.0f / CV_PI;

                    // Trigger if either:
                    //   - vertical motion reverses, OR
                    //   - motion direction changes significantly
                    bool bounced = (speed > 50 && prevSpeed > 50) &&
                                   (yReversed || angleDeg > 20.0f);

                    if (bounced)
                    {
                        putText(frame, "BOUNCE DETECTED",
                                Point(10, 30),
                                FONT_HERSHEY_SIMPLEX,
                                0.8, Scalar(0, 0, 255), 2);
                    }

                    // Display velocity
                    putText(frame,
                            "Vx: " + std::to_string((int)vel.x) +
                                " Vy: " + std::to_string((int)-vel.y),
                            Point(10, 60),
                            FONT_HERSHEY_SIMPLEX,
                            0.6, Scalar(255, 255, 0), 2);

                    // Display acceleration
                    putText(frame,
                            "Ax: " + std::to_string((int)accel.x) +
                                " Ay: " + std::to_string((int)-accel.y),
                            Point(10, 90),
                            FONT_HERSHEY_SIMPLEX,
                            0.6, Scalar(0, 255, 255), 2);

                    // Display angle change
                    putText(frame,
                            "Angle: " + std::to_string((int)angleDeg) + " deg",
                            Point(10, 120),
                            FONT_HERSHEY_SIMPLEX,
                            0.6, Scalar(255, 200, 0), 2);
                }

                // Draw coordinates (origin at bottom-left)
                double display_y = frame.rows - pos.y;

                putText(frame,
                        "(" + std::to_string((int)pos.x) + ", " +
                            std::to_string((int)display_y) + ")",
                        Point(pos.x + 10, pos.y - 10),
                        FONT_HERSHEY_SIMPLEX,
                        0.5, Scalar(0, 255, 0), 2);

                // Update history
                prevPos = pos;
                prevVel = vel;
                prevTime = now;
                hasPrev = true;
            }
        }

        imshow("Thresholded", thresh_frame);
        imshow("Camera", frame);
        fpsFrames++;
        auto fpsNow = Clock::now();
        double elapsed = std::chrono::duration<double>(fpsNow - fpsStart).count();

        // if (elapsed >= 1.0)
        // {
        //     std::cout << "FPS: " << fpsFrames / elapsed << std::endl;
        //     fpsFrames = 0;
        //     fpsStart = fpsNow;
        // }
        // ESC to quit
        if (waitKey(1) == 27)
            break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}