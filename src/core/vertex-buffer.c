#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "macro.h"
#include "utils.h"
#include "lmath.h"
#include "vertex-buffer.h"
#include "logger.h"
#include "utils.h"
/**
 * Buffer status
 */
#define CLEAN  (0)
#define DIRTY  (1)
#define FROZEN (2)


// ----------------------------------------------------------------------------
vertex_buffer_t *
vertex_buffer_new( const char *format )
{
    size_t i, index = 0, stride = 0;
    const char *start = 0, *end = 0;
    GLchar *pointer = 0;

    vertex_buffer_t *self = (vertex_buffer_t *) malloc (sizeof(vertex_buffer_t));
    if( !self )
    {
        return NULL;
    }

    self->format = sen_strdup( format );

    for( i=0; i<MAX_VERTEX_ATTRIBUTE; ++i )
    {
        self->attributes[i] = 0;
    }

    start = format;
    do
    {
        char *desc = 0;
        vertex_attribute_t *attribute;
        GLuint attribute_size = 0;
        end = (char *) (strchr(start+1, ','));

        if (end == NULL)
        {
            desc = sen_strdup( start );
        }
        else
        {
            desc = sen_strndup( start, end-start );
        }
        attribute = vertex_attribute_parse( desc );
        start = end+1;
        free(desc);
        attribute->pointer = pointer;

        switch( attribute->type )
        {
        case GL_BOOL:           attribute_size = sizeof(GLboolean); break;
        case GL_BYTE:           attribute_size = sizeof(GLbyte); break;
        case GL_UNSIGNED_BYTE:  attribute_size = sizeof(GLubyte); break;
        case GL_SHORT:          attribute_size = sizeof(GLshort); break;
        case GL_UNSIGNED_SHORT: attribute_size = sizeof(GLushort); break;
        case GL_INT:            attribute_size = sizeof(GLint); break;
        case GL_UNSIGNED_INT:   attribute_size = sizeof(GLuint); break;
        case GL_FLOAT:          attribute_size = sizeof(GLfloat); break;
        default:                attribute_size = 0;
        }
        stride  += attribute->size*attribute_size;
        pointer += attribute->size*attribute_size;
        self->attributes[index] = attribute;
        index++;
    } while ( end && (index < MAX_VERTEX_ATTRIBUTE) );

    for( i=0; i<index; ++i )
    {
        self->attributes[i]->stride = stride;
    }

    self->vertices = vector_new( stride );
    self->vertices_id  = 0;
    self->GPU_vsize = 0;

    self->indices = vector_new( sizeof(GLushort) );
    self->indices_id  = 0;
    self->GPU_isize = 0;

    self->items = vector_new( sizeof(ivec4) );
    self->state = DIRTY;
    self->mode = GL_TRIANGLES;
    return self;
}



// ----------------------------------------------------------------------------
void
vertex_buffer_delete( vertex_buffer_t *self )
{
    size_t i;

    sen_assert( self );


    for( i=0; i<MAX_VERTEX_ATTRIBUTE; ++i )
    {
        if( self->attributes[i] )
        {
            vertex_attribute_delete( self->attributes[i] );
        }
    }


    vector_delete( self->vertices );
    self->vertices = 0;
    if( self->vertices_id )
    {
        glDeleteBuffers( 1, &self->vertices_id );
    }
    self->vertices_id = 0;

    vector_delete( self->indices );
    self->indices = 0;
    if( self->indices_id )
    {
        glDeleteBuffers( 1, &self->indices_id );
    }
    self->indices_id = 0;

    vector_delete( self->items );

    if( self->format )
    {
        free( self->format );
    }
    self->format = 0;
    self->state = 0;
    free( self );
}


// ----------------------------------------------------------------------------
const char *
vertex_buffer_format( const vertex_buffer_t *self )
{
    sen_assert( self );

    return self->format;
}


// ----------------------------------------------------------------------------
size_t
vertex_buffer_size( const vertex_buffer_t *self )
{
    sen_assert( self );

    return vector_size( self->items );
}


// ----------------------------------------------------------------------------
void
vertex_buffer_print( vertex_buffer_t * self )
{
#ifdef SEN_DEBUG
    int i = 0;
    static char *gltypes[9] = {
        "GL_BOOL",
        "GL_BYTE",
        "GL_UNSIGNED_BYTE",
        "GL_SHORT",
        "GL_UNSIGNED_SHORT",
        "GL_INT",
        "GL_UNSIGNED_INT",
        "GL_FLOAT",
        "GL_VOID"
    };

    sen_assert(self);

    _logfe("%ld vertices, %ld indices\n",
             vector_size( self->vertices ), vector_size( self->indices ) );
    while( self->attributes[i] )
    {
        int j = 8;
        switch( self->attributes[i]->type )
        {
        case GL_BOOL:           j=0; break;
        case GL_BYTE:           j=1; break;
        case GL_UNSIGNED_BYTE:  j=2; break;
        case GL_SHORT:          j=3; break;
        case GL_UNSIGNED_SHORT: j=4; break;
        case GL_INT:            j=5; break;
        case GL_UNSIGNED_INT:   j=6; break;
        case GL_FLOAT:          j=7; break;
        default:                j=8; break;
        }
        _logfe( "%s : %dx%s (+%p)\n",
                self->attributes[i]->name, 
                self->attributes[i]->size, 
                gltypes[j],
                self->attributes[i]->pointer);

        i += 1;
    }
#else
    UNUSED(self);
#endif
}


// ----------------------------------------------------------------------------
void
vertex_buffer_upload ( vertex_buffer_t *self )
{
    size_t vsize, isize;


    if( self->state == FROZEN )
    {
        return;
    }

    if( !self->vertices_id )
    {
        glGenBuffers( 1, &self->vertices_id );
       // _logfi("Gen vertex buffer, vertices id = %u", self->vertices_id);
        gl_check_error();
    }
    if( !self->indices_id )
    {
        glGenBuffers( 1, &self->indices_id );
        //_logfi("Gen vertex buffer, indices id = %u", self->indices_id);
        gl_check_error();
    }
    gl_check_error();
    vsize = self->vertices->size*self->vertices->item_size;
    isize = self->indices->size*self->indices->item_size;

    //-----------------------------------------------------------------------------------------//
    glBindBuffer( GL_ARRAY_BUFFER, self->vertices_id );
/*
    glBufferData(GL_ARRAY_BUFFER, vsize, NULL, GL_DYNAMIC_DRAW);
    void *vbuf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(vbuf, self->vertices->items, vsize);
    glUnmapBuffer(GL_ARRAY_BUFFER);
*/

//    if( vsize != self->GPU_vsize )
    if( self->GPU_vsize < vsize)
    {
        glBufferData( GL_ARRAY_BUFFER, vsize, self->vertices->items, GL_DYNAMIC_DRAW );
        self->GPU_vsize = vsize;
        gl_check_error();
    }
    else
    {
        glBufferSubData( GL_ARRAY_BUFFER, 0, vsize, self->vertices->items );
        self->GPU_vsize = vsize;
        gl_check_error();
    }
    glBindBuffer( GL_ARRAY_BUFFER, 0 );


    //-----------------------------------------------------------------------------------------//
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->indices_id );
/*
    // Upload indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, NULL, GL_DYNAMIC_DRAW);
    void *ibuf = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(ibuf, self->indices->items, isize);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    */
//    if( isize != self->GPU_isize )
    if( self->GPU_isize < isize)
    {
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, isize, self->indices->items, GL_DYNAMIC_DRAW );
        //glBufferData( GL_ELEMENT_ARRAY_BUFFER, isize, self->indices->items, GL_STREAM_DRAW );
        self->GPU_isize = isize;
        gl_check_error();
        //_logfv(">buffer indexes data %d", self->GPU_isize);
    }
    else
    {
        glBufferSubData( GL_ELEMENT_ARRAY_BUFFER,
                         0, isize, self->indices->items );
        //_logfv(">buffer indexes sub data %d", isize);
        gl_check_error();
    }

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    gl_check_error();
}



// ----------------------------------------------------------------------------
void
vertex_buffer_clear( vertex_buffer_t *self )
{
    sen_assert( self );

    self->state = FROZEN;
    vector_clear( self->indices );
    vector_clear( self->vertices );
    vector_clear( self->items );
    self->state = DIRTY;
}



// ----------------------------------------------------------------------------
void
vertex_buffer_render_setup ( vertex_buffer_t *self, GLenum mode )
{
    size_t i;

    if( self->state != CLEAN )
    {
        vertex_buffer_upload( self );
        self->state = CLEAN;
    }

    glBindBuffer( GL_ARRAY_BUFFER, self->vertices_id );

    for( i=0; i<MAX_VERTEX_ATTRIBUTE; ++i )
    {
        vertex_attribute_t *attribute = self->attributes[i];
        if ( attribute == 0 )
        {
            continue;
        }
        else
        {
            vertex_attribute_enable( attribute );
        }
    }

    if( self->indices->size )
    {
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->indices_id );
    }
    self->mode = mode;
    gl_check_error();
}

// ----------------------------------------------------------------------------
void
vertex_buffer_render_finish ( vertex_buffer_t *self )
{
  UNUSED(self);
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  gl_check_error();
}


// ----------------------------------------------------------------------------
void
vertex_buffer_render_item ( vertex_buffer_t *self,
                            size_t index )
{ 
    ivec4 * item = (ivec4 *) vector_get( self->items, index );
    sen_assert( self );
    sen_assert( index < vector_size( self->items ) );

 
    if( self->indices->size )
    {
        size_t start = item->istart;
        size_t count = item->icount;
        glDrawElements( self->mode, count, GL_UNSIGNED_SHORT, (void *)(start*sizeof(GLushort)) );
    }
    else if( self->vertices->size )
    {
        size_t start = item->vstart;
        size_t count = item->vcount;
        glDrawArrays( self->mode, start*self->vertices->item_size, count);
    }
}


// ----------------------------------------------------------------------------
void
vertex_buffer_render ( vertex_buffer_t *self, GLenum mode )
{
    size_t vcount = self->vertices->size;
    size_t icount = self->indices->size;

    vertex_buffer_render_setup( self, mode );
    if( icount )
    {

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->indices_id );
        glDrawElements( mode, icount, GL_UNSIGNED_SHORT, 0 );
    }
    else
    {
        glDrawArrays( mode, 0, vcount );
    }
    vertex_buffer_render_finish( self );
    gl_check_error();
}
    


// ----------------------------------------------------------------------------
void
vertex_buffer_push_back_indices ( vertex_buffer_t * self,
                                  const GLushort * indices,
                                  const size_t icount )
{
    sen_assert( self );

    self->state |= DIRTY;
    vector_push_back_data( self->indices, indices, icount );
}



// ----------------------------------------------------------------------------
void
vertex_buffer_push_back_vertices ( vertex_buffer_t * self,
                                   const void * vertices,
                                   const size_t vcount )
{
    sen_assert( self );

    self->state |= DIRTY;
    vector_push_back_data( self->vertices, vertices, vcount );
}



// ----------------------------------------------------------------------------
void
vertex_buffer_insert_indices ( vertex_buffer_t *self,
                               const size_t index,
                               const GLushort *indices,
                               const size_t count )
{
    sen_assert( self );
    sen_assert( self->indices );
    sen_assert( index < self->indices->size+1 );

    self->state |= DIRTY;
    vector_insert_data( self->indices, index, indices, count );
}



// ----------------------------------------------------------------------------
void
vertex_buffer_insert_vertices( vertex_buffer_t *self,
                               const size_t index,
                               const void *vertices,
                               const size_t vcount )
{
    size_t i;
    sen_assert( self );
    sen_assert( self->vertices );
    sen_assert( index < self->vertices->size+1 );

    self->state |= DIRTY;

     for( i=0; i<self->indices->size; ++i )
    {
        if( *(GLushort *)(vector_get( self->indices, i )) > index )
        {
            *(GLushort *)(vector_get( self->indices, i )) += index;
        }
    }

    vector_insert_data( self->vertices, index, vertices, vcount );
}



// ----------------------------------------------------------------------------
void
vertex_buffer_erase_indices( vertex_buffer_t *self,
                             const size_t first,
                             const size_t last )
{
    sen_assert( self );
    sen_assert( self->indices );
    sen_assert( first < self->indices->size );
    sen_assert( (last) <= self->indices->size );

    self->state |= DIRTY;
    vector_erase_range( self->indices, first, last );
}



// ----------------------------------------------------------------------------
void
vertex_buffer_erase_vertices( vertex_buffer_t *self,
                              const size_t first,
                              const size_t last )
{
    size_t i;
    sen_assert( self );
    sen_assert( self->vertices );
    sen_assert( first < self->vertices->size );
    sen_assert( (first+last) <= self->vertices->size );
    sen_assert( last > first );

    self->state |= DIRTY;
    for( i=0; i<self->indices->size; ++i )
    {
        if( *(GLushort *)(vector_get( self->indices, i )) > first )
        {
            *(GLushort *)(vector_get( self->indices, i )) -= (last-first);
        }
    }
    vector_erase_range( self->vertices, first, last );    
}



// ----------------------------------------------------------------------------
size_t
vertex_buffer_push_back( vertex_buffer_t * self,
                         const void * vertices, const size_t vcount,  
                         const GLushort * indices, const size_t icount )
{
    return vertex_buffer_insert( self, vector_size( self->items ),
                                 vertices, vcount, indices, icount );
}

// ----------------------------------------------------------------------------
size_t
vertex_buffer_insert( vertex_buffer_t * self, const size_t index,
                      const void * vertices, const size_t vcount,  
                      const GLushort * indices, const size_t icount )
{
    size_t vstart, istart, i;
    ivec4 item;
    sen_assert( self );
    sen_assert( vertices );
    sen_assert( indices );

    self->state = FROZEN;

    // Push back vertices
    vstart = vector_size( self->vertices );
    vertex_buffer_push_back_vertices( self, vertices, vcount );

    // Push back indices
    istart = vector_size( self->indices );
    vertex_buffer_push_back_indices( self, indices, icount );

    // Update indices within the vertex buffer
    for( i=0; i<icount; ++i )
    {
        *(GLushort *)(vector_get( self->indices, istart+i )) += vstart;
    }
    
    // Insert item
    item.x = vstart;
    item.y = vcount;
    item.z = istart;
    item.w = icount;
    vector_insert( self->items, index, &item );

    self->state = DIRTY;
    return index;
}

// ----------------------------------------------------------------------------
void
vertex_buffer_erase( vertex_buffer_t * self,
                     const size_t index )
{
    ivec4 * item;
    size_t vstart, vcount, istart, icount, i;
    
    sen_assert( self );
    sen_assert( index < vector_size( self->items ) );

    item = (ivec4 *) vector_get( self->items, index );
    vstart = item->vstart;
    vcount = item->vcount;
    istart = item->istart;
    icount = item->icount;

    // Update items
    for( i=0; i<vector_size(self->items); ++i )
    {
        ivec4 * item = (ivec4 *) vector_get( self->items, i );
        if( (size_t)item->vstart > vstart)
        {
            item->vstart -= vcount;
            item->istart -= icount;
        }
    }

    self->state = FROZEN;
    vertex_buffer_erase_indices( self, istart, istart+icount );
    vertex_buffer_erase_vertices( self, vstart, vstart+vcount );
    vector_erase( self->items, index );
    self->state = DIRTY;
}

void
vertex_buffer_invalidate(vertex_buffer_t * self)
{
  self->vertices_id = 0;
  self->indices_id = 0;
  self->GPU_vsize = 0;
  self->GPU_isize = 0;
  self->state = DIRTY;
}
