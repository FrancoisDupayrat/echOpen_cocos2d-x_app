//
//  ScanData.cpp
//  echOpen
//
//  Created by François Dupayrat on 28/11/2015.
//
//

#include "ScanData.h"
#include <fstream>
#include <streambuf>

using namespace cocos2d;

#if IMAGE == KIDNEY
static float distanceBetween[LINES_NUMBER];
static float angleBetween[POINTS_PER_LINE];
#else
static float distanceBetween[POINTS_PER_LINE];
static float angleBetween[LINES_NUMBER];
#endif

static Vec2 positionsOnscreen[EXPECTED_VALUES];

void ScanData::precomputeValues(float screenWidth) {
    float distance;
    float angle;
#if IMAGE == KIDNEY
    for(int i = 0; i < LINES_NUMBER; i++) {
        distance = (i % LINES_NUMBER) * DISTANCE_INCREMENT * screenWidth + INITIAL_DISTANCE;
        distanceBetween[i] = 2 * sinf(ANGLE_INCREMENT / 2*PI/180) * distance;
    }
    for(int i = 0; i < POINTS_PER_LINE; i++) {
        angleBetween[i] = -1 * (i * ANGLE_INCREMENT - INITIAL_ANGLE);
    }
#else
    for(int i = 0; i < POINTS_PER_LINE; i++) {
        distance = (i % POINTS_PER_LINE) * DISTANCE_INCREMENT * screenWidth + INITIAL_DISTANCE;
        distanceBetween[i] = 2 * sinf(ANGLE_INCREMENT / 2*PI/180) * distance;
    }
    for(int i = 0; i < LINES_NUMBER; i++) {
        angleBetween[i] = -1 * ((i % LINES_NUMBER) * ANGLE_INCREMENT - INITIAL_ANGLE);
    }
#endif
    
    for(int i = 0; i < EXPECTED_VALUES; i++) {
#if IMAGE == KIDNEY
        distance = (i / POINTS_PER_LINE) * DISTANCE_INCREMENT * screenWidth + INITIAL_DISTANCE;
        angle = (i % POINTS_PER_LINE) * ANGLE_INCREMENT - INITIAL_ANGLE;
#else
        distance = (i % POINTS_PER_LINE) * DISTANCE_INCREMENT * screenWidth + INITIAL_DISTANCE;
        angle = (i / POINTS_PER_LINE) * ANGLE_INCREMENT - INITIAL_ANGLE;
#endif
        //Sin = opposé (x) / hypothénuse (distance)
        //Cos = adjacent (y) / hypothénuse (distance)
        positionsOnscreen[i] = Vec2(distance * cosf(angle*PI/180), distance * sinf(angle*PI/180) + screenWidth / 2);
    }
}

void ScanData::readFromFile(std::string filePath, bool internal) {
    
    std::string fileData = "";
    //Special case for Android, since internal file are in the apk (which is a zip) and can't be accessed directly from C++
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    if(internal)
    {
        ssize_t bufferSize = 0;
        //Load file from apk
        fileData = FileUtils::getInstance()->getStringFromFile(filePath);
    }
    std::string path = internal ? "" : (FileUtils::getInstance()->getWritablePath() + filePath);
#else
    
    std::string path = internal ? FileUtils::getInstance()->fullPathForFilename(filePath) : (FileUtils::getInstance()->getWritablePath() + filePath);
#endif
    if(fileData.empty()) {
        //Solution taken from http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        //Use this instead of FileUtils because it's the most performant way
        std::ifstream inputStream(path);
        
        inputStream.seekg(0, std::ios::end);
        fileData.reserve(inputStream.tellg());
        inputStream.seekg(0, std::ios::beg);
        
        fileData.assign((std::istreambuf_iterator<char>(inputStream)),
                        std::istreambuf_iterator<char>());
    }
    if(!fileData.empty()) {
        //We could split the string with the STL, but that would mean lots of allocation, for a large file, it's probably not a good idea
        int i = 0; //current position in the string
        int numberBegin = 0; //beginning position of current number, make it > i to mean we are not in a number
        while(i < fileData.length()) {
            char character = fileData[i];
            //Abuse the fact that number are consecutives in Unicode, also test for '-' and '.' in case the file use floats
            if((character >= '0' && character <= '9') || character == '-' || character == '.') {
                if(numberBegin > i) {
                    numberBegin = i;
                }
            }
            //Treat everything else as a separator
            else {
                //End of a number, decode it by getting the part of the string and atoi to convert into an integer
                if(numberBegin < i) {
                    std::string substr = fileData.substr(numberBegin, i - numberBegin);
                    double value = atof(substr.c_str());
                    if(value < 256) {
                        data.push_back(value);
                    }
                }
                //Mark that we are not in a number anymore
                numberBegin = i + 2;
            }
            i++;
        }
        CCLOG("Read %lu integers from %s", data.size(), filePath.c_str());
    }
    else {
        CCLOG("Error reading from file %s, %s", filePath.c_str(), internal ? "internal" : "not internal");
    }
}
const std::vector<int>& ScanData::getRawData() {
    return data;
}


//Positions assume the reference point is at 0,0
std::vector<PointData> ScanData::getPositions(int screenWidth) {
    std::vector<PointData> positions;
    //Fill random positions, waiting for real implementation
    int i;
#if DEBUG_LOG
    float minx, maxx, miny, maxy, x, y;
#endif
    for(i = 0; i < data.size(); i++) {
        int value = data[i] < 0 ? data[i] + 255 : data[i];
        if(value > MINIMAL_VALUE) {
#if IMAGE == KIDNEY
            positions.push_back(PointData(positionsOnscreen[i], value, distanceBetween[i / POINTS_PER_LINE], angleBetween[i % POINTS_PER_LINE]));
#else
            positions.push_back(PointData(positionsOnscreen[i], value, distanceBetween[i % POINTS_PER_LINE], angleBetween[i / POINTS_PER_LINE]));
#endif
#if DEBUG_LOG
            x = positionsX[i];
            y = positionsY[i];
            if(data[i] > 255 || data[i] < -255) {
                CCLOG("bad data: %d, position: %d", data[i], i);
            }
            if(std::isinf(x) || std::isinf(y)) {
                CCLOG("error, invalid x or y value at position %d", i);
            }
            if(i == 0) {
                minx = x;
                maxx = x;
                miny = y;
                miny = y;
            }
            else {
                if(x < minx) minx = x;
                if(x > maxx) maxx = x;
                if(y < miny) miny = y;
                if(y > maxy) maxy = y;
            }
#endif
        }
    }
#if DEBUG_LOG
    CCLOG("EXPECTED_VALUES: %d, values: %lu\nx:[%f, %f]; y:[%f, %f]", EXPECTED_VALUES, data.size(), minx, maxx, miny, maxy);
#endif
    return positions;
}