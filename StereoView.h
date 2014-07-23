#pragma once

#include <iostream>
#include <string>

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#define CAMERA_NUM 2
#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480

class StereoView {
private:
    cv::VideoCapture cameras[CAMERA_NUM];
    cv::StereoBM sbm;

    // calibration parameters for cameras;
    cv::Mat cameraMat[CAMERA_NUM];
    cv::Mat distCoeffMat[CAMERA_NUM];

    void distortionRemoval(cv::Mat& view, cv::Mat& rview);
    // stereoRectify
    void computeDepth(cv::Mat& disparity);
    
    int cameraCalibrationSetup();
    int cameraSetup();
    int loadConfiguration();

public:
    inline StereoView() {          
        cameraSetup();
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
