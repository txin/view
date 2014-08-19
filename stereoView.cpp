/*
 * Getting depth from 2 cameras (stereo vision system setup)
 * 1. cameras calibration
 * 2. get the fundamental matrix and essential matrix, disprity
 * 3. compute depth
 * reference: docs.opencv.org
 * disparity opencv-2.4.9/samples/cpp/tutorial_code/calib3d/stereoBM
 *                       /SBM_Sample.cpp
 */
// #include "StereoView.h"
#include "Global.h"

using namespace cv;

const int alpha_slider_max = 20;
int alpha_slider;
double alpha, beta;

// load calibration configuration files for the cameras
int StereoView::loadConfiguration() {

    // load calibration file for stereo camera pair
    cv::FileStorage fs1("intrinsics.yml", cv::FileStorage::READ);
    fs1["M1"] >> cameraMatrix[0];
    fs1["D1"] >> distCoeffs[0];
    fs1["M2"] >> cameraMatrix[1];
    fs1["D2"] >> distCoeffs[1];
    fs1.release();

    // load extrinsics file, R, T translation and rotation matrix of 2 cameras
    cv::FileStorage fs2("extrinsics.yml", cv::FileStorage::READ);
    fs2["R"] >> R;
    fs2["T"] >> T;
    fs2["R1"] >> R1;
    fs2["R2"] >> R2;
    fs2["P1"] >> P1;
    fs2["P2"] >> P2;
    fs2["Q"] >> Q;
    fs2.release();
    return 0;
}

// open cameras, currently 2 cameras
int StereoView::cameraSetup() {
    for (int i = 0; i < CAMERA_NUM; i++) {
        cameras[i].open(i + 1);
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

void StereoView::distortionRemoval(cv::Mat& view, cv::Mat& rview, 
                                   int camNo) {
    cv::Mat rmap[2];
    cv::Size imageSize(CAMERA_WIDTH, CAMERA_HEIGHT);

    if (view.empty()) {
        return ;
    }

    cv::initUndistortRectifyMap(cameraMatrix[camNo], 
                                distCoeffs[camNo], cv::Mat(),
                                getOptimalNewCameraMatrix(cameraMatrix[0], 
                                                          distCoeffs[0], 
                                                          imageSize,
                                                          1, imageSize, 0),
                                imageSize, CV_16SC2, rmap[0], rmap[1]);
    cv::remap(view, rview, rmap[0], rmap[1], CV_INTER_LINEAR);
}

// use rectifyStereo first and reprojectImage to 3D
void StereoView::computeDepth(cv::Mat& disparity) {
    
    if (disparity.empty()) {
        return ;
    }

    cv::Size imgSize(CAMERA_WIDTH, CAMERA_HEIGHT);
    cv::Mat img3D(disparity.size(), CV_32FC3);

    // convert the disparity map
    cv::Mat disparity32F;
    disparity.convertTo(disparity32F, CV_32F, 1./16);
    cv::Mat_<float> vec(4,1);
    cv::Mat Q_32F;
    Q.convertTo(Q_32F, CV_32F);
    // get the depthImg reference from the global class
    cv::reprojectImageTo3D(disparity32F, img3D, Q, false, CV_32F);
    // display the 3d image
    cv::imshow("img3D", img3D);
    cv::waitKey(5);
}

// capture images from 2 cameras, and convert to grayscale images and 
// show disparity map, in order to calculate depth
int StereoView::showDepthData(cv::Mat& imgLeft, cv::Mat& imgRight) {

    cv::Mat disp;
    cv::Mat disp8;
    const char *windowName = "Disparity";

    // apply the rectified images to get the disparity map
    sbm(imgLeft, imgRight, disp);

    // normalize disparity map to display
    cv::normalize(disp, disp8, 0, 255, CV_MINMAX, CV_8U);

    // check the faceRect whether is empty or not
    cv::imshow(windowName, disp8);
    
    // compute the 3d point from the disparity map, CV_16S format
    computeDepth(disp);
    return 0;
}

// show cameradata, RGB format captured and convert to grayscale
void StereoView::run() {

    cv::Mat frames[CAMERA_NUM];
    sbm_here = sbm;

    for (int i = 0; i < CAMERA_NUM; i++) {
        std::string windowName("Camera ");
        windowName += std::to_string(i);
        cv::namedWindow(windowName, CV_WINDOW_NORMAL);
        cv::moveWindow(windowName, CAMERA_WIDTH * i, 0);
    }

    cv::Mat imgLeft, imgRight;
    // TODO: change the camera Index
    while (cv::waitKey(15) != 'q') {

        cameras[0] >> frames[0];
        cameras[1] >> frames[1];
        if (frames[0].empty()) break;
        if (frames[1].empty()) break;
            
        // Flip the frame horizontally, Windows users might need this
        cv::flip(frames[0], frames[0], 1);
        cv::flip(frames[1], frames[1], 1);

        cv::cvtColor(frames[0], frames[0], CV_RGB2GRAY);
        cv::cvtColor(frames[1], frames[1], CV_RGB2GRAY);
    
        // apply the calibration parameters to the first matrix captured by cam 0
        cv::Mat rImgLeft, rImgRight; 
        distortionRemoval(frames[0], rImgLeft, 0);
        distortionRemoval(frames[1], rImgRight, 1);
        
        cv::imshow("Camera 0", rImgLeft);
        cv::imshow("Camera 1", rImgRight);
            
        eyeTracking[0].detectEye(rImgLeft);
        showDepthData(rImgLeft, rImgRight);
    }
    cameras[0].release();
    cameras[1].release();
}
