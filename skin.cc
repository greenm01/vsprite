#include "skin.h"

using namespace std;
#include <iostream>
#include <vector>
#include "../poly2tri/poly2tri.h"
#include "../util.h"
#include "svgparser.h"

static float32 SignedPolygonArea(std::vector<b2Vec2> vs) {
	
  int i,j;
	float32 area = 0;
  int N = vs.size();
  
	for (i=0;i<N;i++) {
		j = (i + 1) % N;
		area += vs[i].x * vs[j].y;
		area -= vs[i].y * vs[j].x;
	}
	area /= 2.0;
  return(area);
}

static b2Vec2 ComputeCentroid(std::vector<b2Vec2> vs) {

  float32 cx=0,cy=0;
	float32 A = SignedPolygonArea(vs);
	b2Vec2 res;
	int i,j;

	float32 factor=0;
  int count = vs.size();
	for (i=0;i<count;i++) {
		j = (i + 1) % count;
		factor=(vs[i].x*vs[j].y-vs[j].x*vs[i].y);
		cx+=(vs[i].x+vs[j].x)*factor;
		cy+=(vs[i].y+vs[j].y)*factor;
	}
	A*=6.0f;
	factor=1/A;
	cx*=factor;
	cy*=factor;
	res.x=cx;
	res.y=cy;
	return res;
}

//========================================================================
//                               LUA API
//========================================================================

// LunaWrapper boilerplate
const char Skin::className[] = "Skin";
const Luna<Skin>::RegType Skin::Register[] = {
  {"draw",              &Skin::draw},
  {NULL, NULL}
};

//------------------------------------------------------------------------
// Lua usage:  skin = Skin(filename, scale)
//
Skin::Skin(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  float32 scale = luaL_checknumber(L, 2);

  cout << "-- new Skin('" << filename << "', " << scale << ")\n";
  SVGparser *svgparser = new SVGparser();
  svgparser->load(filename, scale, this);
  //cout << "new Skin... ptr= " << this << "\n";
  //cout << "-- end Skin()\n";

  // Returns self on Lua stack
}

//------------------------------------------------------------------------
Skin::~Skin() {
  cout << "-- in ~Skin(" << this << ")\n";
}

//------------------------------------------------------------------------
void Skin::compute_bbox() {
  bbx1 = bby1 = +INFINITY;
  bbx2 = bby2 = -INFINITY;

  PathList::iterator p;         // path
  Vec2lol::iterator i;          // loop
  Vec2list::iterator j;         // vertex

  p = path_list.begin();
  while(p != path_list.end()) {
    Vec2lol loops = (*p)->loops;
    i = loops.begin();
    while(i != loops.end()) {
      Vec2list *vertices = *i;
      j = vertices->begin();
      while(j != vertices->end()) {
        b2Vec2 v = *j;
        if(v.x < bbx1) bbx1 = v.x;
        if(v.x > bbx2) bbx2 = v.x;
        if(v.y < bby1) bby1 = v.y;
        if(v.y > bby2) bby2 = v.y;
        ++j;
      }
      ++i;
    }
    ++p;
  }
  printf("-- bbox: %.0f,%.0f %.0f %.0f\n", bbx1,bby1, bbx2,bby2);
}

void Skin::init_skeleton() {

  std::vector<b2Vec2> points(skeleton.begin(), skeleton.end());
  b2Vec2 centroid = ComputeCentroid(points);
    
  using namespace p2t;
  vector<p2t::Point*> polyline;
  
  Vec2list::iterator v;
  v = skeleton.begin(); 
  // translate skeleton
  while(v != skeleton.end()) {
    v->x = (v->x - centroid.x) * scale;
    v->y = (v->y - centroid.y) * scale;
    polyline.push_back(new Point(v->x, v->y));
    ++v;
  }
  
  // translate path list
  PathList::iterator x = path_list.begin();
  while(x != path_list.end()) {
    (*x)->translate(centroid);
    ++x;
  }
  
  // Triangulate
  CDT* cdt = new CDT(polyline);
  cdt->Triangulate();

  vector<Triangle*> tlist = cdt->GetTriangles();

  // convert from poly2tri to box2d verts
  for(unsigned int i = 0; i < tlist.size(); i++) {
    Point *a = tlist[i]->GetPoint(0);
    Point *b = tlist[i]->GetPoint(1);
    Point *c = tlist[i]->GetPoint(2);
    b2Vec2 p1 = b2Vec2(a->x, a->y);
    b2Vec2 p2 = b2Vec2(b->x, b->y);
    b2Vec2 p3 = b2Vec2(c->x, c->y);
    bool ccw = triangle_ccw(p1, p2, p3);
    if(!ccw) {
      b2Vec2 temp = p3;
      p3 = p2;
      p2 = temp;
    }
    b2Vec2 *triangle = new b2Vec2[3];
    triangle[0] = p1; triangle[1] = p2; triangle[2] = p3;
    triangles.push_back(triangle);
  }

  delete cdt;

}

//------------------------------------------------------------------------
// LUA USAGE: skin:draw(x,y, angle, scale)
int Skin::draw(lua_State *L) {
  if(!displist) return 0;

  float32 x = luaL_checknumber(L, 2);
  float32 y = luaL_checknumber(L, 3);
  float32 a = luaL_checknumber(L, 4);
  float32 s = luaL_checknumber(L, 5);

  glPushMatrix();
    glTranslated(x, y, 0);
    glScaled(s, s, 0);
    glRotatef(degrees(a), 0, 0, 1);
    glCallList(displist);
  glPopMatrix();
  return 0;
}
//------------------------------------------------------------------------
