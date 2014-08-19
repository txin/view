#include "Global.h"

Global::Global() {
    pthread_mutex_init(&start, NULL);
    // average position of the two eye
    position = new cv::Point(CAMERA_WIDTH / 2, CAMERA_HEIGHT / 2);
    running = new bool;
    *running = true;
    // initialise the depth pointer
    depth = new int;
    *depth = DEFAULT_DEPTH;
    faceRect = new cv::Rect;

    // set up eyeBox, initialise the matrices of the eyeBox
    // use the fixed length of the square
    // allocate space for eyeBoxes
    for (int i = 0; i < CAMERA_NUM; i++) {
        eyeBox[i] = new cv::Mat(cv::Size(EYEBOX_SIDE, EYEBOX_SIDE), CV_32FC3);
        eyePosition[i] = new cv::Point(CAMERA_WIDTH / 2, CAMERA_HEIGHT / 2);
    }
    
};
    
// lock to update the bounding box matrix for the eyes
// index refering to the index of the cameras
void Global::setEyeBox(int index, cv::Mat eyeMat) {
    pthread_mutex_lock(&start);
    eyeMat.copyTo(*eyeBox[index]);
    pthread_mutex_unlock(&start);
}

// get the eyeBox matrix
cv::Mat Global::getEyeBox(int index) {
    return *eyeBox[index];
}

// TODO: set face bounding box, rectangle
void Global::setFaceRect(cv::Rect rect) {
    pthread_mutex_lock(&start);
    *faceRect = rect;
    pthread_mutex_unlock(&start);
}

cv::Rect Global::getFaceRect() {
    return *faceRect;
}

// lock to update the position
void Global::setEyePosition(cv::Point posIn) {
    pthread_mutex_lock(&start);
    *position = posIn;
    pthread_mutex_unlock(&start);
}
  
cv::Point Global::getEyePosition() {
    return *position;
}

void Global::setRawImg(cv::Mat& raw) {
    *rawImg = raw.clone();
}

// TODO: test reference or copyTo ?
void Global::setDepthImg(cv::Mat& img3D) {
    // TODO: lock the thread??
    img3D.copyTo(*depthImg);
    // TODO: print the coordinates        
}
    
void Global::getDepthData(int row, int col) {
    if (depthImg != NULL) {
        //std::cout << depthImg->at<cv::Point3f>(row, col) << std::endl;
    }
}

bool Global::getRunningStatus() {
    return *running;
}

void Global::setRunningStatus(bool val) {
    pthread_mutex_lock(&start);
    *running = val;
    pthread_mutex_unlock(&start);
}
