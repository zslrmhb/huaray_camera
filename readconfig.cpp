//
// Created by andrewz on 3/4/23.
//
#include "opencv2/cvconfig.h"
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"
#include "readconfig.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <cstdio>
#include "Camera.h"
using namespace std;
using namespace cv;
using namespace rapidjson;
int exposure, width, height;

const float calibrationSquareDimension = 0.01905f; //meters (might need to change)
const float arucoSquareDimension = 0.1016f; //meters (might need to change)
const Size chessboardDimensions = Size(9, 6);
Mat frame;
Mat drawToFrame;

Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
Mat distanceCoefficients;

vector<Mat> savedImages;

vector<vector<Point2f>> markerCorners, rejectCandidates;

//VideoCapture vid(0);

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners){
    for(int i = 0; i < boardSize.height; i++){
        for(int j = 0; j < boardSize.width; j++){
            corners.push_back(Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));
        }
    }
}

void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>> allFoundCorners, bool showResults = false){
    for(vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++){
        vector<Point2f> pointBuf;
        bool found = findChessboardCorners(*iter, Size(9,6), pointBuf, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
        if(found){
            allFoundCorners.push_back(pointBuf);
        }

        if(showResults){
            drawChessboardCorners(*iter, Size(9, 6), pointBuf, found);
            imshow("Looking for Corners", *iter);
            waitKey(0);
        }
    }
}

void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCore){
    vector<vector<Point2f>> checkerboardImageSpacePoints;
    getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

    vector<vector<Point3f>> worldSpaceCornerPoints(1);

    createKnownBoardPosition(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
    worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

    vector<Mat> rVectors, tVectors;
    distanceCoefficients = Mat::zeros(8, 1, CV_64F);

    calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);
}

bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoefficients){
    ofstream outStream(name);
    if(outStream){
        uint16_t rows = cameraMatrix.rows;
        uint16_t columns = cameraMatrix.cols;

        for(int r = 0; r < rows; r++){
            for(int c = 0; c < columns; c++){
                double value = cameraMatrix.at<double>(r, c);
                outStream << value << endl;
            }
        }
        rows = distanceCoefficients.rows;
        columns = distanceCoefficients.cols;

        for(int r = 0; r < rows; r++){
            for(int c = 0; c < columns; c++){
                double value = distanceCoefficients.at<double>(r, c);
                outStream << value << endl;
            }
        }
        outStream.close();
        return true;
    }
    return false;
}

int getVals() {


//    if(!vid.isOpened()){
//        return 1;
//    }

    int framesPerSecond = 20;
    namedWindow("Webcam", cv::WINDOW_AUTOSIZE);
    Camera camera;
    camera.init();
    camera.getImage(frame);
    while(true){
//        if(!vid.read(frame)){
//            break;
//        }
        vector<Vec2f> foundPoints;
        bool found = false;

        found = findChessboardCorners(frame, chessboardDimensions, foundPoints, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
        frame.copyTo(drawToFrame);
        drawChessboardCorners(drawToFrame, chessboardDimensions, foundPoints, found);
        if(found){
            imshow("Webcam", drawToFrame);
        } else{
            imshow("Webcam", frame);
        }
        char character = waitKey(1000 / framesPerSecond);

        switch(character){
            case ' ':
                //saving image
                if(found){
                    Mat temp;
                    frame.copyTo(temp);
                    savedImages.push_back(temp);
                }
                break;
            case 13:
                //start calibration
                if(savedImages.size() > 15) {
                    cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension, cameraMatrix,
                                      distanceCoefficients);
                    saveCameraCalibration("config.json", cameraMatrix, distanceCoefficients);
                }
                break;
            case 27:
                //exit
                return 0;
        }
    }



    // Open the file
    FILE * fp = fopen("../config.json", "rb");

    // Check if the file was opened successfully
    if (!fp) {
        cerr << "Error: unable to open file"
                  << endl;
        return 1;
    }

    // Read the file into a buffer
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer,
                                 sizeof(readBuffer));

    // Parse the JSON document
    Document doc;
    doc.ParseStream(is);

    // Check if the document is valid
    if (doc.HasParseError()) {
        cerr << "Error: failed to parse JSON document"
                  << endl;
        fclose(fp);
        return 1;
    }

    // Close the file
    fclose(fp);

    exposure = doc["camProperties"]["exposureTime"].GetInt();
    width = doc["camProperties"]["width"].GetInt();
    height = doc["camProperties"]["width"].GetInt();

//    cout << doc["camProperties"]["exposureTime"].GetInt() << endl;
//    cout << doc["camProperties"]["width"].GetInt() << endl;
//    cout << doc["camProperties"]["height"].GetInt() << endl;
    return 0;
}

