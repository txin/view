/*
 * Getting depth from 2 cameras (stereo vision system setup)
 * 1. cameras calibration
 * 2. get the fundamental matrix and essential matrix, disprity
 * 3. compute depth
 * reference: docs.opencv.org
 * disparity opencv-2.4.9/samples/cpp/tutorial_code/calib3d/stereoBM
 *                       /SBM_Sample.cpp
 *
 */
#include "StereoView.h"

// open cameras, currently 2 cameras
int StereoView::cameraSetup() {
    for (int i = 0; i < CAMERA_NUM; i++) {
        cameras[i].open(i);
        if (!cameras[i].isOpened()) {
            std::cerr << "error: camera"<< i <<" is not opened" << std::endl;
            return -1;
        } else {
            cameras[i].set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH);
            cameras[i].set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT);
        }
    }
    return 0;
}

// capture images from 2 cameras, and convert to grayscale images and 
// show disparity map, in order to calculate depth
int StereoView::showDepthData(cv::Mat& imgLeft, cv::Mat& imgRight) {
    
    cv::cvtColor(imgLeft, imgLeft, CV_RGB2GRAY);
    cv::cvtColor(imgRight, imgRight, CV_RGB2GRAY);
    cv::imshow("Camera 0", imgLeft);
    cv::imshow("Camera 1", imgRight);
    cv::waitKey(5);
    cv::Mat imgDisparity16S = cv::Mat(imgLeft.rows, imgLeft.cols, CV_16S);
    cv::Mat imgDisparity8U = cv::Mat(imgLeft.rows, imgLeft.cols, CV_8UC1);
    
    if (!imgLeft.data || !imgRight.data) {
        std::cout << "Error reading images" << std::endl;
        return -1;
    }

    int ndisparities = 16 * 5;
    int SADWindowSize = 21;
    
    cv::StereoBM sbm(cv::StereoBM::BASIC_PRESET, ndisparities, SADWindowSize);
    
    sbm(imgLeft, imgRight, imgDisparity16S, CV_16S);
    
    double minVal;
    double maxVal;
    
    cv::minMaxLoc(imgDisparity16S, &minVal, &maxVal);
    
    imgDisparity16S.convertTo(imgDisparity8U, CV_8UC1, 255 / (maxVal - minVal));
    
    const char *windowName = "Disparity";
    cv::namedWindow(windowName, CV_WINDOW_NORMAL);
    cv::imshow(windowName, imgDisparity8U);

    // cv::imwrite("SBM_sample.png", imgDisparity16S);
    cv::waitKey(5);
    
    return 0;
}

// show cameradata, RGB format captured and convert to grayscale
void StereoView::showCameraData() {
    cv::Mat frames[CAMERA_NUM];
    cv::Mat grayFrames[CAMERA_NUM];

    for (int i = 0; i < CAMERA_NUM; i++) {
        std::string windowName("Camera ");
        windowName += std::to_string(i);
        cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);
        cv::moveWindow(windowName, CAMERA_WIDTH * i, 0);
    }
  
    // TODO: 2 threads to show camera data?
    while (cv::waitKey(15) != 'q') {
        cameras[0] >> frames[0];
        cameras[1] >> frames[1];
        if (frames[0].empty()) break;
        if (frames[1].empty()) break;
        showDepthData(frames[0], frames[1]);
    }
}

int main(int argc, char** argv) {
    StereoView view;
    view.showCameraData();
    return 0;
}
