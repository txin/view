view
====

change view depending on the eye locations

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


By Tianxin Tang
University of Bristol
Contact: titian.tang@gmail.com