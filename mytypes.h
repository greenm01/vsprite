#ifndef MYTYPES_H
#define MYTYPES_H

typedef int bool;
#define true 1
#define false 0
//const bool true = 1;
//const bool false = 0;

typedef float float32; //XXX was this a box2d typedef?

typedef struct Vec2 {
  double x, y;
} Vec2;

#define radians(x) (x*M_PI/180.0)
#define degrees(x) (x*180.0/M_PI)

#endif
