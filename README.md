view
====

change view depending on the eye locations

## Running
Main: main program for eyetracking and disparity map 
make main
StereoCalibration: calibration program for stereo cameras
make stereoCalibration
Tuner: SBM tuner for disparity map
make sbmTuner
gpuStereo: SBM with GPU module
make gpuStero
calibration: calibration for single camera
make calibration
./calibration.out 1
// calibrate camera index 1, default camera 0

### StereoCalibration
calibration for customized stereo cameras pair,
save the matrices cameraMatrices and distcoeffs as intrinsics.yml 
and R, T, P1, P2, R1, R2 in the extrinsics.yml


### StereoView
using stereo block maching algorithm to compute the disparity map from 
the 2 calibrated stereo cameras pair, and use reprojectTo3D to get the 
corresponding 3D coordinates

### EyeTracking
Using harcarscade classifying method for eye tracking


### Tuner
SBM tuner, adjust the slider to find the optimum parameters of the SBM function


### Global
Global class provides the parameters and storage for global use

## Profiler
Using gprofiler for profiling the memory usage of the program


By Tianxin Tang
University of Bristol
Contact: titian.tang@gmail.com