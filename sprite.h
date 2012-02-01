#ifndef SPRITE_H
#define SPRITE_H

#include <GL/glfw.h>
#include "mytypes.h"
#include "path.h"

typedef struct Sprite {
  GLuint displist;
  double width, height;
  double scale;         // arbitrary scaling factor

  Path *paths;
  Path *skeleton;
  int npaths;
} Sprite;

Sprite* sprite_new(const char *filename, float32 scale);

#endif
