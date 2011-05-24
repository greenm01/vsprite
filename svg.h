#ifndef SVG_H
#define SVG_H

#include <string.h>
#include <GL/glfw.h>
#include "uthash/uthash.h"
#include "uthash/utlist.h"
#include "mytypes.h"
#include "matrix.h"
#include "gradient.h"
#include "path.h"
#include "sprite.h"

bool svg_load(const char *filename, float32 scale, Sprite *sprite);

#if 0
//========================================================================
class SVGparser {
public:
  SVGparser() {
    svg_debug = false;
    vtable_init();
    path = NULL;
    transform_ = NULL;
  }

  ~SVGparser() {
    delete path;
    delete transform_;
  }

  // MAIN EXTERNAL INTERFACE
  bool load(const char *filename, float32 scale, Skin *skin);

  // Recursive SVG tag parser
  void parse_svg(XMLNode node, int indent);

  // Dictionary of SVG tags
  // (Heh, this is simply  vtable={'path': parse_path, ...}  in Python!)
  typedef std::map< std::string, void(SVGparser::*)(XMLNode) >  fnmap;
  fnmap vtable;

  void vtable_init();

  // SVG "path" command-code parser
  bool parse_cmd();

  // Internal parser state
  Path *path;
  PathList path_list;
  // Current (x,y) position
  b2Vec2 pos;
  // Current char in path string being parsed
  const char *p;

private:
  // Parsers for each SVG tag that we support
  void parse_path_(XMLNode node);
  void parse_rect_(XMLNode node);
  void parse_circle_(XMLNode node);
  void parse_radial_gradient_(XMLNode node);
  void parse_linear_gradient_(XMLNode node);
  void init_gradient_(XMLNode node, Gradient *gradient);

  // Parsing helpers
  void color_style_(XMLNode node);
  void parse_ws_();
  bool parse_coord_(float32 *coord);
  bool parse_coord_pair_(b2Vec2 *pos);
  void parse_stop_style_(const char *style, GLubyte *stop);
  bool parse_style_(const char *style);
  void parse_color_(std::string s, GLubyte *color);
  void parse_arc_(bool offset);
  void line_to_(float32 x2, float32 y2);
  void curve_to_(bool offset);
  void arc_to_(b2Vec2 radii, float32 phi, bool large_arc, bool sweep, b2Vec2 end);
  void lerp_(b2Vec2 &dest, b2Vec2 &a, b2Vec2 &b, float t);
  void cubic_bezier_(b2Vec2 &dest, float t, b2Vec2 a, b2Vec2 b, b2Vec2 c, b2Vec2 d);
  float32 angle_(b2Vec2 u, b2Vec2 v);

  // Gradient container
  GradientMap gradients_;
  Vec2list skeleton;    // Skeleton vertices
  Matrix *transform_;   // SVG transform
};

inline void SVGparser::line_to_(float32 x2, float32 y2) {
  path->push(b2Vec2(x2, y2));
  pos.Set(x2, y2);
}

// simple linear interpolation between two points
inline void SVGparser::lerp_(b2Vec2 &dest, b2Vec2 &a, b2Vec2 &b, float t) {
  dest.x = a.x + (b.x-a.x)*t;
  dest.y = a.y + (b.y-a.y)*t;
}

/* DeCasteljau Algorithm: http://www.cubic.org/docs/bezier.htm
 * Create a cubic Bézier curve
 * t goes from 0 to 1.0
*/
inline void SVGparser::cubic_bezier_(b2Vec2 &dest, float t, b2Vec2 a, b2Vec2 b, b2Vec2 c, b2Vec2 d) {
  b2Vec2 ab,bc,cd,abbc,bccd;
  lerp_(ab, a,b,t);           // point between a and b (green)
  lerp_(bc, b,c,t);           // point between b and c (green)
  lerp_(cd, c,d,t);           // point between c and d (green)
  lerp_(abbc, ab,bc,t);       // point between ab and bc (blue)
  lerp_(bccd, bc,cd,t);       // point between bc and cd (blue)
  lerp_(dest, abbc,bccd,t);   // point on the bezier-curve (black)
}

inline float32 SVGparser::angle_(b2Vec2 u, b2Vec2 v) {
  float32 a = acosf((u.x*v.x + u.y*v.y) / sqrtf((u.x*u.x + u.y*u.y) * (v.x*v.x + v.y*v.y)));
  float32 sgn = u.x*v.y > u.y*v.x ? 1 : -1;
  return sgn * a;
}

//------------------------------------------------------------------------
// Skip whitespace OR comma which is treated as whitespace in SVG
//
inline void SVGparser::parse_ws_() {
  while (*p && strchr(", \t\r\n", *p)) {
    p++;
  }
}

//========================================================================
#endif

#endif
