//
#include "sen.h"
#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define _MAX_I (16)
#define _MAX_J (16)

#define _DIFFICULTY_MAX (100)


#define CELL_EMPTY  (0)
#define CELL_ITEM   (1 << 0)
#define CELL_ARROW  (1 << 1)
#define CELL_PIN    (1 << 3)

typedef struct cell_t {
  int type;
  int direction;
  int item_color;
  int pin_color;
} cell_t, board_t[_MAX_I][_MAX_J];

typedef struct map_t {
  int difficulty;
  int colors_num;
  int arrows_num;

  board_t board;


} map_t;


static void cell_reset(cell_t* c)
{
  memset(c,0,sizeof(cell_t));
}

static void map_init_colors_num(map_t* self)
{
  if (self->difficulty > 80)
    self->colors_num = rand() % 2 + 3;
  else if (self->difficulty > 30)
    self->colors_num = rand() % 3 + 2;
  else 
    self->colors_num = rand() % 2 + 2;
}

static void map_init_board(map_t* self)
{
  int i,j;
  for (i = 0; i<_MAX_I; ++i)
    for(j = 0; j<_MAX_J; ++j) 
      cell_reset( & (self->board[i][j]) );
}

static void map_gen_board(map_t* self)
{
  int i, j, l, r, s, n, m;
  cell_t* c;
  map_init_board(self);
  
  i = (_MAX_I/2) + 2 - (rand() % 5);
  j = (_MAX_J/2) + 2 - (rand() % 5);
  c = & ( self->board[i][j] );
  c->type = CELL_ITEM | CELL_PIN;
  c->item_color = self->colors_num;
  c->pin_color = self->colors_num;

}

static void  map_init_arrows_num(map_t* self)
{
  if (self->difficulty > 80)
    self->arrows_num = rand() % 5 + 2;
  else if (self->difficulty > 60)
    self->arrows_num = rand() % 4 + 2;
  else if (self->difficulty > 40)
    self->arrows_num = rand() % 4 + 1;
  else if (self->difficulty > 20)
    self->arrows_num = rand() % 3 + 1;
  else 
    self->arrows_num = rand() % 3 ;
}

static map_t* map_new(int difficulty)
{

  struct_malloc(map_t, self);

  self->difficulty = difficulty > _DIFFICULTY_MAX ? _DIFFICULTY_MAX : difficulty;
  if (self->difficulty < 1) self->difficulty = 1;

  map_init_colors_num(self);
  map_init_arrows_num(self);
  map_init_board(self);

  return self;
}

static void map_delete(map_t* self)
{
  free(self);
}

char* cm_ai_genLevel()
{
  time_t t;
  map_t* map = NULL;
  int i,j,n,m;
  srand((unsigned) time(&t));

  for( i=0; i< 4; ++i){
    map = map_new(rand()%100+1);
    printf("-------------------------------\n");
    printf("difficuly: %d\n", map->difficulty);
    printf("colors num: %d\n", map->colors_num);
    printf("arrows num: %d\n", map->arrows_num);
    map_gen_board(map);
    for (n = 0; n<_MAX_I; ++n) {
      if (n%2) printf("   ");
      for(m = 0; m<_MAX_J; ++m) {
        cell_t* c = & (map->board[n][m]);
        printf("%1d:%1d:%1d:%1d ", c->type, c->direction,  c->item_color, c->pin_color);
      }
      printf("\n");
    }
    map_delete(map);
  }

  return NULL;//sen_strdup("hello");
}