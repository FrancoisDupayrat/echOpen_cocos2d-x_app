//
//  Constants.h
//  echOpen
//
//  Created by François Dupayrat on 19/12/2015.
//
//

#ifndef Constants_h
#define Constants_h

/*The Image you want to use. Possible values :
 - KIDNEY : will use data_keydney.csv. Note : the values are reserved for this file ...
 - TEST : will use image.txt
 - anything else : you have to provide values
 */
#define OTHER 0
#define KIDNEY 1
#define TEST 2

#define IMAGE TEST

/* What is a basic "point" : a square or a point (see images in resources)
 */
#define POINT 1
#define SQUARE 2

#define DISPLAY_IMAGE SQUARE

//The size of the image, in pixels
#if KIDNEY //Use 1.5 to cover a bit more surface
#define DISPLAY_IMAGE_SIZE (32.0f*1.5)
#else
#define DISPLAY_IMAGE_SIZE 32.0f
#endif

//Number of values, dependant on the file or sensor properties
#if IMAGE == KIDNEY
#define EXPECTED_VALUES (1672*128)//214016
#define POINTS_PER_LINE 128
#define LINES_NUMBER (EXPECTED_VALUES / POINTS_PER_LINE) //128
#elif IMAGE == TEST
#define EXPECTED_VALUES (61*512)//31232
#define POINTS_PER_LINE 512 //1672
#define LINES_NUMBER (EXPECTED_VALUES / POINTS_PER_LINE) //61
#else
#define EXPECTED_VALUES (61*512)//Total number of values
#define POINTS_PER_LINE 512
#define LINES_NUMBER (EXPECTED_VALUES / POINTS_PER_LINE)
#endif

//The angle properties
#define INITIAL_ANGLE 45.0f //from a vertical standpoint, in degree

#if IMAGE == KIDNEY
#define ANGLE_INCREMENT (INITIAL_ANGLE*2/POINTS_PER_LINE)
#else
#define ANGLE_INCREMENT (INITIAL_ANGLE*2/LINES_NUMBER)
#endif

//The distance properties
#define INITIAL_DISTANCE 0.01f //0.05 (in meters?) CANNOT BE 0
#define MAX_DISTANCE 1.0f
#if IMAGE == KIDNEY
#define DISTANCE_INCREMENT ((MAX_DISTANCE-INITIAL_DISTANCE)/LINES_NUMBER) //0.105 total (in meters?)
#else
#define DISTANCE_INCREMENT ((MAX_DISTANCE-INITIAL_DISTANCE)/POINTS_PER_LINE) //0.105 total (in meters?)
#endif
#ifndef PI
#define PI 3.14159265
#endif

//The minimum threshold to display a point
#define MINIMAL_VALUE 30 //Out of 255

//Add debug logs
#define DEBUG_LOG 0


#endif /* Constants_h */
