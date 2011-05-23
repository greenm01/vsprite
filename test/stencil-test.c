/* 2D Concave shape rendering test - using even-odd stencil buffer method
 *
 * First of all, it works fine with OpenGL's 3D features turned off.
 *
 * FACE CULLING was causing trouble, whether or not DEPTH TEST is on.
 * FIX: disable face culling when drawing 2D concave polygons.
 *
 * DEPTH TEST alone also seems problematic with multiple polygons, but I'll
 * save that for another day, as we don't really need it in mostly-2D games.
 *
 * Also discovered that stencil antialiasing wants the FULL opacity, not half!
 */

#define ANTIALIAS 1
#define LIGHT 1
#define DEPTH 1
#define CULL 0
#define TEXTURE 1

using namespace std;
#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <GL/glfw.h>

// Bastard case: draw the "bridge" shape of the Kzer-Za ship
//
// Hmm.. works here, I think there's something wrong with our SVG file...
// or rather, with my basic SVG parser, which _should_ (when finished)
// render anything that looks right in Inkscape or any other SVG renderer!
//
void draw_bridge() {
  glVertex2f(0,0);
  glVertex2f(2,0);
  glVertex2f(3,1);
  glVertex2f(9,1);
  glVertex2f(10,0);
  glVertex2f(12,0);
  glVertex2f(12,1);
  glVertex2f(9,4);
  glVertex2f(3,4);
  glVertex2f(0,1);
  glVertex2f(0,0);
}

void draw() {
  //
  // TODO test/debug the DEPTH_TEST stuff... disabling it for the stencil & outline *seems* to work but I haven't really studied how it interacts with STENCIL_TEST... see Red Book ch.10
  //
  // Fancy stencil buffer method
  // Draws filled concave polygons without tesselation
  //
  // References:
  //    "Drawing Filled, Concave Polygons Using the Stencil Buffer"
  //    OpenGL Red Book, Chapter 14
  //    http://glprogramming.com/red/chapter14.html#name13
  //
  //    "Hardware accelerated polygon rendering", Zack Rusin, 2006.
  //    http://zrusin.blogspot.com/2006/07/hardware-accelerated-polygon-rendering.html
  //

  glEnable (GL_STENCIL_TEST);

  // For each polygon:
  //i = loops.begin(); 
  //while(i != loops.end()) {

    GLubyte fill[4] = {0,255,255, 255};
    
    // Compute a bounding box (for drawing a filled quad behind the stencil)
    float x1,y1,x2,y2;
    x1=y1=0;
    x2=12;
    y2=4;

glDisable(GL_DEPTH_TEST);

    // Draw to stencil, using the even-odd rule for concave polygons
    glDisable (GL_BLEND);
    glStencilMask (0x01);
    glStencilOp (GL_KEEP, GL_KEEP, GL_INVERT);  // INVERT = even-odd rule
    glStencilFunc (GL_ALWAYS, 0, ~0);
    glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glBegin (GL_TRIANGLE_FAN);
      draw_bridge();
    glEnd ();
              
    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

#if ANTIALIAS
    // Antialiasing: Draw aliased off-pixels to real
    glEnable (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glStencilFunc (GL_EQUAL, 0x00, 0x01);
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);

    glEnable(GL_LINE_SMOOTH);
    glBegin (GL_LINE_LOOP);
    //glColor4ub(fill[0], fill[1], fill[2], fill[3]/2);  // Half-transparent
    glColor4ubv(fill);  // Actually, this works better...  SWEET
      draw_bridge();
    glEnd ();
    glDisable (GL_LINE_SMOOTH);
#endif

//glEnable(GL_DEPTH_TEST);

    // Draw fill
    glStencilFunc (GL_EQUAL, 0x01, 0x01);
    glStencilOp (GL_ZERO, GL_ZERO, GL_ZERO);

    glColor4ubv(fill);

#if 1
    //glBegin (GL_LINE_STRIP);
    glBegin (GL_POLYGON);
    // Draw a filled polygon, which may not render correctly if concave
    // - Seems to work
      draw_bridge();
    glEnd ();
#else
    // Draw a filled quad behind the stencil
    // - Seems to work
    glBegin(GL_QUADS);
      glVertex2f(x1,y1);
      glVertex2f(x2,y1);
      glVertex2f(x2,y2);
      glVertex2f(x1,y2);
    glEnd ();
#endif

glEnable(GL_DEPTH_TEST);

    //++i;
  //}
  glDisable (GL_STENCIL_TEST);
}


void GLFWCALL resize_view(int w, int h) {
  cout << "Resize to " << w << "x" << h << endl;

  glViewport( 0,0, w,h );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  //glOrtho(0, S-1, 0, S-1, -1., 1.);
  glOrtho(0, w-1, 0, h-1, -200., 2000.);

  // Go back to model mode for drawing!
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}


int main(int argc, char **argv) {
  glfwInit();

  if( !glfwOpenWindow( 800, 600, 0,0,0,0, 24,1, GLFW_WINDOW ) )
  {
    glfwTerminate();
    return 0;
  }

  printf("Depth bits: %d\n", glfwGetWindowParam(GLFW_DEPTH_BITS));
  printf("Stencil bits: %d\n", glfwGetWindowParam(GLFW_STENCIL_BITS));

  glfwSwapInterval( 1 ); // Enable vsync
  glfwSetWindowSizeCallback(resize_view);

  //================================================================
  // GL settings
  //
  glDisable(GL_DITHER);
  //glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if LIGHT
  GLfloat pos[4] = {500,500,1000,0};
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
#endif

  //glEnable(GL_RESCALE_NORMAL);  // Rescale _unit length_ normals  XXX not in win32...
  //glEnable(GL_NORMALIZE);     // Rescale _any length_ normals (slower)

#if CULL
  glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);          // (this is the default)
  //glCullFace(GL_FRONT_AND_BACK);
#endif

#if DEPTH
  glEnable(GL_DEPTH_TEST);
#endif

#if TEXTURE
  glEnable(GL_TEXTURE_2D);
#endif

  glClearColor(0.0, 0.0, 0.0, 0.0);

  //================================================================
  // Draw
  //

  while( !glfwGetKey( GLFW_KEY_ESC )
      && glfwGetWindowParam(GLFW_OPENED) )
  {
    glClear(GL_COLOR_BUFFER_BIT
        | GL_DEPTH_BUFFER_BIT
        | GL_STENCIL_BUFFER_BIT
        );
    glLoadIdentity();

    glTranslatef(300, 200, 0);
    glScalef(20,20,20);

    draw();
    glRotatef(133.33, 0,0,1);
    draw();

    glfwSwapBuffers();
  }

  glfwTerminate();

  return 0;
}
