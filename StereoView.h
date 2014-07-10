#include <iostream>
#include "opencv2/opencv.hpp"

class StereoView {
private:
  int cameraSetup();

public:
  inline StereoView() {
    cameraSetup();
  };
};
