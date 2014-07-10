/*
*  Getting depth from 2 cameras (stereo vision system setup)
*  1. cameras calibration
*  2. get the fundamental matrix and essential matrix, disprity
*  3. compute depth
*/
#include "StereoView.h"

int StereoView::cameraSetup() {
  cv::VideoCapture cameraLeft(0);
  cv::VideoCapture cameraRight(1);
  if (!cameraLeft.isOpened() || !cameraRight.isOpened()) {
    std::cerr << "error: camera is not opened" << std::endl;
    return -1;
  } else {
    cv::namedWindow("test",1);
    cameraLeft.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    cameraLeft.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    while (cv::waitKey(15) != 'q') {
      cv::Mat frame;
      cameraLeft >> frame;
      if (frame.empty()) break;
      imshow("test", frame);
    }
    return 0;
  }
}

int main(int argc, char** argv) {
  StereoView view;
  return 0;
}

