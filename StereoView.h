#pragma once

#include <iostream>
#include <string>

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "EyeTracking.h"

#define CAMERA_NUM 2
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480
#define EYETRACKING_NUM 2
#define DEFAULT_DEPTH 200

class StereoView {
private:
    cv::VideoCapture cameras[CAMERA_NUM];
    cv::StereoBM sbm;

    // calibration parameters for cameras;
    cv::Mat cameraMatrix[CAMERA_NUM];
    cv::Mat distCoeffs[CAMERA_NUM];

    // camera parameters
    cv::Mat R, T, R1, R2, P1, P2, Q;

    void distortionRemoval(cv::Mat& view, cv::Mat& rview, int camNo);
    // stereoRectify
    void computeDepth(cv::Mat& disparity);
    int cameraCalibrationSetup();
    int cameraSetup();
    int loadConfiguration();

public:
    // can track eye separately
    // eyeTracking objects for 2 cameras
    EyeTracking eyeTracking[2];
   
    inline StereoView() {          
        cameraSetup();

        for (int i = 0; i < EYETRACKING_NUM; i++) {
            eyeTracking[i].setUp();
            eyeTracking[i].index = i;
        }
        
        const char *windowName = "Disparity";
        cv::namedWindow(windowName, CV_WINDOW_NORMAL);
        cv::moveWindow(windowName, 0, CAMERA_HEIGHT + 50);
    };
    inline cv::StereoBM getSbm() {
        return sbm;
    }
    void run();
    int showDepthData(cv::Mat& imgLeft, cv::Mat& imgRight);
};
