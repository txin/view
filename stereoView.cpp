/*
 * Getting depth from 2 cameras (stereo vision system setup)
 * 1. cameras calibration
 * 2. get the fundamental matrix and essential matrix, disprity
 * 3. compute depth
 * reference: docs.opencv.org
 * disparity opencv-2.4.9/samples/cpp/tutorial_code/calib3d/stereoBM
 *                       /SBM_Sample.cpp
 */
#include "StereoView.h"
#include "Global.h"

const int alpha_slider_max = 20;
int alpha_slider;
double alpha, beta;

// load calibration configuration files for the cameras
int StereoView::loadConfiguration() {
    for (int i = 0; i < CAMERA_NUM; i++) {
        std::string indexStr = std::to_string(i);
        std::string fileName = "Camera" + indexStr + "config.xml";
        cv::FileStorage fs("Camera0config.xml", cv::FileStorage::READ);
        fs["Camera_Matrix"] >> cameraMat[i];
        fs["Distortion_Coefficients"] >> distCoeffMat[i];
        fs.release();
    }
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

void StereoView:: distortionRemoval(cv::Mat& view, cv::Mat& rview) {
    cv::Mat map1, map2;
    cv::Size imageSize(CAMERA_WIDTH, CAMERA_HEIGHT);
    cv::initUndistortRectifyMap(cameraMat[0], distCoeffMat[0], cv::Mat(),
                                getOptimalNewCameraMatrix(cameraMat[0], 
                                                          distCoeffMat[0], 
                                                          imageSize,
                                                          1, imageSize, 0),
                                imageSize, CV_16SC2, map1, map2);
    cv::remap(view, rview, map1, map2, CV_INTER_LINEAR);
    // cv::imshow(windowName, rview);
    //char c = cv::waitKey(5);
}

// use rectifyStereo first and reprojectImage to 3D
void StereoView::computeDepth(cv::Mat& disparity) {
    
    // TODO: change the values of the camera
    cv::Mat_<double> R(3,3); // 3x3 matrix, rotation left to right camera
    cv::Mat_<double> T(3,1); // * 3 * x1 matrix, translation left to 
    //right proj. center
    
    // output matrices
    cv::Mat R1; // 3x3 matrix
    cv::Mat R2; // 3x3 matrix
    cv::Mat P1; // 3x4 matrix
    cv::Mat P2; // 3x4 matrix
    cv::Mat Q;  // 4x4 matrix

    cv::Size imgSize(CAMERA_WIDTH, CAMERA_HEIGHT);
    cv::stereoRectify(cameraMat[0], distCoeffMat[0],cameraMat[1],
                      distCoeffMat[1], imgSize, R, T, R1, R2, P1, P2, Q);

    cv::Mat img3D(disparity.size(), CV_32FC3);

    // TODO: get the depthImg reference from the global class
    cv::reprojectImageTo3D(disparity, img3D, Q, false, CV_32F);
    
    Global global = Global::getInstance();
    global.setDepthImg(img3D);
}

// capture images from 2 cameras, and convert to grayscale images and 
// show disparity map, in order to calculate depth

bool debug_readimg = true;

// TODO: change to the camera reading
int StereoView::showDepthData(cv::Mat& imgLeft, cv::Mat& imgRight) {
   
    if (!debug_readimg) {
        cv::cvtColor(imgLeft, imgLeft, CV_RGB2GRAY);
        cv::cvtColor(imgRight, imgRight, CV_RGB2GRAY);
    }

    // apply the calibration parameters to the first matrix captured by cam 0
    cv::Mat rImgLeft, rImgRight; 
    distortionRemoval(imgLeft, rImgLeft);
    distortionRemoval(imgRight, rImgRight);

    // set global
    Global global = Global::getInstance();
    global.setRawImg(rImgLeft);

    cv::imshow("Camera 0", imgLeft);
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
   
    //setup paremeters
    //sbm.state->SADWindowSize = 11;
    //sbm.state->numberOfDisparities = 32; // 112
    //sbm.state->blockSize=15;
    //sbm.state->preFilterSize = 31; // 5
    //sbm.state->preFilterCap = 31; //6, 61
    //sbm.state->minDisparity = 3; // -39
    //sbm.state->textureThreshold = 0; // 507
    //sbm.state->uniquenessRatio = 0;
    //sbm.state->speckleWindowSize = 10;
    //sbm.state->speckleRange = 10; // 8
    //sbm.state->disp12MaxDiff = 0; // 1
    
    
    double minVal;
    double maxVal;
    
//cv::minMaxLoc(imgDisparity16S, &minVal, &maxVal);
    //  imgDisparity16S.convertTo(imgDisparity8U, CV_8UC1, 255 / (maxVal - minVal));
  
    cv::Mat disp;
    cv::Mat disp8;
    const char *windowName = "Disparity";

    // apply the rectified images to get the disparity map
    if (!debug_readimg) {
        sbm(rImgLeft, rImgRight, disp);
    } else {
        sbm(imgLeft, imgRight, disp);
    }
    
    cv::normalize(disp, disp8, 0, 255, CV_MINMAX, CV_8U);
    cv::imshow(windowName, disp8);

    // TODO: depth format?
    computeDepth(disp8);

    if ((char)cv::waitKey(5) == 'q') return 0;
    return 0;
}

// show cameradata, RGB format captured and convert to grayscale
void StereoView::run() {


    cv::Mat frames[CAMERA_NUM];
    cv::Mat grayFrames[CAMERA_NUM];
    sbm_here = sbm;
    for (int i = 0; i < CAMERA_NUM; i++) {
        std::string windowName("Camera ");
        windowName += std::to_string(i);
        cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);
        cv::moveWindow(windowName, CAMERA_WIDTH * i, 0);
    }

    cv::Mat imgLeft, imgRight;
//    imgLeft = cv::imread("left1.jpg", CV_LOAD_IMAGE_GRAYSCALE);
//    imgRight = cv::imread("right1.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    
    // sample image
    imgLeft = cv::imread("left.ppm", CV_LOAD_IMAGE_GRAYSCALE);
    imgRight = cv::imread("right.ppm", CV_LOAD_IMAGE_GRAYSCALE);


    // TODO: 2 threads to show camera data?

    while (cv::waitKey(15) != 'q') {
        if (debug_readimg) {
            showDepthData(imgLeft, imgRight);
        } else {
            cameras[0] >> frames[0];
            cameras[1] >> frames[1];
            if (frames[0].empty()) break;
            if (frames[1].empty()) break;
            showDepthData(frames[0], frames[1]);
        }
    }
}

/*int main(int argc, char** argv) {
  StereoView view;
  view.run();
  return 0;
  }*/
