/* 
 * camera calibration
 * reference: opencv sample code
 * opencv-2.4.9/samples/cpp/tutorial_code/calib3d/
 * camera_calibration/camera_calibration.cpp
 */
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif

using namespace cv;
using namespace std;

class Settings {
public:
    Settings(): goodInput(false) {} 
    
    void read(const FileNode& node) {                          //Read serialization for this class
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
        interprate();
    }
    void interprate() {
        goodInput = true;
        if (boardSize.width <= 0 || boardSize.height <= 0) {
            cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
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

        // default input images files
        if (input.empty())      // Check for valid input
            goodInput = false;
        else {
            if (input[0] >= '0' && input[0] <= '9') {
                // camera
            }
            else {
                if (readStringList(input, imageList)) {
                    nrFrames = (nrFrames < (int)imageList.size()) ? nrFrames : (int)imageList.size();
                }
            }
        }
        flag = 0;
        if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
        if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
        if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;
        atImageList = 0;
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
    Size boardSize;            // The size of the board -> Number of items by width and height
    float squareSize;          // The size of a square in your defined unit (point, millimeter,etc).
    int nrFrames;              // The number of frames to use from the input for calibration
    float aspectRatio;         // The aspect ratio
    int delay;                 // In case of a video input
    bool bwritePoints;         //  Write detected feature points
    bool bwriteExtrinsics;     // Write extrinsic parameters
    bool calibZeroTangentDist; // Assume zero tangential distortion
    bool calibFixPrincipalPoint;// Fix the principal point at the center
    bool flipVertical;          // Flip the captured images around the horizontal axis
    string outputFileName;      // The name of the file where to write
    bool showUndistorsed;       // Show undistorted images after calibration
    string input;              
    int cameraID;
    vector<string> imageList;
    int atImageList;
    bool goodInput;
    int flag;
private:
    string patternToUse;
};

static void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

int main(int argc, char** argv) {
    Settings s;
    const string inputSettingsFile = "default.xml";
    FileStorage fs(inputSettingsFile, FileStorage::READ);
    if (!fs.isOpened()) {
        cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
    }
    fs["Settings"] >> s;
    fs.release();

    if (!s.goodInput) {
        cout << "Invalid input detected. Application stopping." << endl;
        return -1;
    }
    return 0;
}
