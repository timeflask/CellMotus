#ifndef __vector_h__
#define __vector_h__

#include <stdlib.h>
#include <stddef.h>

typedef struct
 {
     void * items;
     size_t capacity;
     size_t size;
     size_t item_size;
} vector_t;


  vector_t *
  vector_new( size_t item_size );

  void
  vector_delete( vector_t *self );


  const void *
  vector_get( const vector_t *self,
              size_t index );


  const void *
  vector_front( const vector_t *self );


  const void *
  vector_back( const vector_t *self );


  int
  vector_contains( const vector_t *self,
                   const void *item,
                   int (*cmp)(const void *, const void *) );

  int
  vector_contains_b( const vector_t *self,
                   const void *item,
                   int (*cmp)(const void *, const void *) );

  int
  vector_empty( const vector_t *self );


  size_t
  vector_size( const vector_t *self );


  void
  vector_reserve( vector_t *self,
                  const size_t size );


  size_t
  vector_capacity( const vector_t *self );


  void
  vector_shrink( vector_t *self );


  void
  vector_clear( vector_t *self );


  void
  vector_set( vector_t *self,
              const size_t index,
              const void *item );

  void
  vector_erase( vector_t *self,
                const size_t index );


  void
  vector_erase_range( vector_t *self,
                      const size_t first,
                      const size_t last );


  void
  vector_push_back( vector_t *self,
                    const void *item );

  void
  vector_pop_back( vector_t *self );


  void
  vector_resize( vector_t *self,
                 const size_t size );


  void
  vector_insert( vector_t *self,
                 const size_t index,
                 const void *item );


  void
  vector_insert_data( vector_t *self,
                      const size_t index,
                      const void * data,
                      const size_t count );


  void
  vector_push_back_data( vector_t *self,
                         const void * data,
                         const size_t count );


  void
  vector_sort( vector_t *self,
               int (*cmp)(const void *, const void *) );

  typedef int (*vector_for_each_fp)(const void *, void*);

  void
  vector_for_each( vector_t          *self,
                   vector_for_each_fp func,
                   void*              user_data);



#endif /* __VECTOR_H__ */
