#include "input.h"
#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "signals.h"
#include "khash.h"
#include "camera.h"
#include "view.h"
#include "hrtimer.h"
#include <math.h>

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:Input"

//----------------------------------------------------------------------------
touch_t*
sen_touch_new()
{
  struct_malloc(touch_t, self);
  self->id = 0;
  self->has_start = 0;
  self->point.x = 0;
  self->point.y = 0;
  self->point.z = 0;
  self->point.w = 0;
  return self;
}

void
sen_touch_delete(touch_t* self)
{
  free(self);
}

void
sen_touch_reset(touch_t* self, int id, float x, float y, int reset_start)
{
  double now = sen_timer_now();

  if (reset_start) self->has_start = 0;
  self-> id = id;
  self->prev = self->point;
  self->point.x = x;
  self->point.y = y;
  if (! self->has_start )
  {
    self->start =  self->point;

    self->start.z = now;
    self->start.w = 0;

    self->point.z = 0;
    self->point.w = 0;

    self->prev.z = 0;
    self->prev.w = 0;

    self->has_start = 1;
  }
  else
  {
    self->point.z = max(0, now - self->start.z );
    //self->point.z = sen_timer_now() - self->start.z;
    self->point.w = self->prev.w +
        sqrtf((x - self->prev.x)*(x - self->prev.x) + (y - self->prev.y)*(y - self->prev.y));
  }
}

void
sen_touch_to_world(touch_t* t, touch_t* res)
{
  camera_t* cam = sen_camera();
  const vec4* vp =
  sen_view_get_viewport();
  //vec4 v0 = {{t->point.x-vp->width/2,-t->point.y+vp->height/2,0, 1}};
  //vec4 v1 = {{t->start.x,t->start.y,0, 1}};
  //vec4 v2 = {{t->prev.x, t->prev.y,0, 1}};

  //_logfi(".%f .%f ", v0.x, v0.y);
  const mat4* mv = &(((node_t*)cam)->model);
  float sx =  mv->data[12];
  float sy =  mv->data[13];

  //v4_transform(cam->view.data, v0.data, v0.data);
  //v4_transform(cam->view.data, v1.data, v1.data);
  //v4_transform(cam->view.data, v2.data, v2.data);

  //_logfi(".%f .%f ", v0.x, v0.y);


  memcpy(res, t, sizeof(touch_t));

  res->point.x = t->point.x-vp->width/2 - sx;
  res->point.y = -t->point.y+vp->height/2 - sy;

  res->start.x = t->start.x-vp->width/2 - sx;
  res->start.y = -t->start.y+vp->height/2 - sy;

  res->prev.x = t->prev.x-vp->width/2 - sx;
  res->prev.y = -t->prev.y+vp->height/2 - sy;
  //_logfi(".%f .%f ", res->point.x, res->point.y);
}

//----------------------------------------------------------------------------

#define KEYCODE_BACK 0x04
#define KEYCODE_MENU 0x52
#define KEYCODE_DPAD_UP 0x13
#define KEYCODE_DPAD_DOWN 0x14
#define KEYCODE_DPAD_LEFT 0x15
#define KEYCODE_DPAD_RIGHT 0x16
#define KEYCODE_ENTER 0x42
#define KEYCODE_PLAY  0x7e
#define KEYCODE_DPAD_CENTER  0x17
static const void* signal_keyDown = NULL;
static int on_key_down(object_t* _null, void* data, object_t* _null2, const char* sig)
{
  UNUSED(_null); UNUSED(_null2);UNUSED(sig);

  sen_signal_emit(signal_keyDown, data);
  _logfi("KEY DOWN : %d", *(int*)data);
  return 0;
}


#define MAX_TOUCHES  15
static khash_t(hmip) *g_touches_indexer;
static touch_t* g_touches[MAX_TOUCHES];
static uint32_t g_usedBits = 0;

static int get_index() {
  int i;
  int temp = g_usedBits;

  for (i = 0; i < MAX_TOUCHES; i++) {
    if (! (temp & 0x00000001)) {
      g_usedBits |= (1 <<  i);
      return i;
    }
    temp >>= 1;
  }
  return -1;
}
static void release_index(int index)
{
  uint32_t temp = 1 << index;
  sen_assert( index >= 0 && index < MAX_TOUCHES );
  temp = ~temp;
  g_usedBits &= temp;
}

static const void* signal_touchesBegin = NULL;
static int on_touches_begin(object_t* _null, void* data, object_t* _null2, const char* sig)
{
  touch_data_t *td = (touch_data_t *)data;
  touch_t* t;
  int i;

  sen_assert(data);

  for ( i = 0; i < td->num; ++i )
  {
    int  id = td->ids[i];
    khiter_t k = kh_get(hmip, g_touches_indexer, id );
    if (k == kh_end(g_touches_indexer)) {
      int index = get_index();
      if (index == -1) continue;

      t = g_touches[index];
      sen_touch_reset(t, index, td->xs[i], td->ys[i], 1);

      kh_insert(hmip, g_touches_indexer, id, t);
      if (signal_touchesBegin)
        sen_signal_emit(signal_touchesBegin, t);

     // _logfi("TOUCHES BEGIN id=%d x=%.2f y=%.2f", td->ids[i], td->xs[i], td->ys[i]);
    }
  }
  UNUSED(_null); UNUSED(_null2);UNUSED(sig);
  return 0;
}

static const void* signal_touchesMove = NULL;
static int on_touches_move(object_t* _null, void* data, object_t* _null2, const char* sig)
{
//  UNUSED(_null); UNUSED(_null2);UNUSED(sig);
  touch_data_t *td = (touch_data_t *)data;
  int i; touch_t* t;

  sen_assert(data);
  for ( i = 0; i < td->num; ++i )
  {
    int  id = td->ids[i];
    khiter_t k = kh_get(hmip, g_touches_indexer, id );
    if (k == kh_end(g_touches_indexer))
      continue;

    t = kh_val(g_touches_indexer, k);

    sen_touch_reset(t, t->id, td->xs[i], td->ys[i], 0);

    if (signal_touchesMove)
        sen_signal_emit(signal_touchesMove, t);
 //   _logfi("TOUCHES MOVE id=%d x=%.2f y=%.2f", td->ids[i], td->xs[i], td->ys[i]);

  }
  UNUSED(_null); UNUSED(_null2);UNUSED(sig);
  return 0;
}

static int do_touches_end(touch_data_t *td, const void* signal)
{
  int i; touch_t* t;

  sen_assert(td);

  for ( i = 0; i < td->num; ++i )
  {
    int  id = td->ids[i];
    khiter_t k = kh_get(hmip, g_touches_indexer, id );
    if (k == kh_end(g_touches_indexer))
      continue;

    t = kh_val(g_touches_indexer, k);

    sen_touch_reset(t, t->id, td->xs[i], td->ys[i], 0);

    if (signal)
      sen_signal_emit(signal, t);

    release_index(t->id);
    kh_del(hmip, g_touches_indexer, k);
  //  _logfi("TOUCHES END CANCEL id=%d x=%.2f y=%.2f", td->ids[i], td->xs[i], td->ys[i]);
  }
  return 0;
}


static const void* signal_touchesEnd = NULL;
static const void* signal_touchesCancel = NULL;
static int on_touches_end(object_t* _null, void* data, object_t* _null2, const char* sig)
{
  UNUSED(_null); UNUSED(_null2);UNUSED(sig);
  sen_assert(data);
  do_touches_end((touch_data_t *)data, signal_touchesEnd);
  return 0;
}

static int on_touches_cancel(object_t* _null, void* data, object_t* _null2, const char* sig)
{
  UNUSED(_null); UNUSED(_null2);UNUSED(sig);
  sen_assert(data);
  do_touches_end((touch_data_t *)data, signal_touchesCancel);
  return 0;
}

void
sen_input_init()
{
  int i;

  sen_signal_connect_name("platform", "keyDown", &on_key_down, "input");
  sen_signal_connect_name("platform", "touchesBegin", &on_touches_begin, "input");
  sen_signal_connect_name("platform", "touchesEnd", &on_touches_end, "input");
  sen_signal_connect_name("platform", "touchesMove", &on_touches_move, "input");
  sen_signal_connect_name("platform", "touchesCancel", &on_touches_cancel, "input");

  signal_touchesBegin = sen_signal_get_name("touchesBegin", "input");
  signal_touchesMove = sen_signal_get_name("touchesMove", "input");
  signal_touchesEnd = sen_signal_get_name("touchesEnd", "input");
  signal_touchesCancel = sen_signal_get_name("touchesCancel", "input");
  signal_keyDown = sen_signal_get_name("keyDown", "input");


  g_touches_indexer = kh_init(hmip);
  kh_resize(hmip, g_touches_indexer, MAX_TOUCHES);

  for (i = 0; i < MAX_TOUCHES; ++i)
    g_touches[i] = sen_touch_new();


}

void
sen_input_destroy()
{
  int i;

  sen_signal_release_emitter_name("input");
  sen_signal_disconnect_name("input",NULL,NULL);


  for (i = 0; i < MAX_TOUCHES; ++i)
    sen_touch_delete(g_touches[i]);

  kh_destroy(hmip, g_touches_indexer);

}
