// singleton class
// share the location
#include <opencv2/imgproc/imgproc.hpp>
#include <pthread.h>
#include "StereoView.h"

class Global {

private:
    
    // eye position from EyeTracking class
    cv::Point *position;
    // depth image generated from StereoView class
    cv::Mat *depthImg;
    
    // raw image from the camera, default use the cam0
    cv::Mat *rawImg;

    pthread_mutex_t start;
    // eye position from EyeTracking class
    // share with Cube class to compute the dynamic view matrix
    Global() {
        pthread_mutex_init(&start, NULL);
        position = new cv::Point(CAMERA_WIDTH / 2, CAMERA_HEIGHT / 2);
        
        // TODO: need to initialise? or just assign to the new position
        depthImg = new cv::Mat(cv::Size(CAMERA_WIDTH, CAMERA_HEIGHT), CV_32FC3);
    };

    // TODO: delete the space generated by new
public:
    static Global& getInstance() {
        static Global instance;
        return instance;        
    }
    
    // lock to update the position
    inline void setPosition(cv::Point posIn) {
        pthread_mutex_lock(&start);
        *position = posIn;
        pthread_mutex_unlock(&start);
    }
    
    inline void setPosition(int x, int y) {
        position->x = x;
        position->y = y;
    } 
    
    inline cv::Point getPosition() {
        return *position;
    }

    inline void setRawImg(cv::Mat& raw) {
        *rawImg = raw;
    }

    // TODO: test reference or copyTo ?
    inline void setDepthImg(cv::Mat& img3D) {
        // TODO: lock the thread??
        img3D.copyTo(*depthImg);
        // TODO: print the coordinates        
    }
    
    inline void getDepthData(int row, int col) {
        if (depthImg != NULL) {
            std::cout << depthImg->at<cv::Point3f>(row, col) << std::endl;
        }
    }
};
