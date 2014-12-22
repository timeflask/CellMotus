#ifndef __VERTEX_BUFFER_H__
#define __VERTEX_BUFFER_H__
#include "opengl.h"
#include "vector.h"
#include "vertex-attribute.h"


typedef GLushort vb_index_t;
#define SEN_GL_INDEX_TYPE GL_UNSIGNED_SHORT

typedef struct
{
    char * format;

    vector_t * vertices;
    GLuint vertices_id;
    vector_t * indices;
    GLuint indices_id;
    size_t GPU_vsize;
    size_t GPU_isize;
    GLenum mode;
    char state;
    vector_t * items;
    vertex_attribute_t *attributes[MAX_VERTEX_ATTRIBUTE];
} vertex_buffer_t;

  vertex_buffer_t *
  vertex_buffer_new( const char *format );

  void
  vertex_buffer_delete( vertex_buffer_t * self );

  size_t
  vertex_buffer_size( const vertex_buffer_t *self );

  const char *
  vertex_buffer_format( const vertex_buffer_t *self );


  void
  vertex_buffer_print( vertex_buffer_t * self );


  void
  vertex_buffer_render_setup ( vertex_buffer_t *self,
                               GLenum mode );
  void
  vertex_buffer_render_finish ( vertex_buffer_t *self );


  void
  vertex_buffer_render ( vertex_buffer_t *self,
                         GLenum mode );


  void
  vertex_buffer_render_item ( vertex_buffer_t *self,
                              size_t index );
  void
  vertex_buffer_upload( vertex_buffer_t *self );


  void
  vertex_buffer_clear( vertex_buffer_t *self );

  void
  vertex_buffer_push_back_indices ( vertex_buffer_t *self,
                                    const vb_index_t * indices,
                                    const size_t icount );

  void
  vertex_buffer_push_back_vertices ( vertex_buffer_t *self,
                                     const void * vertices,
                                     const size_t vcount );


  void
  vertex_buffer_insert_indices ( vertex_buffer_t *self,
                                 const size_t index,
                                 const vb_index_t *indices,
                                 const size_t icount );


  void
  vertex_buffer_insert_vertices ( vertex_buffer_t *self,
                                  const size_t index,
                                  const void *vertices,
                                  const size_t vcount );

  void
  vertex_buffer_erase_indices ( vertex_buffer_t *self,
                                const size_t first,
                                const size_t last );

  void
  vertex_buffer_erase_vertices ( vertex_buffer_t *self,
                                 const size_t first,
                                 const size_t last );


  size_t
  vertex_buffer_push_back( vertex_buffer_t * self,
                           const void * vertices, const size_t vcount,  
                           const vb_index_t * indices, const size_t icount );


  size_t
  vertex_buffer_insert( vertex_buffer_t * self,
                        const size_t index,
                        const void * vertices, const size_t vcount,  
                        const vb_index_t * indices, const size_t icount );

  void
  vertex_buffer_erase( vertex_buffer_t * self,
                       const size_t index );

  void
  vertex_buffer_invalidate(vertex_buffer_t * self);

#endif 
