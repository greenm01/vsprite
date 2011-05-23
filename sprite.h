#ifndef SPRITE_H
#define SPRITE_H

#include <GL/glfw.h>
#include "path.h"

typedef float float32; //XXX was this a box2d typedef?

typedef struct Sprite {
  GLuint displist;
  double width, height;
  double scale;         // arbitrary scaling factor

  Path *paths;
  int npaths;
} Sprite;

Sprite* sprite_new(const char *filename, float32 scale);

#endif
