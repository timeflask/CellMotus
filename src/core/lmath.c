#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lmath.h"



#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

void
vec4_set(vec4* self, vec4* vec)
{
  memcpy(self->data, vec->data, sizeof(self->data));
}


mat4 *
mat4_new( void )
{
    mat4 *self = (mat4 *) malloc( sizeof(mat4) );
    return self;

}

void
mat4_set_zero( mat4 *self )
{
    assert( self );

    memset( self, 0, sizeof( mat4 ));
}

void
mat4_set_identity( mat4 *self )
{
    assert( self );

    memset( self, 0, sizeof( mat4 ));
    self->m00 = 1.0;
    self->m11 = 1.0;
    self->m22 = 1.0;
    self->m33 = 1.0;
}

void
mat4_multiply( mat4 *self, mat4 *other )
{
    mat4 m;
    size_t i;

    assert( self );
    assert( other );

    for( i=0; i<4; ++i )
    {
        m.data[i*4+0] =
            (self->data[i*4+0] * other->data[0*4+0]) +
            (self->data[i*4+1] * other->data[1*4+0]) +
            (self->data[i*4+2] * other->data[2*4+0]) +
            (self->data[i*4+3] * other->data[3*4+0]) ;

        m.data[i*4+1] =
            (self->data[i*4+0] * other->data[0*4+1]) +
            (self->data[i*4+1] * other->data[1*4+1]) +
            (self->data[i*4+2] * other->data[2*4+1]) +
            (self->data[i*4+3] * other->data[3*4+1]) ;

        m.data[i*4+2] =
            (self->data[i*4+0] * other->data[0*4+2]) +
            (self->data[i*4+1] * other->data[1*4+2]) +
            (self->data[i*4+2] * other->data[2*4+2]) +
            (self->data[i*4+3] * other->data[3*4+2]) ;

        m.data[i*4+3] =
            (self->data[i*4+0] * other->data[0*4+3]) +
            (self->data[i*4+1] * other->data[1*4+3]) +
            (self->data[i*4+2] * other->data[2*4+3]) +
            (self->data[i*4+3] * other->data[3*4+3]) ;
    }
    memcpy( self, &m, sizeof( mat4 ) );

}

void
mat4_multiply2( const mat4 *self, const mat4 *other, mat4* res)
{
  size_t i;


  for( i=0; i<4; ++i )
  {
      res->data[i*4+0] =
          (self->data[i*4+0] * other->data[0*4+0]) +
          (self->data[i*4+1] * other->data[1*4+0]) +
          (self->data[i*4+2] * other->data[2*4+0]) +
          (self->data[i*4+3] * other->data[3*4+0]) ;

      res->data[i*4+1] =
          (self->data[i*4+0] * other->data[0*4+1]) +
          (self->data[i*4+1] * other->data[1*4+1]) +
          (self->data[i*4+2] * other->data[2*4+1]) +
          (self->data[i*4+3] * other->data[3*4+1]) ;

      res->data[i*4+2] =
          (self->data[i*4+0] * other->data[0*4+2]) +
          (self->data[i*4+1] * other->data[1*4+2]) +
          (self->data[i*4+2] * other->data[2*4+2]) +
          (self->data[i*4+3] * other->data[3*4+2]) ;

      res->data[i*4+3] =
          (self->data[i*4+0] * other->data[0*4+3]) +
          (self->data[i*4+1] * other->data[1*4+3]) +
          (self->data[i*4+2] * other->data[2*4+3]) +
          (self->data[i*4+3] * other->data[3*4+3]) ;
  }
}


void
mat4_set_orthographic( mat4 *self,
                       float left,   float right,
                       float bottom, float top,
                       float znear,  float zfar )
{
    assert( self );
    assert( right  != left );
    assert( bottom != top  );
    assert( znear  != zfar );

    mat4_set_zero( self );

    self->m00 = +2.0/(right-left);
    self->m30 = -(right+left)/(right-left);
    self->m11 = +2.0/(top-bottom);
    self->m31 = -(top+bottom)/(top-bottom);
    self->m22 = -2.0/(zfar-znear);
    self->m32 = -(zfar+znear)/(zfar-znear);
    self->m33 = 1.0;
}

void
mat4_set_perspective( mat4 *self,
                      float fovy,  float aspect,
                      float znear, float zfar)
{
    float h, w;

    assert( self );
    assert( znear != zfar );

    h = tan(fovy / 360.0 * M_PI) * znear;
    w = h * aspect;

    mat4_set_frustum( self, -w, w, -h, h, znear, zfar );
}

void
mat4_set_frustum( mat4 *self,
                  float left,   float right,
                  float bottom, float top,
                  float znear,  float zfar )
{

    assert( self );
    assert( right  != left );
    assert( bottom != top  );
    assert( znear  != zfar );

    mat4_set_zero( self );

    self->m00 = +2.0*znear/(right-left);
    self->m20 = (right+left)/(right-left);

    self->m11 = +2.0*znear/(top-bottom);
    self->m31 = (top+bottom)/(top-bottom);

    self->m22 = -(zfar+znear)/(zfar-znear);
    self->m32 = -2.0*znear/(zfar-znear);

    self->m23 = -1.0;
}

void
mat4_set_rotation( mat4 *self,
                   float angle,
                   float x, float y, float z)
{
    float c, s, norm;

    assert( self );

    c = cos( M_PI*angle/180.0 );
    s = sin( M_PI*angle/180.0 );
    norm = sqrt(x*x+y*y+z*z);

    x /= norm; y /= norm; z /= norm;

    mat4_set_identity( self );

    self->m00 = x*x*(1-c)+c;
    self->m10 = y*x*(1-c)-z*s;
    self->m20 = z*x*(1-c)+y*s;

    self->m01 =  x*y*(1-c)+z*s;
    self->m11 =  y*y*(1-c)+c;
    self->m21 =  z*y*(1-c)-x*s;

    self->m02 = x*z*(1-c)-y*s;
    self->m12 = y*z*(1-c)+x*s;
    self->m22 = z*z*(1-c)+c;
}

void
mat4_set_translation( mat4 *self,
                      float x, float y, float z)
{
    assert( self );

    mat4_set_identity( self );
    self-> m30 = x;
    self-> m31 = y;
    self-> m32 = z;
}

void
mat4_set_scaling( mat4 *self,
                  float x, float y, float z)
{
    assert( self );

    mat4_set_identity( self );
    self-> m00 = x;
    self-> m11 = y;
    self-> m22 = z;
}

void
mat4_rotate( mat4 *self,
             float angle,
             float x, float y, float z)
{
    mat4 m;

    assert( self );

    mat4_set_rotation( &m, angle, x, y, z);
    mat4_multiply( self, &m );
}

void
mat4_translate( mat4 *self,
                float x, float y, float z)
{
    mat4 m;
    assert( self );

    mat4_set_translation( &m, x, y, z);
    mat4_multiply( self, &m );
}

void
mat4_scale( mat4 *self,
            float x, float y, float z)
{
    mat4 m;
    assert( self );

    mat4_set_scaling( &m, x, y, z);
    mat4_multiply( self, &m );
}

void
mat4_set( mat4 *self,
          float data[])
{
  memcpy(self->data, data, sizeof(float)*16);
}

void v4_transform(const float* m, const float* v, float* dst)
{
  /*
  asm volatile
  (
   "vld1.32    {d0, d1}, [%1]     \n\t"   // V[x, y, z, w]
   "vld1.32    {d18 - d21}, [%2]! \n\t"   // M[m0-m7]
   "vld1.32    {d22 - d25}, [%2]  \n\t"    // M[m8-m15]

   "vmul.f32   q13, q9, d0[0]     \n\t"   // DST->V = M[m0-m3] * V[x]
   "vmla.f32   q13, q10, d0[1]    \n\t"   // DST->V = M[m4-m7] * V[y]
   "vmla.f32   q13, q11, d1[0]    \n\t"   // DST->V = M[m8-m11] * V[z]
   "vmla.f32   q13, q12, d1[1]    \n\t"   // DST->V = M[m12-m15] * V[w]

   "vst1.32    {d26, d27}, [%0]   \n\t"   // DST->V
   :
   : "r"(dst), "r"(v), "r"(m)
   : "q0", "q9", "q10","q11", "q12", "q13", "memory"
   );
   */

  // Handle case where v == dst.
    float x = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
    float y = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
    float z = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
    float w = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];

    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;

}
