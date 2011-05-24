#include "gradient.h"

#if 1
GLubyte* gradient_interp(Gradient *g, float x1, float y1) {
    GLubyte c[4] = {255, 0, 255, 255};
    return c;
}
#else
GLubyte *Gradient::interp(b2Vec2 pt) {
  if(stops.size() == 0) {
    GLubyte c[4] = {255, 0, 255, 255};
    return c;
  }
  float32 t = grad_value(inv_transform->call(pt));
  StopMap::iterator bottom = stops.begin();
  if (t < bottom->first) {
    return bottom->second;
  }
  using namespace std;
  StopMap::const_iterator top = bottom;
  for(top++; top != stops.end(); top++) {
    if (t < top->first) {
      float32 u = bottom->first;
      float32 v = top->first;
      float32 alpha = (t - u)/(v - u);
      GLubyte *x = bottom->second;
      GLubyte *y = top->second;
      GLubyte *rgba = new GLubyte[4];
      for(int i = 0; i < 4; i++) {
        rgba[i] = (int)(x[i] * (1 - alpha) + y[i] * alpha);
      }
      return rgba;
    }
    bottom++;
  }
  top--;
  return top->second;
}
#endif
