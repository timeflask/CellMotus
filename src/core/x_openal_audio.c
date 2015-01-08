#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "audio.h"
#include "khash.h"
#include "asset.h"
#include "vector.h"
#include <stdio.h>
#include "stdlib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:OpenALAudio"
//------------------------------------------------------------------------- mci

typedef struct sound_t 
{
  ALuint          bufferID;		
  //ALuint          sourceID;
  ALenum          error;				
  ALvoid         *data;	
  vector_t       *sources;
  char           *full_path;
  char           *path;
} sound_t;
static sound_t* sound_new( const char* path );
static void sound_delete( sound_t* self);
static unsigned int sound_play( sound_t* self);

static khash_t(hmsp)* mapSound = NULL;
static void soundMap_clear();
static sound_t* soundMap_get(const char* path);
static void soundMap_set(const char* path);

static void openAL_shutdown();
static void openAL_init();

void
sen_audio_init()
{
  mapSound = kh_init(hmsp);
  openAL_init();  
  
  //sound_t *s = sound_new("assets/audio/matrix0.wav");  
  //sound_play(s);
  
}

void
sen_audio_destroy()
{
  soundMap_clear();
  kh_destroy(hmsp, mapSound);
  openAL_shutdown();  
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
  if (!path || !path[0]) return;
  if ( soundMap_get(path) ) return;
  
  soundMap_set( path );
}

void
sen_sound_unload(const char* path)
{
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
  sound_t* p;
  sen_sound_preload(path);
  p = soundMap_get(path);
  return p ? sound_play(p) : 0;
}

void
sen_sound_pause(unsigned int id)
{
}

void
sen_sound_pause_all()
{
}

void
sen_sound_resume(unsigned int id)
{
}

void
sen_sound_resume_all()
{
}

void
sen_sound_stop(unsigned int id)
{
}

void
sen_sound_stop_all()
{
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


static void openAL_shutdown()
{
  ALCcontext	*context = alcGetCurrentContext();
  ALCdevice	*device = alcGetContextsDevice(context);
  alcDestroyContext(context);
  alcCloseDevice(device);
}

static void openAL_init()
{
  ALCcontext *context = NULL;
  ALCdevice *device = NULL;

  device = alcOpenDevice(NULL); 
  if(!device) {
    _logfe("alcOpenDevice(NULL) Failed");
    return;
  }

  context = alcCreateContext(device, NULL);
  if(!context) {
    _logfe("alcCreateContext(device, NULL) Failed");
    return;
  }

  alcMakeContextCurrent(context);
  
}

static sound_t* sound_new( const char* path )
{
  ALenum  format;
  ALsizei size;
  ALsizei freq;
  ALboolean loop = AL_FALSE;
  ALuint sourceID;
   
  struct_malloc(sound_t, self);
  sen_assert(path);
  self->full_path = sen_assets_get_full_path(path);
  self->path = sen_strdup(path);
  self->sources = vector_new(sizeof(ALuint));
  
  _logfi("New sound : %s", self->full_path);
#undef check_exit
#define check_exit(code) do{ code; if ((self->error = alGetError()) != AL_NO_ERROR) { _logfe("OpenAL Error[0x%X]: %s", self->error, #code); return self; }	} while(0)
  
  self->error = AL_NO_ERROR; alGetError(); 
  

  check_exit( alutLoadWAVFile((ALbyte*)self->full_path, &format, &self->data, &size, &freq, &loop) );
  //self->data = alutLoadMemoryFromFile((ALbyte*)self->path,&format,&size, &freq);
  check_exit( alGenBuffers(1, &(self->bufferID) ) );
  check_exit( alBufferData(self->bufferID, format, self->data, size, freq) );
  //self->bufferID = alutCreateBufferFromFile (self->path);
//  check_exit( alGenSources(1, & (self->sourceID) ) );
  //check_exit( alSourcei(self->sourceID, AL_BUFFER, self->bufferID) );
  check_exit( alGenSources(1, & sourceID ) );
  check_exit( alSourcei(sourceID, AL_BUFFER, self->bufferID) );
  vector_push_back(self->sources, &sourceID);
  return self;
}

static void sound_delete( sound_t* self)
{
  int i;
  if (self->path) free(self->path);
  if (self->full_path) free(self->full_path);
  if (self->error != AL_NO_ERROR) {
    for (i = 0; i < self->sources->size; ++i) {
      const ALuint* sourceid = vector_get(self->sources, i);
      alDeleteSources(1, sourceid );
    }
//    alDeleteSources(1, &(self->sourceID) );
  }
  vector_delete(self->sources);
  if (self->data) {
    alDeleteBuffers(1, &(self->bufferID) );
    free(self->data);	
  }
  free(self);  
}

static int sound_playing(ALuint sourceID)
{
  ALenum state;	
  alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
  return (state == AL_PLAYING);
}

static unsigned int sound_play( sound_t* self)
{
  int i;ALuint newID;
  for (i = 0; i < self->sources->size; ++i) {
    const ALuint* sourceID = vector_get(self->sources, i);
    if (!sound_playing(*sourceID)) {
      alSourcePlay(*sourceID);
      return self->bufferID;
    }
  }
  if (i < 16) {
    alGenSources(1, & newID ) ;
    alSourcei(newID, AL_BUFFER, self->bufferID) ;
    vector_push_back(self->sources, &newID);
    alSourcePlay(newID);
    return self->bufferID;
    
  }
  return 0;
}

static void soundMap_clear()
{
  void* ps;
  kh_foreach_value(mapSound, ps, 
    sound_t* s = (sound_t*)ps; 
    sound_delete(s);
  );
  kh_clear(hmsp, mapSound);  
}

static sound_t* soundMap_get(const char* path)
{
  
  khiter_t j = kh_get(hmsp, mapSound, path);
  return j != kh_end(mapSound) ?
    (sound_t*) kh_val(mapSound, j) :
    NULL;
}

static void soundMap_set(const char* path)
{
  
  sound_t* s = NULL;
  khiter_t j = kh_get(hmsp, mapSound, path);
  if (j == kh_end(mapSound))
  {
    s = sound_new(path);
    kh_insert(hmsp, mapSound, s->path, (void*) s);
  }  
}
