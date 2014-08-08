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
    void trackEyeFeature(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect);
    
public:
    EyeTracking(){};
    int run();

};
