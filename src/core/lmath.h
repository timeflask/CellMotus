#ifndef __lmath_H_
#define __lmath_H_

typedef union
{
  int data[4];    /**< All compoments at once     */
  struct {
        int x;      /**< Alias for first component  */
        int y;      /**< Alias for second component */
        int z;      /**< Alias for third component  */
        int w;      /**< Alias for fourht component */
    };
  struct {
        int x_;     /**< Alias for first component  */
        int y_;     /**< Alias for second component */
        int width;  /**< Alias for third component  */
        int height; /**< Alias for fourth component */
    };
  struct {
        int r;      /**< Alias for first component  */
        int g;      /**< Alias for second component */
        int b;      /**< Alias for third component  */
        int a;      /**< Alias for fourth component */
    };
  struct {
        int red;    /**< Alias for first component  */
        int green;  /**< Alias for second component */
        int blue;   /**< Alias for third component  */
        int alpha;  /**< Alias for fourth component */
    };
  struct {
        int vstart; /**< Alias for first component  */
        int vcount; /**< Alias for second component */
        int istart; /**< Alias for third component  */
        int icount; /**< Alias for fourth component */
    };
} ivec4;


typedef union
{
  int data[3];    /**< All compoments at once     */
  struct {
        int x;      /**< Alias for first component  */
        int y;      /**< Alias for second component */
        int z;      /**< Alias for third component  */
    };
  struct {
        int r;      /**< Alias for first component  */
        int g;      /**< Alias for second component */
        int b;      /**< Alias for third component  */
    };
  struct {
        int red;    /**< Alias for first component  */
        int green;  /**< Alias for second component */
        int blue;   /**< Alias for third component  */
    };
} ivec3;


typedef union
{
  int data[2];    /**< All compoments at once     */
  struct {
        int x;      /**< Alias for first component  */
        int y;      /**< Alias for second component */
    };
  struct {
        int s;      /**< Alias for first component  */
        int t;      /**< Alias for second component */
    };
  struct {
        int start;  /**< Alias for first component  */
        int end;    /**< Alias for second component */
    };
} ivec2;


typedef union
{
  float data[4];
  struct { float x; float y; float z;float w; };
  struct {
        float x_;
        float y_;
        float width;
        float height;
    };
  struct {
        float r;
        float g;
        float b;
        float a;
    };
  struct {
        float red;
        float green;
        float blue;
        float alpha;
    };
  struct {
        float s0;
        float t0;
        float s1;
        float t1;
    };
} vec4;


typedef union
{
  float data[3];   /**< All compoments at once    */
  struct {
        float x;     /**< Alias for first component */
        float y;     /**< Alias fo second component */
        float z;     /**< Alias fo third component  */
    };
  struct {
        float r;     /**< Alias for first component */
        float g;     /**< Alias fo second component */
        float b;     /**< Alias fo third component  */
    };
  struct {
        float red;   /**< Alias for first component */
        float green; /**< Alias fo second component */
        float blue;  /**< Alias fo third component  */
    };
} vec3;

typedef union
{
  float data[2];
  struct {
        float x;
        float y;
    };
  struct {
        float s;
        float t;
    };
  struct {
        float w;
        float h;
    };
} vec2;

typedef union
{
  float data[16];    /**< All compoments at once     */
  struct {
        float m00, m01, m02, m03;
        float m10, m11, m12, m13;
        float m20, m21, m22, m23;
        float m30, m31, m32, m33;
    };
} mat4;

void
vec4_set(vec4* self, vec4* vec);

mat4 *
mat4_new( void );

void
mat4_set_identity( mat4 *self );

void
mat4_set_zero( mat4 *self );

void
mat4_multiply( mat4 *self, mat4 *other );

void
mat4_multiply2( const mat4 *m1, const mat4 *m2, mat4* res);

void
mat4_set_orthographic( mat4 *self,
                       float left,   float right,
                       float bottom, float top,
                       float znear,  float zfar );

void
mat4_set_perspective( mat4 *self,
                      float fovy,  float aspect,
                      float zNear, float zFar);

void
mat4_set_frustum( mat4 *self,
                  float left,   float right,
                  float bottom, float top,
                  float znear,  float zfar );

void
mat4_set_rotation( mat4 *self,
                   float angle,
                   float x, float y, float z);

void
mat4_set_translation( mat4 *self,
                      float x, float y, float z);

void
mat4_set_scaling( mat4 *self,
                  float x, float y, float z);

void
mat4_rotate( mat4 *self,
             float angle,
             float x, float y, float z);

void
mat4_translate( mat4 *self,
                float x, float y, float z);

void
mat4_scale( mat4 *self,
            float x, float y, float z);

void
mat4_set( mat4 *self,
          float data[]);

void v4_transform(const float* m, const float* v, float* dst);


#endif
