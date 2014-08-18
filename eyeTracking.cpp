/**
 * eye-tracking.cpp:
 * Eye detection and tracking with OpenCV
 *
 * This program tries to detect and tracking the user's eye with webcam.
 * At startup, the program performs face detection followed by eye detection
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
 * @return zero=failed, nonzero=success
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


int EyeTracking::detectEye(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect) {

    std::vector<cv::Rect> faces, eyes;
    // detect largest face
    face_cascade.detectMultiScale(im, faces, 1.1, 2, 
                                  0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT,
                                  cv::Size(30, 30));

    if (faces.size() > 0) {      
        cv::Mat face = im(faces[0]);
//        std::cout << "face height" << faces[i].height << std::endl;

    //-- Find eye regions and draw them
        int eye_region_width = faces[0].width * (kEyePercentWidth/100.0);
        int eye_region_height = faces[0].width * (kEyePercentHeight/100.0);
        int eye_region_top = faces[0].height * (kEyePercentTop/100.0);
        
        cv::Rect leftEyeRegion(faces[0].width*(kEyePercentSide/100.0),
                               eye_region_top,eye_region_width,eye_region_height);
        rect = leftEyeRegion + cv::Point(faces[0].x, faces[0].y);
        tpl = face(leftEyeRegion);
    }

    return 1;
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

    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(im, faces, 1.1, 2, 
                                  0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT,
                                  cv::Size(30, 30));

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

// use camshift algorithm for tracking
// will track the neck
cv::Mat frame, hsv, hue, mask, hist, backproj;
cv::Rect trackWindow;
int hsize = 16;
float hranges[] = {0, 180};
const float *phranges = hranges;
cv::Mat histimg = cv::Mat::zeros(640, 480, CV_8UC3);
int vmin = 32, vmax = 256, smin = 60;

void EyeTracking::trackCamShift(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect) {
    if (im.empty() || rect.area() == 0) return ;
    
    cv::cvtColor(im, hsv, CV_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(0, smin, vmin),
                cv::Scalar(180, 256, vmax), mask);

    int ch[] = {0, 0};

    hue.create(hsv.size(), hsv.depth());
    cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);


    if (trackObject < 0) {
        cv::Mat roi(hue, rect), maskroi(mask, rect);
        cv::calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
        // change NORM_MINMAX to 32
        cv::normalize(hist, hist, 0, 255, 32);
        trackWindow = rect;
        trackObject = 1;
    }
   
    cv::calcBackProject(&hue, 1, 0, hist, backproj, &phranges);    
    backproj &= mask;
    cv::RotatedRect trackBox = cv::CamShift(backproj, trackWindow, 
                               cv::TermCriteria(cv::TermCriteria::EPS
                                  | cv::TermCriteria::COUNT, 10, 1));
     
    // trackobject

    if( trackWindow.area() <= 1 )  {
        int cols = backproj.cols, rows = backproj.rows, 
            r = (MIN(cols, rows) + 5)/6;
        trackWindow = cv::Rect(trackWindow.x - r, trackWindow.y - r,
                               trackWindow.x + r, trackWindow.y + r) &
            cv::Rect(0, 0, cols, rows);
    }

    cv::Rect faceBox = trackBox.boundingRect();
    
    cv::rectangle( im, faceBox, cv::Scalar(0,0,255));

    Global global = Global::getInstance();
    global.setEyeDepth(faceBox.height);
}

// Track eye feature with SurfFeatureDetector
// SurfFeatureDetector implementation
void EyeTracking::trackEyeFeature(cv::Mat& im, cv::Mat& tpl, cv::Rect& rect) {

    std::vector<cv::Rect> faces;
    cv::Size size(rect.width * 2, rect.height * 2);
    cv::Rect window(rect + size - cv::Point(size.width/2, size.height/2));

    window &= cv::Rect(0, 0, im.cols, im.rows);

    cv::Mat dst(window.width - tpl.rows + 1, 
                window.height - tpl.cols + 1, CV_32FC1);
    
    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 400;

    cv::SurfFeatureDetector detector( minHessian );

    std::vector<cv::KeyPoint> keypoints_1, keypoints_2;

    detector.detect(tpl , keypoints_1 );
    detector.detect(im, keypoints_2 );

    //-- Draw keypoints
    cv::Mat img_keypoints_1; cv::Mat img_keypoints_2;

    cv::drawKeypoints(tpl, keypoints_1, img_keypoints_1, 
                      cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT );
    cv::drawKeypoints(im, keypoints_2, img_keypoints_2,
                      cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT );

    //-- Show detected (drawn) keypoints
    imshow("Keypoints 1", img_keypoints_1 );
    imshow("Keypoints 2", img_keypoints_2 );

    cv::waitKey(5);

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

    // open the default webcam
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

        // flip the frame horizontally
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
            trackCamShift(frame, eye_tpl, eye_bb);
            // trackEyeFeature(gray, eye_tpl, eye_bb);

            // detectEye(gray, eye_tpl, eye_bb);
            
            // set eye position to change the view of the cube
            global.setPosition(eye_bb.x, eye_bb.y);
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
