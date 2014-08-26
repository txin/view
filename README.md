view
====
Eye location based view changing

### Be aware
1. The index of the cameras. The program uses 1, 2 for 2 USB cameras.
0 is for default integrated web cam.
2. required library (library used)
   openCV 2.4.9
   glfw 3.0
   glm 0.9.5.4
   openGL 4.0
3. May need configue the library path and Makefile
4. tested under Ubuntu 14.04

## Compiling & Running
Main: main program runs eyetracking, steroview, and cube. 
make main
./main.out

calibration: calibration for single camera
make calibration
./calibration.out 1 // calibrate camera index 1, default camera 0

stereoCalibration: calibration program for one camera pair.
make stereoCalibration 
./stereoCalibration

tuner: SBM tuner for disparity map with 2 default images
make sbmTuner
./sbmTuner.out

gpuStereo: SBM algorithm implemented with GPU module
make gpuStero
./gpuStereo.out

### stereoCalibration.cpp
Calibration for customized stereo cameras pair,
save the matrices cameraMatrices and distcoeffs as intrinsics.yml 
and R, T, P1, P2, R1, R2 in the extrinsics.yml

### stereoView.cpp
Using stereo block maching algorithm to compute the disparity map from 
the 2 calibrated stereo cameras pair, and use reprojectTo3D to get the 
corresponding 3D coordinates.

### eyeTracking.cpp
Using haarcascade classifying method for eye tracking and 
CamShift algorithm is provided to track face size.

### tuner.cpp
SBM tuner, adjust the slider to find the optimum parameters of the SBM class.

### global.cpp
Global class provides the parameters and storage for global use.

## Profiler
Using gprofiler for profiling the memory usage of the program.

## Implementation
1. Calibration
   2 calibration methods are provided.
   input: configuration file (default.xml)
   output: Cam0Config.xml (calibration.cpp) extrinsics.yml, intrinsics.yml(stereoCalibration.cpp)
        
   calibration.cpp only calibrates one single camera.   
   stereoCalibration.cpp calibrate a stereo camera pair.
   Note: stereoCalibration return the found pattern once both camera find the same pattern.
        The condition is stricter than the single calibration, may needs more time.

2. Eye Tracking
Implementing openCV haarcascade classifier to detect face. (Only detects the largest face, with flag enabled) 
Once user face detected, eye template is extracted. Then use trackEye function to do template matching
on the current frame.

3. Eye Depth
Two ways of measure eye depth have been implemented.
(1) Using CamShift algorithm to track hue image of the face detected. Then set the height of the bounding
box of the tracked area to indicate the depth change. 
(2) Using stereo blocking matching algorithm to compute disparity map with the camera matrices known from 
the calibration stage. 
4. cube
Use openGL to display a 3D cube to present the eye position change.

## Limitation & Improvement
### Disparity
1. Disparity map only works for a certain distance range between the user and the camera pair.
   [30 cm, 2 meters]
2. Disparity map accuracy mainly is determined by the locations of two cameras.
   To get good quality disparity map, two cameras must be parallel, and be calibrated.

### Eye Tracking   
1. Camshift algorithm tracks the area with similar hues, so it may track the user's neck 
   as well. Need to restrict the tracking algorithm further. 
2. Haarcascade algorithm for eye template extraction may not be very accurate.
3. SURF algorithm was also implemented, can match certain features, but not work well with estimating the face bounding box.

By Tianxin Tang
University of Bristol
Contact: tt1589@my.bristol.ac.uk