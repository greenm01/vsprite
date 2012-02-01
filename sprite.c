// sprite.c

#include <stdio.h>
#include <math.h>
#include "sprite.h"
//#include "svgparser.h"

void compute_bbox(Sprite *sprite, float32 scale);

//------------------------------------------------------------------------
Sprite* sprite_new(const char *filename, float32 scale) {
  printf("-- new Skin('%s', %f)\n", filename, scale);
  Sprite *sprite = (Sprite*)malloc(sizeof(Sprite));
  //SVGparser *svgparser = new SVGparser();
  svg_load(filename, scale, sprite);
  compute_bbox(sprite, scale);
  sprite->scale = scale;
  return sprite;
}

//------------------------------------------------------------------------
void compute_bbox(Sprite *sprite, float32 scale) {
  float bbx1, bby1, bbx2, bby2;

  bbx1 = bby1 = +INFINITY;
  bbx2 = bby2 = -INFINITY;

#ifdef CPP  //TODO rewrite in C *after* design decisions...
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
#endif
  printf("-- bbox: %.0f,%.0f %.0f %.0f\n", bbx1,bby1, bbx2,bby2);

  /*
  // Bounding box calcs... downsize and center
  float w,h;
  w = bbx2 - bbx1;
  h = bby2 - bby1;
  */
  
  sprite->width *= scale;
  sprite->height *= scale;
}

//------------------------------------------------------------------------
int draw_sprite(Sprite *sprite, float32 x, float32 y, float32 angle, float32 scale) {
  if(!sprite->displist) return 0;

  glPushMatrix();
    glTranslated(x, y, 0);
    glScaled(scale, scale, 0);
    glRotatef(degrees(angle), 0, 0, 1);
    glCallList(sprite->displist);
  glPopMatrix();
  return 0;
}

//------------------------------------------------------------------------
