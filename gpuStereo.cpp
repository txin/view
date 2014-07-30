#include <iostream>
#include <string>

#include "opencv2/opencv.hpp"
#include "opencv2/gpu/gpu.hpp"

// apply the stereoBM_GPU method
int main(int argc, char* argv[]) {
    cv::gpu::StereoBM_GPU stereo;

    try {
        cv::Mat srcLeftImg = cv::imread("left.ppm", CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat srcRightImg = cv::imread("right.ppm", CV_LOAD_IMAGE_GRAYSCALE);

        cv::gpu::GpuMat dst, src;
        cv::gpu::GpuMat dstR, srcR;
        src.upload(srcLeftImg);
        srcR.upload(srcRightImg);

        cv::gpu::threshold(src, dst, 128.0, 255.0, CV_THRESH_BINARY);
        cv::gpu::threshold(srcR, dstR, 128.0, 255.0, CV_THRESH_BINARY);

        cv::Mat result_host = (cv::Mat)dst;
        cv::Mat resultHostRight = (cv::Mat)dstR;
        
        std::string windowNames[] = {"Left", "Right"};
        
        for (int i = 0; i < 2; i++) {
            cv::namedWindow(windowNames[i], CV_WINDOW_AUTOSIZE);
            cv::moveWindow(windowNames[i], 500 * i, 0);
        }

        cv::imshow("Left", result_host);
        cv::imshow("Right", resultHostRight);
        cv::waitKey();
    } catch(const cv::Exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return 0;
}
