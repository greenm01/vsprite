#ifndef PATH_H
#define PATH_H

#include <GL/glfw.h>
#include "uthash/utlist.h"
#include "uthash/utarray.h"
#include "mytypes.h"
#include "matrix.h"
#include "gradient.h"

typedef struct Loop {
  Vec2 *points;
  int npoints;
} Loop;

typedef struct Path {
  char *id;             // XML path ID

  Loop **loops;         // (concave) polygons - array of arrays of points
  int nloops;

  GLubyte fill[4];      // Fill color (for all loops in path)
  GLubyte stroke[4];    // Stroke color
  int opacity;

  Gradient *gradient;
  Matrix *transform;
  // Next path in lined list
  struct Path *next;
  
} Path;

Path* path_new();
void path_reset(Path *p);
void path_push(Path *p, Vec2 vec);
void path_finish(Path *p);
void path_close(Path *p);
void path_apply_transform(Path *p);
void path_translate(Path *p, Vec2 center);
void path_render(Path *p);
void path_dump(Path *p);

#endif
