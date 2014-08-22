view
====

change view depending on the eye locations

### Be aware
1. the index of the cameras. the program uses 1, 2 for 2 USB cameras.
0 is for default integrated web cam.

## Compiling
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

### Running
./main.out
./sbmTuner.out
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


## Implementation
1. calibration
   2 calibration methods are provided.
   input: configuration file (default.xml)
   output: Cam0Config.xml (calibration.cpp) extrinsics.yml, intrinsics.yml(stereoCalibration.cpp)
        
    calibration.cpp only calibrate one single camera.   
        stereoCalibration.cpp calibrate a stereo camera pair.
        Note: stereoCalibration return the found pattern once both camera find the same pattern.
        The condition is stricter than the single calibration, may needs more time.


2. eye tracking
Use openCV hararcsde classifier to detect face. (Only detect the largest face, with flag enabled) 
Once user face detected, eye template is extracted. Then use trackEye function to do template matching
on the current frame.
3. eye depth
Two ways of measure eye depth have been implemented.
(1) Using CamShift algorithm to track hue image of the face detected. Then set the height of the bounding
box of the tracked area as the depth. 
(2) Using stereo blocking matching algorithm to compute disparity map. With the camera matrices known from 
the calibration stage 
4. cube
Use openGL to display a 3D cube to present the eye position change.


## Limitation
1. disparity map only works for a certain distance range.
   currently 30 cm upwards.
2. disparity map accuracy mainly determines on the locations of two cameras.
   to improve quality, two cameras must be parallel, and need to get calibrated.
3. eye tracking
   camshift algorithm 

By Tianxin Tang
University of Bristol
Contact: titian.tang@gmail.com