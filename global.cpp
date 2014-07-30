#include "Global.h"

Global::Global() {
    pthread_mutex_init(&start, NULL);
    position = new cv::Point(CAMERA_WIDTH / 2, CAMERA_HEIGHT / 2);
        
    running = new bool;
    *running = true;
        
    // TODO: need to initialise? or just assign to the new position
    depthImg = new cv::Mat(cv::Size(CAMERA_WIDTH, CAMERA_HEIGHT), CV_32FC3);
    // TODO: camera mat type?
    rawImg = new cv::Mat;
};
    
// lock to update the position
void Global::setPosition(cv::Point posIn) {
    pthread_mutex_lock(&start);
    *position = posIn;
    pthread_mutex_unlock(&start);
}
    
void Global::setPosition(int x, int y) {
    position->x = x;
    position->y = y;
} 
    
cv::Point Global::getPosition() {
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
