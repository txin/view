/*
 * Getting depth from 2 cameras (stereo vision system setup)
 * 1. cameras calibration
 * 2. get the fundamental matrix and essential matrix, disprity
 * 3. compute depth
 * reference: docs.opencv.org
 * disparity opencv-2.4.9/samples/cpp/tutorial_code/calib3d/stereoBM
 *                       /SBM_Sample.cpp
 *
 */
#include "StereoView.h"

const int alpha_slider_max = 100;
int alpha_slider;
double alpha, beta;


// load calibration configuration file for the camera
// currently for camera 0
int StereoView::loadConfiguration() {
    cv::FileStorage fs("cam0_config.xml", cv::FileStorage::READ);

    fs["Camera_Matrix"] >> cameraMat[0];
    fs["Distortion_Coefficients"] >> distCoeffMat[0];
    fs.release();
}


// open cameras, currently 2 cameras
int StereoView::cameraSetup() {
    for (int i = 0; i < CAMERA_NUM; i++) {
        cameras[i].open(i);
        if (!cameras[i].isOpened()) {
            std::cerr << "error: camera"<< i <<" is not opened" << std::endl;
            return -1;
        } else {
            cameras[i].set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH);
            cameras[i].set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT);
        }
    }
    
    // load configuration files
    loadConfiguration();
    return 0;
}

cv::StereoBM sbm_here;
int display;
void on_trackbar(int, void* ) {
    alpha = (double) alpha_slider/alpha_slider_max ;
    beta = ( 1.0 - alpha ) * 10;
    int level = (int)beta + 1;
    display = 16 * level;
    sbm_here.state->numberOfDisparities = 16 * level ; // 112
}

// capture images from 2 cameras, and convert to grayscale images and 
// show disparity map, in order to calculate depth
int StereoView::showDepthData(cv::Mat& imgLeft, cv::Mat& imgRight) {
    
    imgLeft = cv::imread("c0_1.jpg", CV_LOAD_IMAGE_COLOR);
    imgRight = cv::imread("c0_2.jpg", CV_LOAD_IMAGE_COLOR);
    cv::cvtColor(imgLeft, imgLeft, CV_RGB2GRAY);
    cv::cvtColor(imgRight, imgRight, CV_RGB2GRAY);
    //imgLeft = cv::imread("left01.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    //imgRight = cv::imread("right01.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    // apply the calibration parameters to the first matrix captured by cam 0
    cv::Mat temp0 = imgLeft.clone();
    cv::undistort(temp0, imgLeft, cameraMat[0], distCoeffMat[0]);
    cv::imshow("Camera 0", imgLeft);
    cv::imshow("Undistorted_cam0", temp0);
    cv::imshow("Camera 1", imgRight);
    if ((char)cv::waitKey(5) == 'q') return 0;
    cv::Mat imgDisparity16S = cv::Mat(imgLeft.rows, imgLeft.cols, CV_16S);
    cv::Mat imgDisparity8U = cv::Mat(imgLeft.rows, imgLeft.cols, CV_8UC1);
    
    if (!imgLeft.data || !imgRight.data) {
        std::cout << "Error reading images" << std::endl;
        return -1;
    }

    int ndisparities = 16 * 5;
    int SADWindowSize = 21;
    
    // cv::StereoBM sbm(cv::StereoBM::BASIC_PRESET, ndisparities, SADWindowSize);    
    // sbm(imgLeft, imgRight, imgDisparity16S, CV_16S);
    
    // setup paremeters
    sbm.state->SADWindowSize = 87;
    sbm.state->numberOfDisparities = 80; // 112
    //sbm.state->blockSize=15;
   
    //sbm.state->preFilterSize = 9; // 5
    
    sbm.state->preFilterCap = 40; //6, 61
    sbm.state->minDisparity = 0; // -39
    sbm.state->textureThreshold = 0; // 507
    sbm.state->uniquenessRatio = 0;
    sbm.state->speckleWindowSize = 30;
    sbm.state->speckleRange = 29; // 8
    sbm.state->disp12MaxDiff = 0; // 1
    
    
    double minVal;
    double maxVal;
    
//    cv::minMaxLoc(imgDisparity16S, &minVal, &maxVal);
    
    //  imgDisparity16S.convertTo(imgDisparity8U, CV_8UC1, 255 / (maxVal - minVal));
    cv::Mat disp;
    cv::Mat disp8;
    const char *windowName = "Disparity";
    sbm(imgLeft, imgRight, disp);
    cv::normalize(disp, disp8, 0, 255, CV_MINMAX, CV_8U);
    cv::imshow(windowName, disp8);
    // cv::imshow(windowName, imgDisparity8U);
    // cv::imwrite("SBM_sample.png", imgDisparity16S);
    if ((char)cv::waitKey(5) == 'q') return 0;
    
    return 0;
}




// show cameradata, RGB format captured and convert to grayscale
void StereoView::showCameraData() {
    cv::Mat frames[CAMERA_NUM];
    cv::Mat grayFrames[CAMERA_NUM];
    sbm_here = sbm;
    for (int i = 0; i < CAMERA_NUM; i++) {
        std::string windowName("Camera ");
        windowName += std::to_string(i);
        cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);
        cv::moveWindow(windowName, CAMERA_WIDTH * i, 0);
    }
    cv::namedWindow("Undistorted_cam0", CV_WINDOW_AUTOSIZE);
    cv::moveWindow("Undistorted_cam0", 0, CAMERA_HEIGHT);
    
    const char *windowName = "Disparity";
    cv::namedWindow(windowName, CV_WINDOW_NORMAL);   
        // create trackbars
    cv::createTrackbar("num of disparities", "Disparity", &alpha_slider, 
                       alpha_slider_max, on_trackbar);
    on_trackbar(display, 0);
    // TODO: 2 threads to show camera data?
    while (cv::waitKey(15) != 'q') {
        cameras[0] >> frames[0];
        cameras[1] >> frames[1];
        if (frames[0].empty()) break;
        if (frames[1].empty()) break;
        showDepthData(frames[0], frames[1]);
    }
}

int main(int argc, char** argv) {
    StereoView view;
    view.showCameraData();
    return 0;
}
