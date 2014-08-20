/*
 * Getting depth from 2 cameras (stereo vision system setup)
 * 1. cameras calibration
 * 2. get the fundamental matrix and essential matrix, disparity
 * 3. compute depth
 * reference: docs.opencv.org
 * disparity opencv-2.4.9/samples/cpp/tutorial_code/calib3d/stereoBM
 *                       /SBM_Sample.cpp
 */
#include "Global.h"

using namespace cv;

const int alpha_slider_max = 20;
int alpha_slider;
double alpha, beta;

// load calibration configuration files for the cameras
int StereoView::loadConfiguration() {

    // load calibration file for stereo camera pair
    cv::FileStorage fs1("res/intrinsics.yml", cv::FileStorage::READ);
    fs1["M1"] >> cameraMatrix[0];
    fs1["D1"] >> distCoeffs[0];
    fs1["M2"] >> cameraMatrix[1];
    fs1["D2"] >> distCoeffs[1];
    fs1.release();

    // load extrinsics file, R, T translation and rotation matrix of 2 cameras
    cv::FileStorage fs2("res/extrinsics.yml", cv::FileStorage::READ);
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
    
    // rectified images
    cv::Mat rFrames[2];
    while (cv::waitKey(15) != 'q') {
        for (int i = 0; i < 2; i++) {
            cameras[i] >> frames[i];
            // TODO: break outer loop
            if (frames[i].empty()) {
                break;
            }
            cv::flip(frames[i], frames[i], 1);
            cv::cvtColor(frames[i], frames[i], CV_RGB2GRAY);
            // apply the calibration parameters to remove distortions
            distortionRemoval(frames[i], rFrames[i], i);
        }
        eyeTracking[0].detectEye(rFrames[0]);
        cv::imshow("Camera 0", rFrames[0]);
        cv::imshow("Camera 1", rFrames[1]);

        showDepthData(rFrames[0], rFrames[1]);
    }
    cameras[0].release();
    cameras[1].release();
}
