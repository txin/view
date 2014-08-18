/* 
 * camera calibration
 * reference: opencv sample code
 * opencv-2.4.9/samples/cpp/tutorial_code/calib3d/
 * camera_calibration/camera_calibration.cpp
 * add: reference stereo_calibrate.cpp
 * calibrate customed stereo camera
 */

#include "StereoCalibration.h"
#include "StereoView.h"

using namespace cv;
using namespace std;

enum {DETECTION = 0, CAPTURING = 1, CALIBRATED = 2};

class Settings {
public:
    Settings(): goodInput(false) {} 
    enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};

    void write(FileStorage& fs) const {//Write serialization for this class
        fs << "{" << "BoardSize_Width"  << boardSize.width
           << "BoardSize_Height" << boardSize.height
           << "Square_Size"         << squareSize
           << "Calibrate_NrOfFrameToUse" << nrFrames
           << "Calibrate_FixAspectRatio" << aspectRatio
           << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
           << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint
           << "Write_DetectedFeaturePoints" << bwritePoints
           << "Write_extrinsicParameters"   << bwriteExtrinsics
           << "Write_outputFileName"  << outputFileName
           << "Show_UndistortedImage" << showUndistorsed
           << "Input_FlipAroundHorizontalAxis" << flipVertical
           << "Input_Delay" << delay
           << "Input" << input
           << "}";
    }

    void read(const FileNode& node) {       //Read serialization for this class
        node["BoardSize_Width" ] >> boardSize.width;
        node["BoardSize_Height"] >> boardSize.height;
        node["Square_Size"]  >> squareSize;
        node["Calibrate_NrOfFrameToUse"] >> nrFrames;
        node["Calibrate_FixAspectRatio"] >> aspectRatio;
        node["Write_DetectedFeaturePoints"] >> bwritePoints;
        node["Write_extrinsicParameters"] >> bwriteExtrinsics;
        node["Write_outputFileName"] >> outputFileName;
        node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
        node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
        node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
        node["Show_UndistortedImage"] >> showUndistorsed;
        node["Input"] >> input;
        node["Input_Delay"] >> delay;
        interpret();
    }

    void interpret() {
        goodInput = true;
        if (boardSize.width <= 0 || boardSize.height <= 0) {
            cerr << "Invalid Board size: " << boardSize.width << " " 
                 << boardSize.height << endl;
            goodInput = false;
        }
        if (squareSize <= 10e-6) {
            cerr << "Invalid square size " << squareSize << endl;
            goodInput = false;
        }
        if (nrFrames <= 0) {
            cerr << "Invalid number of frames " << nrFrames << endl;
            goodInput = false;
        }

        // default input camera
        if (input.empty()) {      // Check for valid input
            inputType = INVALID;
        } else {
            inputType = CAMERA;
            if (inputType == CAMERA) {
                // open the camera specified in the configuration file
                // TODO: testing for 1, 2, two stereoCamera, maybe need to change!!
                inputCapture[0].open(1);
                inputCapture[1].open(2);
            }
        }
        if (inputType == INVALID) {
            cerr << "Input is invalid here" << input;
            goodInput = false;
        }
        flag = 0;
        if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
        if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
        if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;
        atImageList = 0;
    }

    Mat nextImage(int cameraID) {
        Mat result;
        if (inputCapture[cameraID].isOpened()) {
            Mat view0;
            inputCapture[cameraID] >> view0;
            view0.copyTo(result);
        }
        return result;
    }

// default pattern: CHESSBOARD
public:
    Size boardSize;           
    float squareSize;         
    int nrFrames;             
    float aspectRatio;        
    int delay;                 // In case of a video input
    bool bwritePoints;         //  Write detected feature points
    bool bwriteExtrinsics;     // Write extrinsic parameters
    bool calibZeroTangentDist; // Assume zero tangential distortion
    bool calibFixPrincipalPoint;// Fix the principal point at the center
    bool flipVertical;         // Flip the captured images around the horizontal axis
    string outputFileName;      // The name of the file where to write
    bool showUndistorsed;       // Show undistorted images after calibration
    string input;              
    int cameraID; // default 1, 2
    vector<string> imageList;
    int atImageList;
    VideoCapture inputCapture[2]; // for stereoCamera
    InputType inputType;
    bool goodInput;
    int flag;
private:
    string patternToUse;
};


static void read(const FileNode& node, Settings& x, 
                 const Settings& default_value = Settings()) {
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}


// CALIBRATION QUALITY CHECK
// because the output fundamental matrix implicitly
// includes all the output information,
// we can check the quality of calibration using the
// epipolar geometry constraint: m2^t*F*m1=0
static void checkErrors(vector<vector<Point2f> > *imagePoints,
                        Mat *cameraMatrix, Mat *distCoeffs,
                        Mat& F) {
    double err = 0;
    int npoints = 0;
    vector<Vec3f> lines[2];
   
    // testing about the reprojection error
    int nImageSize = 10;
    for (int i = 0; i < nImageSize; i++) {
        int npt = (int)imagePoints[0][i].size();
        Mat imgpt[2];
    
        for (int k = 0; k < 2; k++ ) {
            imgpt[k] = Mat(imagePoints[k][i]);
            undistortPoints(imgpt[k], imgpt[k], cameraMatrix[k], 
                            distCoeffs[k], Mat(), cameraMatrix[k]);
            // compute the stereoPair
            computeCorrespondEpilines(imgpt[k], k+1, F, lines[k]);
        }
        
        for (int j = 0; j < npt; j++ ){
            double errij = fabs(imagePoints[0][i][j].x*lines[1][j][0] +
                                imagePoints[0][i][j].y*lines[1][j][1] + 
                                lines[1][j][2]) +
                fabs(imagePoints[1][i][j].x*lines[0][j][0] +
                     imagePoints[1][i][j].y*lines[0][j][1] + lines[0][j][2]);
            err += errij;
        }
        npoints += npt;
    }
    cout << "average reprojection err = " <<  err/npoints << endl;
}

// run cameraCalibration separtely for 2 imagePoints, then run stereoCalibration
// to calculate the extrincincs matrix
static bool runStereoCalibration(vector<vector<Point2f> > *imagePoints, 
                                 Settings s) {
    // TODO: change initialisation of objectPoints
    vector<vector<Point3f> > objectPoints;
    vector<Mat> rvecs[2]; 
    vector<Mat> tvecs[2];

    Mat cameraMatrix[2], distCoeffs[2];
    cameraMatrix[0] = Mat::eye(3, 3, CV_64F);
    cameraMatrix[1] = Mat::eye(3, 3, CV_64F);
    Size imageSize = Size(640, 480);
    Mat R, T, E, F;

    // TODO: change the frame size
    int nImageSize = 10;
    imagePoints[0].resize(nImageSize);
    imagePoints[1].resize(nImageSize);
    objectPoints.resize(nImageSize);
    
    // TODO: change??
    for (int i = 0; i < nImageSize; i++ ) {
        for (int j = 0; j < s.boardSize.height; j++ )
            for (int k = 0; k < s.boardSize.width; k++ )
                objectPoints[i].push_back(Point3f(j * s.squareSize, 
                                                  k* s.squareSize, 0));
    }

    // error
    double rms[2];
    // calibrate cameras separately then use stereoCalibrate to 
    // get the extrinsic matrix
    for (int i = 0; i < 2; i++) {
        rms[i] = calibrateCamera(objectPoints, imagePoints[i], imageSize, 
                                 cameraMatrix[i],
                                 distCoeffs[i], rvecs[i], tvecs[i], 
                                 s.flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);
    }


    // TODO: probably need to sort out the initialisation of the objectPoints
 
    cout << "done with RMS rms[0] =" << rms[0] << "rms[1]=" << rms[1] << endl;

    double rmsStereo = stereoCalibrate(objectPoints, 
                                       imagePoints[0], imagePoints[1], 
                                       cameraMatrix[0], distCoeffs[1],
                                       cameraMatrix[0], distCoeffs[1], 
                                       imageSize, R,  T,  E,  F,    
                                       TermCriteria(CV_TERMCRIT_ITER
                                                    + CV_TERMCRIT_EPS, 
                                                    100, 1e-5),
                                       CV_CALIB_FIX_INTRINSIC +
                                       CV_CALIB_FIX_ASPECT_RATIO +
                                       CV_CALIB_FIX_K4 + CV_CALIB_FIX_K5);
    cout << "stereo rms" << rmsStereo << endl;
    checkErrors(imagePoints, cameraMatrix, distCoeffs, F); 
    // save intrinsic parameters
    FileStorage fs("intrinsics.yml", CV_STORAGE_WRITE);
    if (fs.isOpened()) {
        fs << "M1" << cameraMatrix[0] << "D1" << distCoeffs[0] <<
            "M2" << cameraMatrix[1] << "D2" << distCoeffs[1];
        fs.release();
    }
    else
        cout << "Error: can not save the intrinsic parameters\n";

    Mat R1, R2, P1, P2, Q;
    Rect validRoi[2];

    stereoRectify(cameraMatrix[0], distCoeffs[0],
                  cameraMatrix[1], distCoeffs[1],
                  imageSize, R, T, R1, R2, P1, P2, Q,
                  CALIB_ZERO_DISPARITY, 1, 
                  imageSize, &validRoi[0], &validRoi[1]);

    fs.open("extrinsics.yml", CV_STORAGE_WRITE);
    if( fs.isOpened() ) {
        fs << "R" << R << "T" << T << "R1" 
           << R1 << "R2" << R2 << "P1" << P1 << "P2" << P2 << "Q" << Q;
        fs.release();
    }
    else
        cout << "Error: can not save the intrinsic parameters\n";
    return true;
}

bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix, 
                           Mat& distCoeffs,
                           vector<vector<Point2f> > *imagePoints) {
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;

    bool ok = runStereoCalibration(imagePoints, s);
    return ok;
}

void display(Mat& view, int mode, Settings& s, bool blinkOutput, 
             string windowName, vector< vector<Point2f> > imagePoints) {
    const Scalar RED(0, 0, 255), GREEN(0, 255, 0);
    // Output Text
    string msg = (mode == CAPTURING) ? "100/100":
        mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
    int baseLine = 0;
    Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
    Point textOrigin(view.cols - 2 * textSize.width - 10, 
                     view.rows - 2 * baseLine - 10);
    if (mode == CAPTURING) {
        if(s.showUndistorsed)
            msg = format("%d/%d Undist", (int)imagePoints.size(), 
                         s.nrFrames);
        else
            msg = format("%d/%d", (int)imagePoints.size(), s.nrFrames);
    }
    putText(view, msg, textOrigin, 1, 1, mode == CALIBRATED? GREEN : RED);

    if( blinkOutput )
        bitwise_not(view, view);

    cv::imshow(windowName, view);
}


// calibrate for camera 0 and 1
int StereoCalibration::setup(int cameraNo) {
    Settings s;
    // default configuration file for camera use
    const string inputSettingsFile = "default.xml";

    string windowName = "Camera";
    string indexStr = std::to_string(cameraNo);
    windowName += indexStr;

    FileStorage fs(inputSettingsFile, FileStorage::READ);
    if (!fs.isOpened()) {
        cout << "Could not open the configuration file: \"" 
             << inputSettingsFile 
             << "\"" << endl;
    }
    fs["Settings"] >> s;
    fs.release();


    std::cout << "configuration files read" << std::endl;


    // set up camera id
    s.cameraID = cameraNo;
    // set up output file name
    s.outputFileName = windowName + "config.xml";

    s.interpret();

    if (!s.goodInput) {
        cout << "Invalid input detected. Application stopping." << endl;
        return -1;
    }

    vector< vector<Point2f> > imagePoints[2];
    Mat cameraMatrix, distCoeffs;
    Size imageSize;

    mode = s.inputType == Settings::IMAGE_LIST? CAPTURING : DETECTION;

    clock_t prevTimestamp = 0;
    const char ESC_KEY = 27;

    for (int i = 0; ; i++) {
        Mat view[2];
        bool blinkOutput = false;
     
        // get the view from the opened inputCapture
        view[0] = s.nextImage(0);
        view[1] = s.nextImage(1);
     
        if (mode == CAPTURING && imagePoints[0].size() >= (unsigned)s.nrFrames) {
            cout << "first run calibration and save" << endl;
            if (runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs,
                                      imagePoints)) {
                mode = CALIBRATED;
            } else {
                mode = DETECTION;
            }
        }
        
        imageSize = view[0].size();
        
        if (s.flipVertical) {
            flip(view[0], view[0], 0);
        }    
        
        vector<Point2f> pointBuf[2];
        bool found[2];

        for (int i = 0; i < 2; i++) {
            found[i] = findChessboardCorners(view[i], s.boardSize, pointBuf[i],
                                             CV_CALIB_CB_ADAPTIVE_THRESH 
                                             | CV_CALIB_CB_FAST_CHECK 
                                             | CV_CALIB_CB_NORMALIZE_IMAGE);
        }

// pattern: chessboard
        if (found[0] && found[1]) {
            cout << "both found" << endl;

            Mat viewGray[2];

            for (int i = 0; i < 2; i++) {
                cvtColor(view[i], viewGray[i], COLOR_BGR2GRAY);
                cornerSubPix(viewGray[i], pointBuf[i], Size(11,11),
                             Size(-1,-1), 
                             TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 
                                           30, 0.1));
            }
            
            // TODO: change the delay
            // for camera only
            if(mode == CAPTURING &&  
               (!s.inputCapture[0].isOpened() 
                || clock() - prevTimestamp > s.delay * 1e-3 * CLOCKS_PER_SEC)) {

                // push back the pointBuf[0] into the vector
                imagePoints[0].push_back(pointBuf[0]);
                imagePoints[1].push_back(pointBuf[1]);

                prevTimestamp = clock();
                // TODO: blinkOutput
                blinkOutput = s.inputCapture[0].isOpened();
            }
            drawChessboardCorners(view[0], s.boardSize, Mat(pointBuf[0]), found[0]);
            drawChessboardCorners(view[1], s.boardSize, Mat(pointBuf[1]), found[1]);
        } 
        
        display(view[0], mode, s, blinkOutput,"Camera0", imagePoints[0]);
        display(view[1], mode, s, blinkOutput, "Camera1", imagePoints[1]);

        // key detection to end the program
        char key = (char)waitKey(s.inputCapture[0].isOpened() ? 50 : s.delay);
        
        if (key == ESC_KEY || key == 'q' || key == 'Q') {
            break;
        }
        if (key == 'u' && mode == CALIBRATED) {
            s.showUndistorsed = !s.showUndistorsed;
        }
        if (s.inputCapture[0].isOpened() && key == 'g') {
            mode = CAPTURING;
            imagePoints[0].clear();
            imagePoints[1].clear();
        }
    }
    return 0;
}

// for unit testing
int main() {
    for (int i = 0; i < CAMERA_NUM; i++) {
        std::string windowName("Camera");
        windowName += std::to_string(i);
        cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);
        cv::moveWindow(windowName, CAMERA_WIDTH * i, 0);
    }
    StereoCalibration test;
    test.setup(0);
    return 0;
}

