#ifndef GRADIENT_H
#define GRADIENT_H

#include <string>
#include <map>
#include <GL/glfw.h>
#include <Box2D/Box2D.h>
#include "matrix.h"

class Gradient;
typedef std::map< float32, GLubyte * > StopMap;
typedef std::map< std::string, Gradient * > GradientMap;


class Gradient {
public:
  const char *id;
  GLubyte color[4];
  StopMap stops;
  // Inverse matrix transform
  Matrix *inv_transform;

  Gradient() {
    inv_transform = NULL;
  }

  Gradient(const char *id) : id(id) {
    inv_transform = NULL;
  }

  GLubyte *interp(b2Vec2 pt);

  ~Gradient() {
    delete inv_transform;
  }

  virtual float32 grad_value(b2Vec2 pt) {return 0;}
};


class LinearGradient : public Gradient {
public:
  // Gradient vector
  float32 x1, y1, x2, y2;

  LinearGradient(const char *id) : Gradient(id) {
    x1 = y1 = x2 = y2 = 0.0;
  }

  float32 grad_value(b2Vec2 pt) {
    return ((pt.x - x1)*(x2 - x1) + (pt.y - y1)*(y2 - y1)) /
            (pow(x1 - x2, 2) + pow(y1 - y2, 2));
  }
};


class RadialGradient : public Gradient {
public:
  float32 cx, cy, r;

  RadialGradient(const char *id) : Gradient(id) {
    cx = cy = r = 0.0;
  }

  float32 grad_value(b2Vec2 pt) {
    return sqrtf(pow(pt.x - cx, 2) + pow(pt.y - cy, 2))/r;
  }
};

#endif
