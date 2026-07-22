#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;

int main()
{
    // Open the video camera.
    std::string pipeline = "libcamerasrc"
                           " ! video/x-raw, width=800, height=600" // camera needs to capture at a higher resolution
                           " ! videoconvert"
                           " ! videoscale"
                           " ! video/x-raw, width=400, height=300" // can downsample the image after capturing
                           " ! videoflip method=rotate-180"        // remove this line if the image is upside-down
                           " ! appsink drop=true max_buffers=2";
    VideoCapture cap(pipeline, CAP_GSTREAMER);
    if (!cap.isOpened())
    {
        printf("Could not open camera.\n");
        return 1;
    }

    // Create the OpenCV window
    namedWindow("Camera", WINDOW_AUTOSIZE);
    Mat frame;
    Mat thresh_frame;
    Mat hsv_frame;

    // Measure the frame rate - initialise variables
    int frame_id = 0;
    timeval start, end;
    gettimeofday(&start, NULL);

    // Create a control window
    namedWindow("Control", WINDOW_AUTOSIZE);
    int iLowH = 0;
    int iHighH = 179;
    int iLowS = 0;
    int iHighS = 255;
    int iLowV = 0;
    int iHighV = 255;
    int iOpen = 1;
    int iClose = 1;
    // Create trackbars in "Control" window
    createTrackbar("LowH", "Control", &iLowH, 179); // Hue (0 - 179)
    createTrackbar("HighH", "Control", &iHighH, 179);
    createTrackbar("LowS", "Control", &iLowS, 255); // Saturation (0 - 255)
    createTrackbar("HighS", "Control", &iHighS, 255);
    createTrackbar("LowV", "Control", &iLowV, 255); // Value (0 - 255)
    createTrackbar("HighV", "Control", &iHighV, 255);
    createTrackbar("Open", "Control", &iOpen, 10);
    createTrackbar("Close", "Control", &iClose, 10);

    namedWindow("Thresholded", WINDOW_AUTOSIZE);

    for (;;)
    {
        if (!cap.read(frame))
        {
            printf("Could not read a frame.\n");
            break;
        }

        cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

        // Threshold the image
        inRange(hsv_frame, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), thresh_frame);
        if (iOpen != 0)
        {
            morphologyEx(thresh_frame, thresh_frame, MORPH_OPEN, getStructuringElement(MORPH_ELLIPSE, Size(iOpen, iOpen)));
        }
        if (iClose != 0)
        {
            morphologyEx(thresh_frame, thresh_frame, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(iClose, iClose)));
        }
        // Find the contours
        std::vector<std::vector<Point>> contours;
        findContours(thresh_frame, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // Find the largest contour
        double largestArea = 0;
        int largestIndex = -1;

        for (size_t i = 0; i < contours.size(); i++)
        {
            double area = contourArea(contours[i]);

            if (area > largestArea)
            {
                largestArea = area;
                largestIndex = i;
            }
        }

        if (largestIndex >= 0)
        {
            // Calculate moments of the largest contour
            Moments m = moments(contours[largestIndex]);

            if (m.m00 > 0)
            {
                // Calculate centroid
                double x = m.m10 / m.m00;
                double y = m.m01 / m.m00;

                // Draw the object's actual outline
                drawContours(frame, contours, largestIndex, Scalar(0, 255, 0), 2);

                // Draw centroid
                circle(frame, Point(x, y), 5, Scalar(0, 0, 255), FILLED);

                // Convert y-coordinate so origin is at the bottom
                double display_y = frame.rows - y;

                // Draw coordinates
                putText(frame,
                        "(" + std::to_string((int)x) + ", " + std::to_string((int)display_y) + ")",
                        Point(x + 10, y - 10),
                        FONT_HERSHEY_SIMPLEX,
                        0.5,
                        Scalar(0, 255, 0),
                        2);
            }
        }

        imshow("Thresholded", thresh_frame);
        // show frame
        imshow("Camera", frame);
        waitKey(1);

        // Measure the frame rate
        frame_id++;
        if (frame_id >= 30)
        {
            gettimeofday(&end, NULL);
            double diff = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1000000.0;
            printf("30 frames in %f seconds = %f FPS\n", diff, 30 / diff);
            frame_id = 0;
            gettimeofday(&start, NULL);
        }
    }

    // Free the camera
    cap.release();
    return 0;
}
