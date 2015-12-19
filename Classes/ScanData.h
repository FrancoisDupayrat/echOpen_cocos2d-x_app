//
//  ScanData.h
//  echOpen
//
//  Created by François Dupayrat on 28/11/2015.
//
//

#ifndef ScanData_h
#define ScanData_h

#include "cocos2d.h"
#include "Constants.h"

/* The goal of this class is to be able to import scan data from multiple sources (file, stream)
 and present a unified interface to access it.
 The data type is not fixed yet, and will be based on other requirements*/
typedef struct _PointData {
    cocos2d::Vec2 position;
    float opacity;
    float distance;
    float angle;
    
    _PointData(cocos2d::Vec2 pos, float opac, float dist, float ang) : position(pos), opacity(opac), distance(dist), angle(ang)
    {
    }
} PointData;
class ScanData {
public:
    static void precomputeValues(float screenWidth);
    //read data from file, internal if it's embeded in the app (in the Resources folder), otherwise it will read from WritablePath
    void readFromFile(std::string filePath, bool internal);
    //Get the raw data, you need to read data from somewhere first
    const std::vector<int>& getRawData();
    //Get the points positions, computed from raw data, you need to read data from somewhere first
    std::vector<PointData> getPositions(int screenWidth); //TODO: implement me
private:
    std::vector<int> data;
};

#endif /* ScanData_h */
