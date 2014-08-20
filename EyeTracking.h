#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"

class EyeTracking {
private:
    cv::CascadeClassifier face_cascade;
    cv::CascadeClassifier eye_cascade;
    int detectEye(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect);
    void trackEye(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect);
    void findEyes(cv::Mat frame_gray, cv::Rect face);
    void trackEyeFeature(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect);
    void trackCamShift(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect);
    int trackObject = -1;

public:
    EyeTracking(){};
    int run();
    int index;
    int detectEye(cv::Mat& frame);
    int setUp();
};
