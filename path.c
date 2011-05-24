// In SVG terminology, a "path" is composed of one OR MORE "loops"
// (polylines, open or closed), all with the same color properties.

#include "path.h"

//------------------------------------------------------------------------
// let's have 'curloop' be a utarray of Vec2's
// we'll push points onto it as we parse them,
// then push 'curloop' onto 'curpath'.
// When we're all done, convert those to standard C arrays...
//
UT_array *curpath;
UT_array *curloop;

UT_icd ptr_icd = {sizeof(void*), NULL, NULL, NULL};

void curpath_reset() { utarray_new(curpath, &ptr_icd); }
void curloop_reset() { utarray_new(curloop, &ptr_icd); }

//------------------------------------------------------------------------
// Instantiate a new Path object
//
Path* path_new() {
  Path *p = (Path*)malloc(sizeof(Path));
  curpath_reset();
  curloop_reset();
  p->opacity=1;
  p->fill[0]=0; p->fill[1]=0; p->fill[2]=0; p->fill[3]=0;
  p->stroke[0]=0; p->stroke[1]=0; p->stroke[2]=0; p->stroke[3]=0;
  //p->gradient=NULL;
  //p->transform=NULL;
  return p;
}

//------------------------------------------------------------------------
// Start a new loop (push the old one)
//
// XXX RENAME path_end_loop?
void path_finish(Path *p) {
  int n = utarray_len(curloop);
  if(n) {
    //TODO reduce vertices... see Squirtle .end_path()

    utarray_push_back(curpath, curloop);

    p->nloops = n;
    // p->loops = TODO:convert curpath to standard C array
  }
}

//------------------------------------------------------------------------
// Close the current loop, and start a new loop
//
// XXX RENAME path_close_loop?
void path_close(Path *p) {
  utarray_push_back(curloop, utarray_front(curloop));
  path_finish(p);
}

//------------------------------------------------------------------------
void path_dump(Path *p) {
  int n=0;
  //Vec2lol::iterator i = loops.begin();
  //while(i != loops.end()) {
  //  printf("LOOP%d= \n", ++n);
  //  Vec2list::iterator j = (*i)->begin();
  //  while(j != (*i)->end()) {
  //    printf(" %f,%f\n", j->x, j->y);
  //    ++j;
  //  }
  //  printf("\n");
  //  ++i;
  //}
}

//------------------------------------------------------------------------
void path_render(Path *p) {
#if 0
  //
  // Simple version - just draw the outline
  //
  printf("Rendering");
  Vec2lol::iterator i = loops.begin();
  while(i != loops.end()) {
    glBegin(GL_LINE_STRIP);
    Vec2list::iterator j = (*i)->begin();
    while(j != (*i)->end()) {
      glVertex2f(j->x, j->y);
      ++j;
      printf(".");
    }
    glEnd();
    ++i;
  }
  printf("\n");

#else

  //
  // Fancy stencil buffer method
  // Draws filled concave polygons without tesselation
  //
  // Incompatibilities with certain OpenGL features:
  //   - GL_CULL_FACE interferes with even-odd rule, causing incorrect shapes.
  //   - GL_DEPTH_TEST prevents (some/all) fills from rendering.
  //
  // References:
  //    "Drawing Filled, Concave Polygons Using the Stencil Buffer"
  //    OpenGL Red Book, Chapter 14
  //    http://glprogramming.com/red/chapter14.html#name13
  //
  //    "Hardware accelerated polygon rendering", Zack Rusin, 2006.
  //    http://zrusin.blogspot.com/2006/07/hardware-accelerated-polygon-rendering.html
  //

  int l, i;                     // Loop#, Point#
  Loop *loop = p->loops[0];     // current loop
  Vec2 v;                       // current point

  glPushAttrib(GL_ENABLE_BIT);

  glEnable(GL_STENCIL_TEST);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);

  // Compute a bounding box (for drawing a filled quad behind the stencil)
  float x1,y1,x2,y2;
  v = loop->points[0];
  x1 = x2 = v.x;
  y1 = y2 = v.y;
  for(i=0; i < loop->npoints; i++) {
    v = loop->points[i];
    if(v.x < x1) x1=v.x;
    if(v.y < y1) y1=v.y;
    if(v.x > x2) x2=v.x;
    if(v.y > y2) y2=v.y;
  }

  // Draw to stencil, using the even-odd rule for concave polygons
  glDisable(GL_BLEND);
  glStencilMask(0x01);
  glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);  // INVERT = even-odd rule
  glStencilFunc(GL_ALWAYS, 0, ~0);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  for(l=0; l < p->nloops; l++) {
    loop = p->loops[l];
    glBegin(GL_TRIANGLE_FAN);
      for(i=0; i < loop->npoints; i++) {
        v = loop->points[i];
        glVertex2f(v.x, v.y);
      }
    glEnd ();
  }

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  // Antialiasing: Draw aliased off-pixels to real
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glStencilFunc(GL_EQUAL, 0x00, 0x01);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

  // Draw outline if necessary
  if(p->stroke[3] != 0) {
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2);
    for(l=0; l < p->nloops; l++) {
      loop = p->loops[l];
      glBegin(GL_LINE_LOOP);
      glColor4ubv(p->stroke);
      for(i=0; i < loop->npoints; i++) {
        v = loop->points[i];
        glVertex2f(v.x, v.y);
      }
      glEnd();
    }
    glDisable(GL_LINE_SMOOTH);
  }

  // Draw a filled quad behind the stencil
  glStencilFunc(GL_EQUAL, 0x01, 0x01);
  glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

  glBegin(GL_QUADS);
    GLubyte *color;
    if(p->gradient) {
      color = gradient_interp(p->gradient,x1,y1);
      glColor4ubv(color);
      glVertex2f(x1,y1);
      color = gradient_interp(p->gradient,x2,y1);
      glColor4ubv(color);
      glVertex2f(x2,y1);
      color = gradient_interp(p->gradient,x2,y2);
      glColor4ubv(color);
      glVertex2f(x2,y2);
      color = gradient_interp(p->gradient,x1,y2);
      glColor4ubv(color);
      glVertex2f(x1,y2);
    } else {
      color = p->fill;
      glColor4ubv(color);
      glVertex2f(x1,y1);
      glVertex2f(x2,y1);
      glVertex2f(x2,y2);
      glVertex2f(x1,y2);
    }
  glEnd();

  glPopAttrib();
#endif
}

//------------------------------------------------------------------------
inline void path_apply_transform(Path *p) {
  // Transform points if necessary
  if(p->transform) {
    int l, i;
    Loop *loop;
    Vec2 v;
    for(l=0; l < p->nloops; l++) {
      loop = p->loops[l];
      for(i=0; i < loop->npoints; i++) {
        loop->points[i] = matrix_xform(p->transform, loop->points[i]);
      }
    }
  }
}

//------------------------------------------------------------------------
inline void path_translate(Path *p, Vec2 center) {
  // Transform points if necessary
  int l, i;
  Loop *loop;
  Vec2 v;
  for(l=0; l < p->nloops; l++) {
    loop = p->loops[l];
    for(i=0; i < loop->npoints; i++) {
      loop->points[i].x -= center.x;
      loop->points[i].y -= center.y;
    }
  }
}

//------------------------------------------------------------------------
