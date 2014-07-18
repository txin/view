// singleton class
// share the location
#include <opencv2/imgproc/imgproc.hpp>

class Global {

private:
    cv::Point position;
    // eye position from EyeTracking class
    // share with Cube class to compute the dynamic view matrix
    
    Global(){
        // TODO: initialise the eye position
//        position.x = 
    };

public:
    static Global& getInstance() {
        static Global instance;
        return instance;
    }
    
    inline void setPosition(int xin, int yin) {
        position.x = xin;
        position.y = yin;
    }
    
    inline cv::Point getPosition() {
        return position;
    }
};


