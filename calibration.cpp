/* 
 * camera calibration
 * reference: opencv sample code
 * opencv-2.4.9/samples/cpp/tutorial_code/calib3d/
 * camera_calibration/camera_calibration.cpp
 * calibration from live camera
 */

#include "Calibration.h"
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
            // default input Type camera
            inputType = CAMERA;
            if (inputType == CAMERA) {
                inputCapture.open(cameraID);
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

    Mat nextImage() {
        Mat result;
        if (inputCapture.isOpened()) {
            Mat view0;
            inputCapture >> view0;
            view0.copyTo(result);
        }
        else if (atImageList < (int)imageList.size() ) {
            result = imread(imageList[atImageList++], CV_LOAD_IMAGE_COLOR);
        }
        return result;
    }

    static bool readStringList(const string& filename, vector<string>& l) {
        l.clear();
        FileStorage fs(filename, FileStorage::READ);
        if(!fs.isOpened())
            return false;
        FileNode n = fs.getFirstTopLevelNode();
        if(n.type() != FileNode::SEQ )
            return false;
        FileNodeIterator it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it )
            l.push_back((string)*it);
        return true;
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
    int cameraID;
    vector<string> imageList;
    int atImageList;
    VideoCapture inputCapture;
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

static double computeReprojectionErrors(const vector<vector<Point3f> >& objectPoints,
                                        const vector<vector<Point2f> >& imagePoints,
                                        const vector<Mat>& rvecs, 
                                        const vector<Mat>& tvecs, 
                                        const Mat& cameraMatrix , 
                                        const Mat& distCoeffs,
                                        vector<float>& perViewErrors) {
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i) {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);
        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }
    return std::sqrt(totalErr/totalPoints);
}

static void calcBoardCornerPositions(Size boardSize, float squareSize, 
                                     vector<Point3f>& corners) {
    corners.clear();
    for( int i = 0; i < boardSize.height; ++i )
        for( int j = 0; j < boardSize.width; ++j )
            corners.push_back(Point3f(float(j*squareSize), 
                                      float(i*squareSize), 0));
}

static bool runCalibration(Settings& s, Size& imageSize, Mat& cameraMatrix,
                           Mat& distCoeffs, 
                           vector<vector<Point2f> > imagePoints, 
                           vector<Mat>& rvecs, vector<Mat>& tvecs, 
                           vector<float>& reprojErrs,  double& totalAvgErr) {
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = 1.0;
    distCoeffs = Mat::zeros(8, 1, CV_64F);
    vector<vector<Point3f> > objectPoints(1);
    calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0]);
    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters
    double rms = calibrateCamera(objectPoints, imagePoints, imageSize, 
                                 cameraMatrix,
                                 distCoeffs, rvecs, tvecs, 
                                 s.flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;
    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);
    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                                            rvecs, tvecs, cameraMatrix,
                                            distCoeffs, reprojErrs);
    return ok;
}

// Print camera parameters to the output file
static void saveCameraParams( Settings& s, Size& imageSize,
                              Mat& cameraMatrix, Mat& distCoeffs,
                              const vector<Mat>& rvecs, 
                              const vector<Mat>& tvecs,
                              const vector<float>& reprojErrs, 
                              const vector<vector<Point2f> >& imagePoints,
                              double totalAvgErr ) {
    FileStorage fs( s.outputFileName, FileStorage::WRITE );
    time_t tm;
    time( &tm );
    struct tm *t2 = localtime( &tm );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );
    fs << "calibration_Time" << buf;
    if( !rvecs.empty() || !reprojErrs.empty() )
        fs << "nrOfFrames" << (int)std::max(rvecs.size(), reprojErrs.size());
    fs << "image_Width" << imageSize.width;
    fs << "image_Height" << imageSize.height;
    fs << "board_Width" << s.boardSize.width;
    fs << "board_Height" << s.boardSize.height;
    fs << "square_Size" << s.squareSize;

    if(s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        fs << "FixAspectRatio" << s.aspectRatio;

    if(s.flag) {
        sprintf( buf, "flags: %s%s%s%s",
                 s.flag & CV_CALIB_USE_INTRINSIC_GUESS ? " +use_intrinsic_guess" : "",
                 s.flag & CV_CALIB_FIX_ASPECT_RATIO ? " +fix_aspectRatio" : "",
                 s.flag & CV_CALIB_FIX_PRINCIPAL_POINT ? " +fix_principal_point" : "",
                 s.flag & CV_CALIB_ZERO_TANGENT_DIST ? " +zero_tangent_dist" : "" );
        cvWriteComment( *fs, buf, 0 );

    }
    fs << "flagValue" << s.flag;
    fs << "Camera_Matrix" << cameraMatrix;
    fs << "Distortion_Coefficients" << distCoeffs;
    fs << "Avg_Reprojection_Error" << totalAvgErr;
    if (!reprojErrs.empty())
        fs << "Per_View_Reprojection_Errors" << Mat(reprojErrs);
    if (!rvecs.empty() && !tvecs.empty() )    {
        CV_Assert(rvecs[0].type() == tvecs[0].type());
        Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
        for (int i = 0; i < (int)rvecs.size(); i++) {
            Mat r = bigmat(Range(i, i+1), Range(0,3));
            Mat t = bigmat(Range(i, i+1), Range(3,6));

            CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
            CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
            //*.t() is MatExpr (not Mat) so we can use assignment operator
            r = rvecs[i].t();
            t = tvecs[i].t();
        }
        cvWriteComment(*fs, 
                       "a set of 6-tuples (rotation vector + translation vector)for each view", 0 );
        fs << "Extrinsic_Parameters" << bigmat;
    }

    if (!imagePoints.empty()) {
        Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), 
                       CV_32FC2);
        for (int i = 0; i < (int)imagePoints.size(); i++ ) {
            Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
            Mat imgpti(imagePoints[i]);
            imgpti.copyTo(r);
        }
        fs << "Image_points" << imagePtMat;
    }
}

bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix, 
                           Mat& distCoeffs,
                           vector<vector<Point2f> > imagePoints) {
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;

    bool ok = runCalibration(s,imageSize, cameraMatrix, distCoeffs, 
                             imagePoints, rvecs, tvecs,reprojErrs, totalAvgErr);
    cout << (ok ? "Calibration succeeded" : "Calibration failed")
         << ". avg re projection error = "  << totalAvgErr ;
    if(ok)
        saveCameraParams( s, imageSize, cameraMatrix, distCoeffs, rvecs ,
                          tvecs, reprojErrs, imagePoints, totalAvgErr);
    printf("camera parameters saved\n");
    return ok;
}


// calibrate for camera 0 and 1
int Calibration::setup(int cameraNo) {
    Settings s;
    // default configuration file for camera use
    const string inputSettingsFile = "res/default.xml";

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

    // set up camera id
    s.cameraID = cameraNo;
    // set up output file name
    s.outputFileName = windowName + "config.xml";

    s.interpret();

    if (!s.goodInput) {
        cout << "Invalid input detected. Application stopping." << endl;
        return -1;
    }

    vector< vector<Point2f> > imagePoints;
    Mat cameraMatrix, distCoeffs;
    Size imageSize;
    int mode = s.inputType == Settings::IMAGE_LIST? CAPTURING : DETECTION;
    clock_t prevTimestamp = 0;
    const Scalar RED(0, 0, 255), GREEN(0, 255, 0);
    const char ESC_KEY = 27;

    for (int i = 0; ; i++) {
        Mat view;
        bool blinkOutput = false;
     
        view = s.nextImage();
     
        if (mode == CAPTURING && imagePoints.size() >= (unsigned)s.nrFrames) {
            if (runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs,
                                      imagePoints)) {
                mode = CALIBRATED;
            } else {
                mode = DETECTION;
            }
        }
        
        if (view.empty()) {
            if (imagePoints.size() > 0) {
                runCalibrationAndSave(s, imageSize,
                                      cameraMatrix, distCoeffs, imagePoints);
            }
            break;
        }
        
        imageSize = view.size();
        
        if (s.flipVertical) {
            flip(view, view, 0);
        }    
        
        vector<Point2f> pointBuf;
        bool found;
        found = findChessboardCorners(view, s.boardSize, pointBuf,
                                      CV_CALIB_CB_ADAPTIVE_THRESH 
                                      | CV_CALIB_CB_FAST_CHECK 
                                      | CV_CALIB_CB_NORMALIZE_IMAGE);
        // pattern: chessboard
        if (found) {
            Mat viewGray;
            cvtColor(view, viewGray, COLOR_BGR2GRAY);
            cornerSubPix(viewGray, pointBuf, Size(11,11),
                         Size(-1,-1), 
                         TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 
                                       30, 0.1));
        
            // for camera only
            if(mode == CAPTURING &&  
               (!s.inputCapture.isOpened() 
                || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC)) {
                imagePoints.push_back(pointBuf);
                prevTimestamp = clock();
                blinkOutput = s.inputCapture.isOpened();
            }
            drawChessboardCorners(view, s.boardSize, Mat(pointBuf), found);
        } 

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
        // display video calibrated and undistorted
        if(mode == CALIBRATED && s.showUndistorsed) {
            Mat temp = view.clone();
            undistort(temp, view, cameraMatrix, distCoeffs);
        }
        cv::imshow(windowName, view);
        char key = (char)waitKey(s.inputCapture.isOpened() ? 50 : s.delay);
        
        if (key == ESC_KEY || key == 'q' || key == 'Q') {
            break;
        }
        if (key == 'u' && mode == CALIBRATED) {
            s.showUndistorsed = !s.showUndistorsed;
        }
        if (s.inputCapture.isOpened() && key == 'g') {
            mode = CAPTURING;
            imagePoints.clear();
        }
    }

    // Show the undistorted image for the image list
    // default image list
   

    if (s.inputType == Settings::IMAGE_LIST && s.showUndistorsed) {
        Mat view, rview, map1, map2;
        initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
                                getOptimalNewCameraMatrix(cameraMatrix, 
                                                          distCoeffs, imageSize,
                                                          1, imageSize, 0),
                                imageSize, CV_16SC2, map1, map2);

        for (int i = 0; i < (int)s.imageList.size(); i++) {
            view = imread(s.imageList[i], 1);
            if(view.empty())
                continue;
            remap(view, rview, map1, map2, INTER_LINEAR);
            imshow(windowName, rview);
            char c = (char)waitKey();
            if (c  == ESC_KEY || c == 'q' || c == 'Q') break;
        }
    }
    return 0;
}

// thread id same to camera id
void *calibrate(void *t) {
    Calibration test;
    long tid = (long)t;
    int cameraNo = (int)tid;
    test.setup(cameraNo);
    pthread_exit(NULL);
}
/*
 * may have several threads for different camera calibrations
 */
int main(int argc, char* argv[]) {
    
    // creat 2 threads
    /*
    pthread_t threads[CAMERA_NUM];

    pthread_attr_t attr;
    void *status;

    for (int i = 0; i < CAMERA_NUM; i++) {
        std::string windowName("Camera");
        windowName += std::to_string(i);
        cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);
        cv::moveWindow(windowName, CAMERA_WIDTH * i, 0);
    }
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        
    int rc;
    for (int i = 0; i < CAMERA_NUM; i++) {
        std::cout << "main() : creating thread, " << i << endl;
        long index = (long)i;
        rc = pthread_create(&threads[i], NULL, calibrate, (void *)index);
    }

    pthread_attr_destroy(&attr);
    for (int i = 0; i < CAMERA_NUM; i++) {
        rc = pthread_join(threads[i], &status);
        if (rc) {
            std::cout << "Error: unable to join, " << rc << std::endl;
        }
        cout << "Main: completed thread id :" << i;
        cout << " exiting with status : " << status << std::endl;
    }
    std::cout << "Main: program exiting." << std::endl;
    */

    // default camera
    int cameraNo = 0;

    if (argc == 2) {
        if (*argv[1] < '9' && *argv[1] >= '0') {
            cameraNo = *argv[1] - '0';
        }
    }
    Calibration test;
    test.setup(cameraNo);
    return 0;
}

