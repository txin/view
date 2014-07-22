#include "Tuner.h"
#include "StereoView.h"

// add more trackerbars to get the best result.
#define TYPE_SIZE 9

enum {FILTER_SIZE, FILTER_CAP, SAD_SIZE, MIN_DISPARITY, NUM_DISPARITY,
      TEXTURE, UNIQUE_RATIO, SPECKLE_SIZE, SPECKLE_RANGE};

int slider[9];
int slider_max = 100;

cv::StereoBM sbmTuner;

int compute(int i) {
    float alpha = (float)slider[i] / slider_max;
    float beta = (1.0 - alpha) * 10;
    return (int)beta + 1;
}

int setSBM(cv::StereoBM sbm) {
    sbmTuner = sbm;
}

void on_trackbar_filter_size(int, void*) {
    int filter_size = compute(FILTER_SIZE);
    filter_size += 4;
    if (filter_size % 2 == 0) {
        filter_size += 1;
    }
    sbmTuner.state->preFilterSize = filter_size;
}

void on_trackbar_filter_cap(int, void*) {
    sbmTuner.state->preFilterCap = compute(FILTER_CAP);
}

void on_trackbar_SAD_size(int, void*) {
    sbmTuner.state->SADWindowSize = compute(SAD_SIZE);
}

void on_trackbar_min_disparity(int, void*) {
    sbmTuner.state->minDisparity = compute(MIN_DISPARITY);
}

void on_trackbar_num_disparity(int, void*) {
    sbmTuner.state->numberOfDisparities = 16 * compute(NUM_DISPARITY);
}

void on_trackbar_texture(int, void*) {
    sbmTuner.state->textureThreshold = compute(TEXTURE) ;
}

void on_trackbar_unique_ratio(int, void*) {
    sbmTuner.state->uniquenessRatio = compute(UNIQUE_RATIO);
}

void on_trackbar_speckle_size(int, void*) {
    sbmTuner.state->speckleWindowSize = compute(SPECKLE_SIZE);
}

void on_trackbar_speckle_range(int, void*) {
    sbmTuner.state->speckleRange = compute(SPECKLE_RANGE);
}

void createSliders() {
    
    const char *windowName = "Disparity";
    cv::createTrackbar("filter_size", windowName, &slider[0], 
                       slider_max, on_trackbar_filter_size);

    cv::createTrackbar("filter_cap", windowName, &slider[1], 
                       slider_max, on_trackbar_filter_cap);

    cv::createTrackbar("filter_SAD_size", windowName, &slider[2], 
                       slider_max, on_trackbar_filter_size);

    cv::createTrackbar("min_disparity", windowName, &slider[3], 
                       slider_max, on_trackbar_min_disparity);

    cv::createTrackbar("num_disparity", windowName, &slider[4], 
                       slider_max, on_trackbar_num_disparity);

    cv::createTrackbar("texture", windowName, &slider[5], 
                       slider_max, on_trackbar_texture);

    cv::createTrackbar("unique_ratio", windowName, &slider[6], 
                       slider_max, on_trackbar_unique_ratio);

    cv::createTrackbar("speckle_size", windowName, &slider[7], 
                       slider_max, on_trackbar_speckle_size);

    cv::createTrackbar("speckle_range", windowName, &slider[8], 
                       slider_max, on_trackbar_speckle_range);
}

// testing, create window
int main() {
    StereoView view;
    setSBM(view.getSbm());
    createSliders();
    view.run();
    return 0;
}
