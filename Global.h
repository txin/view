// singleton class
// share the location
#include <opencv2/imgproc/imgproc.hpp>
#include <pthread.h>

class Global {

private:

    cv::Point *position;

    pthread_mutex_t start;
    // eye position from EyeTracking class
    // share with Cube class to compute the dynamic view matrix
    Global(){
        pthread_mutex_init(&start, NULL);
        position = new cv::Point();
    };

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
};
