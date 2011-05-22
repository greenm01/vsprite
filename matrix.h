#ifndef MATRIX_H
#define MATRIX_H

#include <string>
#include <cstdio>
#include <Box2D/Box2D.h>

class Matrix {
public:
  float32 val[6];

  // Constructors
  Matrix(std::string m);

  Matrix(float32 v1, float32 v2, float32 v3, float32 v4, float32 v5, float32 v6) {
    val[0] = v1; val[1] = v2; val[2] = v3;
    val[3] = v4; val[4] = v5; val[5] = v6;
  }

  // Inverse
  Matrix *inverse();
  b2Vec2 xform(b2Vec2 v);
  b2Vec2 call(b2Vec2 pt);

  void dump() {
    printf("%f,%f,%f,%f,%f,%f\n", val[0],val[1],val[2],val[3],val[4],val[5]);
  }
};


inline b2Vec2 Matrix::call(b2Vec2 pt) {
  return b2Vec2(val[0]*pt.x + val[2]*pt.y + val[4],
                val[1]*pt.x + val[3]*pt.y + val[5]);
}

inline Matrix *Matrix::inverse() {
  float32 d = val[0]*val[3] - val[1]*val[2];
  return new Matrix(val[3]/d, -val[1]/d, -val[2]/d, val[0]/d,
                   (val[2]*val[5] - val[3]*val[4])/d,
                   (val[1]*val[4] - val[0]*val[5])/d);
}

inline b2Vec2 Matrix::xform(b2Vec2 v) {
  return b2Vec2(val[0]*v.x + val[2]*v.y + val[4], val[1]*v.x + val[3]*v.y + val[5]);
}


inline Matrix * mul(Matrix *m1, Matrix *m2) {
  float32 a = m1->val[0]; float32 b = m1->val[1]; float32 c = m1->val[2];
  float32 d = m1->val[3]; float32 e = m1->val[4]; float32 f = m1->val[5];
  float32 u = m2->val[0]; float32 v = m2->val[1]; float32 w = m2->val[2];
  float32 x = m2->val[3]; float32 y = m2->val[4]; float32 z = m2->val[5];
  return new Matrix(a*u + c*v, b*u + d*v, a*w + c*x, b*w + d*x, a*y + c*z + e, b*y + d*z + f);
}

#endif
