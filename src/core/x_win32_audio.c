#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "audio.h"
#include "khash.h"
#include "asset.h"
#include <Windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "stdlib.h"
#include "macro.h"
#include "logger.h"
#include "utils.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:MCIPlayer"
#pragma comment( lib, "WinMM.lib" )
//------------------------------------------------------------------------- mci
static HINSTANCE hInstance;
static MCIERROR  mciErr;
static const char WINDOW_CLASS[] = "senMCIPlayerWndClass";
static char ErrorMsg[128] = {0};
#define MCI_CHECK \
  do{ if (mciErr) {\
     mciGetErrorString (mciErr, ErrorMsg, 128);\
    _logfe("mciError[%lu] : %s", mciErr, ErrorMsg);\
    sen_assert(0);\
    return;\
  }}while(0)\

#define MCI_CHECK_RET(x) \
  do{ if (mciErr) {\
     mciGetErrorString (mciErr, ErrorMsg, 128);\
    _logfe("mciError[%lu] : %s", mciErr, ErrorMsg);\
    sen_assert(0);\
    return x;\
  }}while(0)\

typedef struct mci_player_t
{
  HWND        wnd;
  MCIDEVICEID dev;
  UINT        id;
  UINT        times;
  int         playing;
}mci_player_t;

LRESULT WINAPI play_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static mci_player_t* player_new()
{
  struct_malloc(mci_player_t,self);
  _logfi("new MCI player");
  memset(self,0,sizeof(mci_player_t));

  if (! hInstance)
  {
    WNDCLASS  wc;
    hInstance = GetModuleHandle( NULL );
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.lpfnWndProc    = play_proc;                    
    wc.hInstance      = hInstance;                    
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );  
    wc.lpszClassName  = WINDOW_CLASS;
    if (! RegisterClass(&wc) && 1410 != GetLastError())
      return self;
  }

  self->wnd = CreateWindowEx( WS_EX_APPWINDOW&~WS_VISIBLE, WINDOW_CLASS, NULL,                                        
                              WS_POPUPWINDOW, 0, 0, 0, 0, NULL, NULL,          
                              hInstance,  NULL );
  if (self->wnd)
    SetWindowLongPtr(self->wnd, GWLP_USERDATA, (LONG_PTR)self);

  sen_assert(self->wnd);
  return self;
}

void player_cmd(mci_player_t* self, int nCmd, DWORD_PTR p1, DWORD_PTR p2)
{
  sen_assert(self);
  if (self->dev) {
    mciErr = mciSendCommand(self->dev, nCmd, p1, p2);
    MCI_CHECK;
  }
}

static void player_stop(mci_player_t* self)
{
  sen_assert(self);
  if (self->dev)
  {
    player_cmd(self, MCI_STOP, 0, 0);
    self->playing = 0; self->times = 0;
  }
}

static void player_close(mci_player_t* self)
{
  sen_assert(self);
  if (self->playing) player_stop(self);
  if (self->dev)
    player_cmd(self, MCI_CLOSE, 0, 0);
  self->dev = 0;
  self->playing = 0;
}

static void player_delete(mci_player_t* self)
{
  sen_assert(self);
  _logfi("del MCI player");
  player_close(self);
  if (self->wnd)
    DestroyWindow(self->wnd);
  free(self);
}

static void player_pause(mci_player_t* self)
{
  sen_assert(self);
  player_cmd(self, MCI_PAUSE, 0, 0);
}

static void player_play(mci_player_t* self, UINT Times)
{
  sen_assert(self);
  if (self->dev) {
    MCI_PLAY_PARMS mciPlay = {0};
    mciPlay.dwCallback = (DWORD_PTR)(self->wnd);
    mciErr = mciSendCommand(self->dev, MCI_PLAY, MCI_FROM|MCI_NOTIFY,
                           (DWORD_PTR)(&mciPlay));
    if (!mciErr)
    {
      self->playing = 1;
      self->times = Times;
    }
    MCI_CHECK;
  }
}

#define BUFSIZE 4096

static void player_open(mci_player_t* self, const char* path, UINT id)
{
  MCI_OPEN_PARMS mciOpen = {0};
  //char* tmp;
  DWORD  retval=0;
  TCHAR  buffer[BUFSIZE]=TEXT(""); 
  TCHAR** lppPart={NULL};

  const char* aroot = sen_assets_get_root();
  char* rpath = (char*) malloc(strlen(aroot) + strlen("assets/") + strlen(path) + 1 );
  char* p = rpath;
  strcpy(p,aroot); p+= strlen(aroot);
  strcpy(p,"assets/"); p+= strlen("assets/");
  strcpy(p,path);

  sen_assert(self);
  if (!path  || !path[0] || !self->wnd) return;
  player_close(self);

  mciOpen.lpstrDeviceType = (LPCTSTR)MCI_ALL_DEVICE_ID;

  retval = GetFullPathName(rpath,
                           BUFSIZE,
                           buffer,
                           lppPart);
  if (retval == 0) {
    _logfe("Failed to GetFullPathName [%s]", path);
    return;
  }
  else
  {
    _logfi("Loading : %s", buffer);
  }
  free(rpath);
  rpath = sen_strdup(buffer);
  mciOpen.lpstrElementName = rpath;
  mciErr = mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT, 
                            (DWORD_PTR)(&mciOpen));
  free(rpath);
  MCI_CHECK;
  
  self->dev = mciOpen.wDeviceID;
  self->id = id;
  self->playing = 0;
}

static void player_rewind(mci_player_t* self)
{
  sen_assert(self);
  if (self->dev) {
    MCI_PLAY_PARMS mciPlay = {0};
    player_cmd(self, MCI_SEEK, MCI_SEEK_TO_START, 0);
    mciPlay.dwCallback = (DWORD_PTR) (self->wnd);
    self->playing =
    mciErr = mciSendCommand(self->dev, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)(&mciPlay) ? 0 : 1);
    MCI_CHECK;
 }
}

static void player_resume(mci_player_t* self)
{
  sen_assert(self);
  player_cmd(self, MCI_RESUME, 0, 0);
}

static LRESULT WINAPI play_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  mci_player_t* p = NULL;

  if (MM_MCINOTIFY == Msg && 
      MCI_NOTIFY_SUCCESSFUL == wParam &&
      (p = (mci_player_t*)GetWindowLongPtr(hWnd, GWLP_USERDATA)))
  {
    if (p->times)
      --p->times;
    if (p->times)
    {
      MCI_PLAY_PARMS mciPlay = {0};
      
      mciErr = mciSendCommand(lParam, MCI_SEEK, MCI_SEEK_TO_START, 0);
      MCI_CHECK_RET(0);
      mciPlay.dwCallback = (DWORD_PTR)(hWnd);
      mciErr = mciSendCommand(lParam, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)(&mciPlay));
      MCI_CHECK_RET(0);
    }
    else
      p->playing = 0;
    return 0;
  }
  return DefWindowProc(hWnd, Msg, wParam, lParam);
}

static unsigned int hash_string(const char *s)
{
	unsigned int h = (unsigned int)*s;
	if (h) for (++s ; *s; ++s) h = (h << 5) - h + (unsigned int)*s;
	return h;
}

//-----------------------------------------------------------------------------
static khash_t(hmip)* mapSound = NULL;

static mci_player_t* sound_get(unsigned int key)
{
  khiter_t j = kh_get(hmip, mapSound, key);
  return j != kh_end(mapSound) ?
    (mci_player_t*) kh_val(mapSound, j) :
    NULL;
}

static mci_player_t* sound_set(const char* path)
{
  mci_player_t* s = NULL;
  unsigned int key = hash_string(path);
  khiter_t j = kh_get(hmip, mapSound, key);
  if (j == kh_end(mapSound))
  {
    s = player_new();
    kh_insert(hmip, mapSound, key, (void*) s);
    return s;
  }
  return NULL;
}

static void sound_rm(unsigned int key)
{
  mci_player_t* s = NULL;
  khiter_t j = kh_get(hmip, mapSound, key);
  if (j != kh_end(mapSound))
  {
    s = (mci_player_t*) kh_val(mapSound, j);
    player_delete(s);
    kh_del(hmip, mapSound, j);
  }
}

void
sen_audio_init()
{
  mapSound = kh_init(hmip);
}

void
sen_audio_destroy()
{
  void* ps;
  kh_foreach_value(mapSound, ps, player_delete(ps));
  kh_destroy(hmip, mapSound);
}


void
sen_music_preload(const char* path)
{
UNUSED(path);
}

void
sen_music_play(const char* path, int loop)
{
  UNUSED(path);UNUSED(loop);
}

void
sen_music_stop(int release)
{
  UNUSED(release);

}

void
sen_music_pause()
{

}

void
sen_music_resume()
{

}

void
sen_music_rewind()
{

}

int
sen_music_is_playing()
{
  return 0;
}

float
sen_music_get_vol()
{
  return 0.0f;
}

void
sen_music_set_vol(float volume)
{
  UNUSED(volume);

}

//-------------------------------------------------------------------------------------
void
sen_sound_preload(const char* path)
{
  unsigned int key = 0;
  mci_player_t* p = NULL;
  if (!path || !path[0]) return;
  key = hash_string(path);
  if ( sound_get(key) ) return;
  p = sound_set( path );
  player_open(p, path, key);
  if (key != p->id ) {
    sound_rm(key);
  }
}

void
sen_sound_unload(const char* path)
{
  sound_rm( hash_string(path) );
}

unsigned int
sen_sound_play(const char* path)
{
  return sen_sound_play_ex(path, 0, 1.0f, 0.0f, 1.0f);
}

unsigned int
sen_sound_play_ex(const char* path,
                  int   bLoop,
                  float pitch,
                  float pan,
                  float gain)
{
  mci_player_t* p;
  unsigned int key = (unsigned int)hash_string(path);
  sen_sound_preload(path);
  p = sound_get(key);
  if (p) 
  {
    player_play(p, bLoop?-1:1);
  }
  return key;
}

void
sen_sound_pause(unsigned int id)
{
  mci_player_t* p = sound_get(id);
  if (p)
    player_pause(p);
}

void
sen_sound_pause_all()
{
  void* ps;
  kh_foreach_value(mapSound, ps, player_pause(ps) );

}

void
sen_sound_resume(unsigned int id)
{
  mci_player_t* p = sound_get(id);
  if (p)
    player_resume(p);
}

void
sen_sound_resume_all()
{
  void* ps;
  kh_foreach_value(mapSound, ps, player_resume(ps) );
}

void
sen_sound_stop(unsigned int id)
{
  mci_player_t* p = sound_get(id);
  if (p)
    player_stop(p);
}

void
sen_sound_stop_all()
{
  void* ps;
  kh_foreach_value(mapSound, ps, player_stop(ps) );
}

float
sen_sound_get_vol()
{
  return 0.0f;
}

void
sen_sound_set_vol(float volume)
{
}

void
sen_audio_update()
{
}

