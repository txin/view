/**
 * eyeTracking.cpp:
 *
 * This program tries to detect and tracking the user's eye with webcam.
 * the program performs face detection followed by eye detection
 * using OpenCV's built-in Haar cascade classifier. If the user's eye detected
 * successfully, an eye template is extracted. This template will be used in
 * the subsequent template matching for tracking the eye.
 * functions provided:
 *
 */

/**
 * Function to detect human face and the eyes from an image.
 *
 * @param  im    The source image
 * @param  tpl   Will be filled with the eye template, if detection success.
 * @param  rect  Will be filled with the bounding box of the eye
 */

#include "EyeTracking.h"
#include "StereoView.h"
#include "Cube.h"
#include "Global.h"

// size constants for eye position in face

const int kEyePercentTop = 25;
const int kEyePercentSide = 13;
const int kEyePercentHeight = 30;
const int kEyePercentWidth = 35;

// detectEye used in stereoView
// detectEye, detecting the face first, and then use parameters to 


int EyeTracking::detectEye(cv::Mat& frame) {
    Global global = Global::getInstance();
    if (eyeRect.width == 0 && eyeRect.height == 0) {
        extractEyeTemplate(frame, eyeTpl, eyeRect);
    } else {
        // Tracking stage with template matching
        trackEye(frame);
        // Use camShift algorithm to track the face, camShift can match
        // different size of face.
        trackCamShift(frame);
        // set eye position to change the view of the cube
        global.setEyePosition(cv::Point(eyeRect.x, eyeRect.y));
        cv::rectangle(frame, eyeRect, CV_RGB(0,255,0));
    } 
    return 0;
}

// Load the cascade classifiers
int EyeTracking::setUp() {
   face_cascade.load("res/haarcascade_frontalface_alt2.xml");
   eye_cascade.load("res/haarcascade_eye_tree_eyeglasses.xml");
    
   // Check if everything is ok
   if (face_cascade.empty() || eye_cascade.empty())
       return 1;
   
   return 0;
}

int EyeTracking::extractEyeTemplate(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect) {

    std::vector<cv::Rect> faces, eyes;
    // detect largest face
    face_cascade.detectMultiScale(im, faces, 1.1, 2, 
                                  0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT,
                                  cv::Size(30, 30));

    if (faces.size() > 0) {      
        cv::Mat face = im(faces[0]);
        cv::rectangle(im, faces[0], cv::Scalar(0,0,255));
    //-- Find eye regions and draw them
        int eye_region_width = faces[0].width * (kEyePercentWidth/100.0);
        int eye_region_height = faces[0].width * (kEyePercentHeight/100.0);
        int eye_region_top = faces[0].height * (kEyePercentTop/100.0);
        
        cv::Rect leftEyeRegion(faces[0].width*(kEyePercentSide/100.0),
                               eye_region_top,eye_region_width,eye_region_height);
        rect = leftEyeRegion + cv::Point(faces[0].x, faces[0].y);
        tpl = face(leftEyeRegion).clone();
    }
    return 1;
}

/**
 * Perform template matching to search the user's eye in the given image.
 *
 * @param   im    The source image
 */
// define default face height 200
void EyeTracking::trackEye(cv::Mat& im) {

    cv::Size size(eyeRect.width * 2, eyeRect.height * 2);
    cv::Rect window(eyeRect + size - cv::Point(size.width/2, size.height/2));
    window &= cv::Rect(0, 0, im.cols, im.rows);
    cv::Mat dst(window.width - eyeTpl.rows + 1, window.height - eyeTpl.cols + 1, CV_32FC1);

    // match the eye template
    cv::matchTemplate(im(window), eyeTpl, dst, CV_TM_SQDIFF_NORMED);
    double minval, maxval;
    cv::Point minloc, maxloc;
    cv::minMaxLoc(dst, &minval, &maxval, &minloc, &maxloc);
    if (minval <= 0.2) {
        eyeRect.x = window.x + minloc.x;
        eyeRect.y = window.y + minloc.y;
    }
    else
        eyeRect.x = eyeRect.y = eyeRect.width = eyeRect.height = 0;

}


// use camshift algorithm for tracking face, and set eyeDepth determined by
// the size of face 

cv::Mat frame, hsv, hue, mask, hist, backproj;
cv::Rect trackWindow;
int hsize = 16;
float hranges[] = {0, 180};
const float *phranges = hranges;
cv::Mat histimg = cv::Mat::zeros(640, 480, CV_8UC3);
int vmin = 32, vmax = 256, smin = 60;

void EyeTracking::trackCamShift(cv::Mat& im) {

    if (im.empty() || eyeRect.area() == 0) return ;
    cv::cvtColor(im, hsv, CV_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(0, smin, vmin),
                cv::Scalar(180, 256, vmax), mask);
    int ch[] = {0, 0};
    hue.create(hsv.size(), hsv.depth());
    cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);
    if (trackObject < 0) {
        cv::Mat roi(hue, eyeRect), maskroi(mask, eyeRect);
        cv::calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
        // change NORM_MINMAX to 32
        cv::normalize(hist, hist, 0, 255, 32);
        trackWindow = eyeRect;
        trackObject = 1;
    }
   
    cv::calcBackProject(&hue, 1, 0, hist, backproj, &phranges);    
    backproj &= mask;
    cv::RotatedRect trackBox = cv::CamShift(backproj, trackWindow, 
                               cv::TermCriteria(cv::TermCriteria::EPS
                                  | cv::TermCriteria::COUNT, 10, 1));
     
    // trackobject
    if (trackWindow.area() <= 1)  {
        int cols = backproj.cols, rows = backproj.rows, 
            r = (MIN(cols, rows) + 5)/6;
        trackWindow = cv::Rect(trackWindow.x - r, trackWindow.y - r,
                               trackWindow.x + r, trackWindow.y + r) &
            cv::Rect(0, 0, cols, rows);
    }
    cv::Rect faceBox = trackBox.boundingRect();
    cv::rectangle(im, faceBox, cv::Scalar(0,0,255));
    Global global = Global::getInstance();

    // setEyeDepth
    global.setEyeDepth(faceBox.height);
}
