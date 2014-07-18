#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

class EyeTracking {
private:
    cv::CascadeClassifier face_cascade;
    cv::CascadeClassifier eye_cascade;
    int detectEye(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect);
    void trackEye(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect);

public:
    EyeTracking(){};
    int run();

};
