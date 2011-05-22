#ifndef PATH_H
#define PATH_H

#include <list>
#include <vector>
#include <GL/glfw.h>
#include <Box2D/Box2D.h>

#include "matrix.h"
#include "gradient.h"

class Path;
typedef std::list< b2Vec2 > Vec2list;
typedef std::list< Vec2list * > Vec2lol;  // List of lists :)
typedef std::list< Path * > PathList;
typedef std::vector< b2Vec2 > Vec2Vec;


class Path {
public:
  Vec2lol loops;        // (concave) polygons
  GLubyte fill[4];      // Fill color (for all loops in path)
  GLubyte stroke[4];    // Stroke color
  int opacity;
  Gradient *gradient;
  Matrix *transform;

  // Constructor
  Path(){
    reset();
    opacity = 1;
    fill[0] = 0; fill[1] = 0; fill[2] = 0; fill[3] = 0;
    stroke[0] = 0; stroke[1] = 0; stroke[2] = 0; stroke[3] = 0;
    gradient = NULL;
    transform = NULL;
  }

  ~Path();

  void reset() {
    loop = new Vec2list;
  }

  void push(b2Vec2 vec) {
    loop->push_back(vec);
  }

  void finish() {
    if(!loop->empty()) {
      //TODO reduce vertices... see Squirtle .end_path()
      loops.push_back(loop);
      reset();
    }
  }

  void close() {
    loop->push_back(loop->front());
    finish();
  }

  void apply_transform();
  void translate(b2Vec2 center);
  void render();
  void dump();

private:
  Vec2list *loop;       // temporary (current loop)
};


//------------------------------------------------------------------------
inline void Path::apply_transform() {
  if(transform) {
    Vec2lol::iterator i = loops.begin();
    Vec2list::iterator v;

    // Transform points if necessary
    while(i != loops.end()) {
      v = (*i)->begin();
      while(v != (*i)->end()) {
        *v = transform->xform(*v);
        ++v;
      }
      ++i;
    }
  }
}

inline void Path::translate(b2Vec2 center) {
  Vec2lol::iterator i = loops.begin();
  Vec2list::iterator v;
  // Transform points if necessary
  while(i != loops.end()) {
    v = (*i)->begin();
    while(v != (*i)->end()) {
      v->x -= center.x;
      v->y -= center.y;
      ++v;
    }
    ++i;
  }
}

#endif
