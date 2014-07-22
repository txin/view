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
    sbmTuner.state->preFilterSize = compute(FILTER_SIZE);
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
    sbmTuner.state->numberOfDisparities = compute(NUM_DISPARITY);
}

void on_trackbar_texture(int, void*) {
    sbmTuner.state->textureThreshold = compute(TEXTURE) ;
}

void on_trackbar_unqiue_ratio(int, void*) {
    sbmTuner.state->uniquenessRatio = compute(UNIQUE_RATIO);
}

void on_trackbar_speckle_size(int, void*) {
    sbmTuner.state->speckleWindowSize = compute(SPECKLE_SIZE);
}

void on_trackbar_speckle_range(int, void*) {
    sbmTuner.state->speckleRange = compute(SPECKLE_RANGE);
}

// testing, create window
int main() {
    StereoView view;
    view.run();
    return 0;
}
