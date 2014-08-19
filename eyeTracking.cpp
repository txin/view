/**
 * eye-tracking.cpp:
 * Eye detection and tracking with OpenCV
 *
 * This program tries to detect and tracking the user's eye with webcam.
 * At startup, the program performs face detection followed by eye detection
 * using OpenCV's built-in Haar cascade classifier. If the user's eye detected
 * successfully, an eye template is extracted. This template will be used in
 * the subsequent template matching for tracking the eye.
 */

/**
 * Function to detect human face and the eyes from an image.
 *
 * @param  im    The source image
 * @param  tpl   Will be filled with the eye template, if detection success.
 * @param  rect  Will be filled with the bounding box of the eye
 * @return zero=failed, nonzero=success
 */

#include "EyeTracking.h"
#include "StereoView.h"
#include "Cube.h"
#include "Global.h"

// Size constants
const int kEyePercentTop = 25;
const int kEyePercentSide = 13;
const int kEyePercentHeight = 30;
const int kEyePercentWidth = 35;

int EyeTracking::detectEye(cv::Mat& frame) {
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(frame, faces, 1.1, 2, 
                                   0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT,
                                   cv::Size(30, 30));

    for (unsigned int i = 0; i < faces.size(); i++ ) {
        rectangle(frame, faces[i], 1234);
    }

    if (faces.size() > 0) {
        findEyes(frame, faces[0]);
    }
    return 0;
}

int EyeTracking::setUp() {
   // Load the cascade classifiers
   face_cascade.load("haarcascade_frontalface_alt2.xml");
   eye_cascade.load("haarcascade_eye_tree_eyeglasses.xml");
    
   // Check if everything is ok
   if (face_cascade.empty() || eye_cascade.empty())
       return 1;
   
   return 0;
}

void EyeTracking::findEyes(cv::Mat frame_gray, cv::Rect face) {

    cv::Mat faceROI = frame_gray(face);
    cv::Mat debugFace = faceROI;

    // extract leftEyeRegion, rightEyeRegion
    int eye_region_width = face.width * (kEyePercentWidth/100.0);
    int eye_region_height = face.width * (kEyePercentHeight/100.0);
    int eye_region_top = face.height * (kEyePercentTop/100.0);

    cv::Rect leftEyeRegion(face.width*(kEyePercentSide/100.0),
                           eye_region_top,eye_region_width,eye_region_height);

    cv::Rect rightEyeRegion(face.width - eye_region_width - 
                            face.width*(kEyePercentSide/100.0),
                            eye_region_top,eye_region_width,eye_region_height);
    
    // set up eyebox and faceRect in global class
    Global global = Global::getInstance();
    global.setFaceRect(face);
    global.setEyeBox(index, faceROI);
    global.setEyePosition(cv::Point(face.x, face.y));
}

int EyeTracking::detectEye(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect) {

    std::vector<cv::Rect> faces, eyes;
    face_cascade.detectMultiScale(im, faces, 1.1, 2, 
                                  0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT,
                                  cv::Size(30, 30));

    for (unsigned int i = 0; i < faces.size(); i++) {
        cv::Mat face = im(faces[i]);
        
        // find eye regions and draw them
        int eye_region_width = faces[i].width * (kEyePercentWidth/100.0);
        int eye_region_height = faces[i].width * (kEyePercentHeight/100.0);
        int eye_region_top = faces[i].height * (kEyePercentTop/100.0);

        cv::Rect leftEyeRegion(faces[i].width*(kEyePercentSide/100.0),
        eye_region_top,eye_region_width,eye_region_height);
        eye_cascade.detectMultiScale(face(leftEyeRegion), eyes, 
                                     1.1, 2, 0|CV_HAAR_SCALE_IMAGE, cv::Size(20, 20));

        if (eyes.size()) {
            rect = leftEyeRegion + cv::Point(faces[i].x, faces[i].y);
            //rect = faces[i] + cv::Point(faces[i].x, faces[i].y);
            //rect = eyes[0] + cv::Point(faces[i].x, faces[i].y);
            //tpl  = im(rect);
            tpl = face(leftEyeRegion);
        }
    }
    return eyes.size();
}

/**
 * Perform template matching to search the user's eye in the given image.
 *
 * @param   im    The source image
 * @param   tpl   The eye template
 * @param   rect  The eye bounding box, will be updated with the new location of the eye
 */
// define default face height 200
void EyeTracking::trackEye(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect) {

    cv::Size size(rect.width * 2, rect.height * 2);
    cv::Rect window(rect + size - cv::Point(size.width/2, size.height/2));
    window &= cv::Rect(0, 0, im.cols, im.rows);
    cv::Mat dst(window.width - tpl.rows + 1, window.height - tpl.cols + 1, CV_32FC1);
    // match the eye template
    cv::matchTemplate(im(window), tpl, dst, CV_TM_SQDIFF_NORMED);
    double minval, maxval;
    cv::Point minloc, maxloc;
    cv::minMaxLoc(dst, &minval, &maxval, &minloc, &maxloc);
    if (minval <= 0.2) {
        rect.x = window.x + minloc.x;
        rect.y = window.y + minloc.y;
    }
    else
        rect.x = rect.y = rect.width = rect.height = 0;
}

int EyeTracking::run() {
    // Load the cascade classifiers
    face_cascade.load("haarcascade_frontalface_alt2.xml");
    eye_cascade.load("haarcascade_eye_tree_eyeglasses.xml");
    Global global = Global::getInstance();

    // open the first, default web cam
    cv::VideoCapture cap(0);

    const char *windowName = "EyeTracking";
    cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);
    cv::moveWindow(windowName, CAMERA_WIDTH, 0);

    // Check if everything is ok
    if (face_cascade.empty() || eye_cascade.empty() || !cap.isOpened())
        return 1;

    // Set video to 640 * 480
    cap.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT);

    cv::Mat frame, eye_tpl;
    cv::Rect eye_bb;
    char k;
    bool runningStatus = global.getRunningStatus();
    while ((k = cv::waitKey(15)) && (k != 'q') && (k != 27) 
           && runningStatus != false) {
        cap >> frame;
        if (frame.empty())
            break;
        // Flip the frame horizontally, Windows users might need this
        cv::flip(frame, frame, 1);
        // Convert to grayscale and
        // adjust the image contrast using histogram equaligzation
        cv::Mat gray;
        cv::cvtColor(frame, gray, CV_BGR2GRAY);
        if (eye_bb.width == 0 && eye_bb.height == 0) {
            // Detection stage
            // Try to detect the face and the eye of the user
            detectEye(gray, eye_tpl, eye_bb);
        } else {
            // Tracking stage with template matching
            trackEye(gray, eye_tpl, eye_bb);
            // detectEye(gray, eye_tpl, eye_bb);
            // set eye position to change the view of the cube
            global.setEyePosition(cv::Point(eye_bb.x, eye_bb.y));
            // get the corresponding depth data from global
            global.getDepthData(eye_bb.x, eye_bb.y);
            // Draw bounding rectangle for the eye
            cv::rectangle(frame, eye_bb, CV_RGB(0,255,0));
        }
        // display window
        cv::imshow(windowName, frame);
        runningStatus = global.getRunningStatus();
    }
    std::cout << "eyeTracking ends" << std::endl;
    global.setRunningStatus(false);
    return 0;
}
