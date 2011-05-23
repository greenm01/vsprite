// ======================================================================
// tom's SVG path parser  2010-04-03
//
// - Simple top-down parser/lexer in the style of 'META II' [Schorre 1964]
//

#include "svgparser.h"

//------------------------------------------------------------------------
// Parses the given SVG file and stores it in the supplied Sprite.
// Returns true on success.
//
bool svg_load(const char *filename, float32 scale, Sprite *sprite) {
  if(svg_debug) {
    printf("===============================================================\n");
    printf("Loading %s  scale=%f\n", filename, scale);
    printf("===============================================================\n");
  }

  //TODO new XML parser....
  XMLNode top = XMLNode::openFileHelper(filename).getChildNode("svg");

  const char *width_s = top.getAttribute("width");
  const char *height_s = top.getAttribute("height");

  double width = atof(width_s);
  double height = atof(height_s);

  // This flips from SVG cord. space to OpenGL/world cord. space
  transform_ = new Matrix(1, 0, 0, -1, 0, height);

  printf("w = %f, h = %f\n", width, height);

  // Parse SVG paths into a temporary list
  path_list.clear();
  parse_svg(top, 0);

  skin->path_list = path_list;

  // Render to a GL display list
  GLuint displist = glGenLists(1);
  if(!displist) {
    std::cerr << "WARNING: Could not allocate displist in SVGparser::load()\n";
  }
  else {
    glNewList(displist, GL_COMPILE);
      glRotatef(180, 0, 0, 1);
      glScaled(scale, scale, scale);
      PathList::iterator x = skin->path_list.begin();
      while(x != skin->path_list.end()) {
        (*x)->render();
        ++x;
      }
    glEndList();
  }
  skin->displist = displist;

  return true;
}

//------------------------------------------------------------------------
// Traverse the XML element tree
//
void SVGparser::parse_svg(XMLNode node, int indent) {

  const char* name = node.getName();
  const char* transform = node.getAttribute("transform");

  Matrix *old_transform = transform_;
  transform_ = mul(transform_, new Matrix(transform?std::string(transform):std::string("")));

  // Parse current node
  fnmap::iterator iter;
  iter = vtable.find(name);
  if(iter != vtable.end()) {
    // Dispatch to the appropriate tag-handler
    (this->*(iter->second))(node);
  } else {
    //TODO debug log...
    if(svg_debug) printf("Ignoring '%s' element\n", name);
  }

  // Apply transform
  if(path) {
    path->transform = transform_;
    path->apply_transform();
    const char* id = (char*) node.getAttribute("id");
    // Capture skeleton
    if(std::string(id) == "skeleton") {
      skeleton = *path->loops.front();
    } else {
      color_style_(node);
      path_list.push_back(path);
    }
  }

  transform_ = old_transform;

  // Process child nodes
  int num_children = node.nChildNode();
  for(int i = 0; i < num_children; i++) {
    XMLNode child = node.getChildNode(i);
    parse_svg(child, indent+1);
  }

  path = NULL;

}

//------------------------------------------------------------------------
// SVG tag dispatch table
//
void SVGparser::vtable_init() {
  vtable["path"] = &SVGparser::parse_path_;
  vtable["rect"] = &SVGparser::parse_rect_;
  vtable["circle"] = &SVGparser::parse_circle_;
  vtable["radialGradient"] = &SVGparser::parse_radial_gradient_;
  vtable["linearGradient"] = &SVGparser::parse_linear_gradient_;
}

//------------------------------------------------------------------------
// SVG tag handlers
//
void SVGparser::parse_path_(XMLNode node) {
  p = (char *) node.getAttribute("d");
  path = new Path;
  pos.Set(0,0);
  while(parse_cmd());
  path->finish();
}

//------------------------------------------------------------------------

void SVGparser::parse_rect_(XMLNode node) {
  float32 x,y,w,h;
  x = (float32) atof(node.getAttribute("x"));
  y = (float32) atof(node.getAttribute("y"));
  w = (float32) atof(node.getAttribute("width"));
  h = (float32) atof(node.getAttribute("height"));
  // NOTE rx,ry (corner radius) attributes exist in our Inkscape SVGs, but they seem to be negligible.

  path = new Path;
  path->push(b2Vec2(x,y));
  path->push(b2Vec2(x+w,y));
  path->push(b2Vec2(x+w,y+h));
  path->push(b2Vec2(x,y+h));
  path->close();
}

void SVGparser::parse_circle_(XMLNode node) {
  const int segments = 24;
  const float increment = 2 * M_PI / segments;
  const float c = cos(increment);
  const float s = sin(increment);

  float cx,cy,r, x,y, t;
  cx = atof(node.getAttribute("cx"));
  cy = atof(node.getAttribute("cy"));
  r = atof(node.getAttribute("r"));

  // Bresenham circle algorithm?
  x = r;
  y = 0;
  path = new Path;
  for(int i=segments; i>0; i--) {
    path->push(b2Vec2(cx+x, cy+y));
    t = x;
    x = c * x - s * y;
    y = s * t + c * y;
  }
  path->close();
}

void SVGparser::color_style_(XMLNode node) {
  // Parse style; default method
  const char *style = node.getAttribute("style");
  if(style) {
    parse_style_(style);
    return;
  }

  // Older formats support individual style settings
  const char *opacity = node.getAttribute("opacity");
  const char *fill = node.getAttribute("fill");
  const char *stroke = node.getAttribute("stroke");
  const char *fill_opacity = node.getAttribute("fill-opacity");
  const char *stroke_opacity = node.getAttribute("stroke-opacity");

  if(fill) parse_color_(std::string(fill), path->fill);
  if(stroke) parse_color_(std::string(stroke), path->stroke);
}


void SVGparser::init_gradient_(XMLNode node, Gradient *gradient) {
  // Parse gradient stops
  int num_stops = node.nChildNode();
  for(int i = 0; i < num_stops; i++) {
    XMLNode child = node.getChildNode(i);
    const char *name = child.getName();
    const char *style = child.getAttribute("style");
    GLubyte *stop = new GLubyte[4];
    float32 offset = atof(std::string(child.getAttribute("offset")).c_str());
    if(style) {
      parse_stop_style_(style, stop);
    } else {
      const char *color = child.getAttribute("stop-color");
      parse_color_(color, stop);
      stop[3] = 255 * atof(std::string(child.getAttribute("stop-opacity")).c_str());
    }
    gradient->stops[offset] = stop;
  }

  // Parse gradient transform
  const char *transform = node.getAttribute("gradientTransform");
  // Use identity matrix if not defined
  Matrix *m = transform ? new Matrix(std::string(transform)) : new Matrix(1, 0, 0, 1, 0, 0);
  gradient->inv_transform = m->inverse();
  delete m;

  // Inherite gradient stops
  const char *xlink = node.getAttribute("xlink:href");
  if(xlink) {
    std::string href = std::string(xlink);
    href.erase(0,1);
    gradient->stops = gradients_[href]->stops;
  }

  // TODO: gradientUnits

}

void SVGparser::parse_radial_gradient_(XMLNode node) {

  const char* id = node.getAttribute("id");
  // TODO: Add subclass
  RadialGradient *gradient = new RadialGradient(id);
  init_gradient_(node, gradient);

  // Parse attributes
  const char* cx = node.getAttribute("cx");
  const char* cy = node.getAttribute("cy");
  const char* r = node.getAttribute("r");
  const char* fx = node.getAttribute("fx");
  const char* fy = node.getAttribute("fy");

  if(cx) gradient->cx = atof(std::string(cx).c_str());
  if(cy) gradient->cy = atof(std::string(cy).c_str());
  if(r) gradient->r = atof(std::string(r).c_str());
  //if(fx) gradient->fx = atof(std::string(fx).c_str());
  //if(fy) gradient->fy = atof(std::string(fy).c_str());

  gradients_[std::string(id)] = gradient;
}

void SVGparser::parse_linear_gradient_(XMLNode node) {
  const char* id = node.getAttribute("id");
  // TODO: Add subclass
  LinearGradient *gradient = new LinearGradient(id);
  init_gradient_(node, gradient);

  // Parse attributes
  const char* x1 = node.getAttribute("x1");
  const char* y1 = node.getAttribute("y1");
  const char* x2 = node.getAttribute("x2");
  const char* y2 = node.getAttribute("y2");

  if(x1) gradient->x1 = atof(std::string(x1).c_str());
  if(y1) gradient->y1 = atof(std::string(y1).c_str());
  if(x2) gradient->x2 = atof(std::string(x2).c_str());
  if(y2) gradient->y2 = atof(std::string(y2).c_str());

  gradients_[std::string(id)] = gradient;
}


//------------------------------------------------------------------------
// Parse a drawing command...
//
// TODO implement all commands
//
bool SVGparser::parse_cmd() {
  // Relative offset
  b2Vec2 offset;
  parse_ws_();
  switch (char c = *p++) {
    // End of input
    case '\0':
      return false;
    case 'M':
      //move, absolute
      while(parse_coord_pair_(&pos))
        path->push(pos);
      return true;
    case 'm':
      //move, relative
      while(parse_coord_pair_(&offset)) {
        pos += offset;
        path->push(pos);
      }
      return true;
    case 'L':
      //Line, absolute
      while (parse_coord_pair_(&pos))
        path->push(pos);
      return true;
    case 'l':
      // Line, relative
      while (parse_coord_pair_(&offset)) {
        pos += offset;
        path->push(pos);
      }
      return true;
    case 'A':
      parse_arc_(false);
      return true;
    case 'a':
      parse_arc_(true);
      return true;
    case 'C':
      // cubic Bézier curve, absolute
      curve_to_(false);
      return true;
    case 'c':
      // cubic Bézier curve, relative
      curve_to_(true);
      return true;
    case 'H': {
      float32 coord;
      // Horizontal lineto, absolute
      while (parse_coord_(&coord)) {
        pos.x = coord;
        path->push(pos);
      }
      return true;
    }
    case 'h': {
      float32 coord;
      // Horizontal lineto, relative
      while (parse_coord_(&coord)) {
        pos.x += coord;
        path->push(pos);
      }
      return true;
    }
    case 'V': {
      float32 coord;
      // Verticle lineto, absolute
      while (parse_coord_(&coord)) {
        pos.y = coord;
        path->push(pos);
      }
      return true;
    }
    case 'v': {
      float32 coord;
      // Verticle lineto, relative
      while (parse_coord_(&coord)) {
        pos.y += coord;
        path->push(pos);
      }
      return true;
    }
    case 'Z':
      // closepath
      // Fall through
    case 'z':
      // closepath
      path->close();
      return true;
    default:
      if(svg_debug) printf("Unimplemented/unknown SVGpath command '%c'\n", c);
      while(*p && !isalpha(*p)) p++;  // Skip the commmand
      return true;
      //return false;
  }
}

void SVGparser::parse_arc_(bool offset) {
  std::vector<float32> num;
  float32 n;
  while(parse_coord_(&n)) {
    num.push_back(n);
  }
  int k = 0;
  for(int j = 1; j <= num.size()/7; j++) {
    b2Vec2 radii = b2Vec2(num[k], num[k+1]);
    float32 phi = num[k+2];
    bool large_arc = num[k+3] == 1 ? true : false;
    bool sweep = num[k+4] == 1 ? true : false;
    b2Vec2 off = offset ? pos : b2Vec2(0,0);
    b2Vec2 end = off + b2Vec2(num[k+5], num[k+6]);
    arc_to_(radii, phi, large_arc, sweep, end);
    k+=7;
  }
}

void SVGparser::curve_to_(bool offset) {
  // Bezier points
  Vec2Vec bez;
  b2Vec2 point;
  while(parse_coord_pair_(&point)) {
    bez.push_back(point);
  }
  for(int i = 0; i < bez.size(); i+=3) {
    b2Vec2 off = offset ? pos : b2Vec2(0.0, 0.0);
    for (int j = 0; j < kBezierPoints; j++ ) {
      float32 t = (float32)j/(kBezierPoints - 1);
      cubic_bezier_(point, t, pos, bez[i]+off, bez[i+1]+off, bez[i+2]+off);
      path->push(point);
    }
    line_to_(bez[i+2].x+off.x, bez[i+2].y+off.y);
  }
}

/*
 * Elliptical arc
 * http://www.w3.org/TR/2003/REC-SVG11-20030114/implnote.html#ArcImplementationNotes
 * Ported from Squirtle
 */
void SVGparser::arc_to_(b2Vec2 radii, float32 phi, bool large_arc, bool sweep, b2Vec2 end) {
  float32 rx = radii.x;
  float32 ry = radii.y;
  float32 x1 = pos.x;
  float32 y1 = pos.y;
  float32 x2 = end.x;
  float32 y2 = end.y;
  float32 cp = cosf(phi);
  float32 sp = sinf(phi);
  float32 dx = .5 * (x1 - x2);
  float32 dy = .5 * (y1 - y2);
  float32 x_ = cp * dx + sp * dy;
  float32 y_ = -sp * dx + cp * dy;
  float32 r2 = (pow(rx * ry, 2) - pow(rx * y_,2) - pow(ry * x_,2)) /
              (pow(rx * y_, 2) + pow(ry * x_,2));
  if (r2 < 0) r2 = 0;
  float32 r = sqrt(r2);
  if (large_arc == sweep) r = -r;
  float32 cx_ = r * rx * y_ / ry;
  float32 cy_ = -r * ry * x_ / rx;
  float32 cx = cp * cx_ - sp * cy_ + .5 * (x1 + x2);
  float32 cy = sp * cx_ + cp * cy_ + .5 * (y1 + y2);

  float32 psi = angle_(b2Vec2(1,0), b2Vec2((x_ - cx_)/rx, (y_ - cy_)/ry));
  float32 delta = angle_(b2Vec2((x_ - cx_)/rx, (y_ - cy_)/ry),
                        b2Vec2((-x_ - cx_)/rx, (-y_ - cy_)/ry));
  if (sweep && delta < 0) delta += b2_pi * 2;
  if (!sweep && delta > 0) delta -= b2_pi * 2;
  int n_points = b2Max(int(b2Abs(kCirclePoints * delta / (2 * b2_pi))), 1);

  for(int i = 0; i < n_points; i++) {
      float32 theta = psi + i * delta / n_points;
      float32 ct = cos(theta);
      float32 st = sin(theta);
      line_to_(cp * rx * ct - sp * ry * st + cx,
               sp * rx * ct + cp * ry * st + cy);
  }
}

void SVGparser::parse_stop_style_(const char *style, GLubyte *stop) {
  std::string k,v;   // key, value
  const char *p, *start;
  p = start = style;
  while (*p) {
    if(*p == ':') {
      k = std::string(start, p - start);
      start = ++p;
      while (*p && *p!=';') ++p;
      v = std::string(start, p-start);
      if(k=="stop-color") {
        parse_color_(v, stop);
      } else if(k=="stop-opacity") {
        stop[3] = 255 * atof(v.c_str());
      }
      start = ++p;
      continue;
    }
    ++p;
  }
}

//------------------------------------------------------------------------
bool SVGparser::parse_style_(const char *style) {
  std::string k,v;   // key, value
  const char *p, *start;
  p = start = style;
  while (*p) {
    if(*p == ':') {
      k = std::string(start, p-start);
      start = ++p;
      while (*p && *p!=';') ++p;
      v = std::string(start, p-start);
      if(k=="fill") {
        parse_color_(v, path->fill);
      } else if(k=="stroke") {
        parse_color_(v, path->stroke);
      } else if(k=="fill-opacity") {
        path->fill[3] = 255 * atof(v.c_str());
      }
      start = ++p;
      continue;
    }
    ++p;
  }
  return true;
}

void SVGparser::parse_color_(std::string s, GLubyte *color) {
  GLubyte r, g, b = 0;
  if(s=="none") {
      return;
  } else if(s[0]=='#') {
    s = s.substr(1);
    int clr = strtol(s.c_str(), NULL, 16);
    switch(s.size()) {
      case 3:
        b = clr & 0xff;
        clr >>= 8;
        g = clr & 0xff;
        clr >>= 8;
        r = clr & 0xff;
        break;
      case 6:
        b = clr & 0xff;
        clr >>= 8;
        g = clr & 0xff;
        clr >>= 8;
        r = clr & 0xff;
        break;
      default:
        std::cerr << "Incorrect hex color length: '" << s << "'\n";
        return;
    }
  } else if(s[0] == 'u') {
    // Gradient color
    s.erase(0, 5);
    s.erase(s.size()-1, s.size()-1);
    //std::cout << s << std::endl;
    path->gradient = gradients_[s];
  }
  color[0] = r; color[1] = g; color[2] = b; color[3] = 255;
}

//------------------------------------------------------------------------
// Parse a single coordinate
//
bool SVGparser::parse_coord_(float32 *coord) {
  char *endpos;

  parse_ws_();
  *coord = strtod(p, &endpos);
  // If p is not a number then it must be a command, so exit
  if(p == endpos) return false;
  p = endpos;

  return true;
}

//------------------------------------------------------------------------
// Parse a coordinate pair
//
bool SVGparser::parse_coord_pair_(b2Vec2 *pos) {

  char *endpos;
  float32 x,y;

  parse_ws_();
  x = strtod(p, &endpos);
  // If p is not a number then it must be a command, so exit
  if(p == endpos) return false;
  p = endpos;

  parse_ws_();
  y = strtod(p, &endpos);
  p = endpos;

  pos->Set(x,y);
  return true;
}

//------------------------------------------------------------------------
