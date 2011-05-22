#ifndef SKIN_H
#define SKIN_H

extern "C" {
#include <lua5.1/lua.h>      // lua_XX functions
#include <lua5.1/lauxlib.h>  // luaL_XX
}
#include "../luna.h"

#include <vector>
#include <GL/glfw.h>
#include <Box2D/Box2D.h>

#include "path.h"

//========================================================================
// A complete "skin" for an Actor
//
class Skin {
public:
  GLuint displist;
  b2Vec2 size;
  PathList path_list;
  /// Triangle list
  std::vector<b2Vec2*> triangles;
  /// Skeleton vertices
  Vec2list skeleton;    
  /// Scaling factor -> pixels to MKS units
  double scale;         

  float bbx1, bby1, bbx2, bby2;   // temporaries

  Skin(lua_State *L);
  ~Skin();

  void compute_bbox();
  void init_skeleton();
  
  // Lua API
  int draw(lua_State *L);

  // LunaWrapper boilerplate
  static const char className[];
  static const Luna<Skin>::RegType Register[];
};

#endif
