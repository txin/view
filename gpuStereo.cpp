/*
 * GPU SBM implementation
 * stereo block matching algorithm to compute disparity map with 
 * openCV GPU module
 */
#include <iostream>
#include <string>

#include "opencv2/opencv.hpp"
#include "opencv2/gpu/gpu.hpp"

// apply the stereoBM_GPU method
int main(int argc, char* argv[]) {
    cv::gpu::StereoBM_GPU sbm;

    try {
        cv::Mat srcLeftImg = cv::imread("left.ppm", CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat srcRightImg = cv::imread("right.ppm", CV_LOAD_IMAGE_GRAYSCALE);
        cv::gpu::GpuMat dstL, srcL;
        cv::gpu::GpuMat dstR, srcR;
        srcL.upload(srcLeftImg);
        srcR.upload(srcRightImg);
        cv::gpu::GpuMat disp, disp8;

        //int nDisparities = 16 * 10;
        //int SADWindowSize = 51;
        //cv::gpu::StereoBM_GPU sbm(cv::gpu::StereoBM_GPU::BASIC_PRESET,
        //                          nDisparities, SADWindowSize);
        cv::gpu::StereoBM_GPU sbm;
        sbm(srcL, srcR, disp);
        cv::gpu::normalize(disp, disp8, 0, 255, CV_MINMAX, CV_8U);
        std::string windowNames[] = {"Left", "Right", "Disparity"};
        for (int i = 0; i < 3; i++) {
            cv::namedWindow(windowNames[i], CV_WINDOW_AUTOSIZE);
            cv::moveWindow(windowNames[i], 480 * i, 0);
        }
        cv::Mat resultDisp8 = (cv::Mat)disp8;
        cv::imshow(windowNames[2], resultDisp8);

        cv::imshow("Left", srcLeftImg);
        cv::imshow("Right", srcRightImg);
        cv::waitKey();
    } catch(const cv::Exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return 0;
}
