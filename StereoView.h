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

  int cameraCalibrationSetup();
  int cameraSetup();

public:
  inline StereoView() {
    cameraSetup();
  };
  void showCameraData();
};
