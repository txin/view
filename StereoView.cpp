/*
 *  Getting depth from 2 cameras (stereo vision system setup)
 *  1. cameras calibration
 *  2. get the fundamental matrix and essential matrix, disprity
 *  3. compute depth
 */
#include "StereoView.h"

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

// show cameradata
void StereoView::showCameraData() {
    cv::Mat frames[CAMERA_NUM];

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
        imshow("Camera 0", frames[0]);
        imshow("Camera 1", frames[1]);
    }
}

int main(int argc, char** argv) {
    StereoView view;
    view.showCameraData();
    return 0;
}