#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <stdio.h>
#include <pthread.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif


class StereoCalibration {

private:
    int mode; // detection mode

public:
    StereoCalibration(){};
    int setup(int cameraNo);
};
