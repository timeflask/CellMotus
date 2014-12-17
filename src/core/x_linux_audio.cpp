extern "C" {
#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "audio.h"
#include "khash.h"
#include "asset.h"
}
#include "fmod.hpp"
#include "fmod_errors.h"
#include <stdio.h>
#include "stdlib.h"


static FMOD::System*       pSystem = NULL;
static FMOD::Sound*        pMusic = NULL;
static FMOD::Channel*      pBGMChannel = NULL;
static FMOD::ChannelGroup* pChannelGroup = NULL;
static khash_t(hmsp)*      mapSound = NULL;
static khash_t(hmip)*      mapChannel =  NULL;
static unsigned int        iSoundChannelCount = 0;

typedef struct sound_t
{
  FMOD::Sound* sound;
  char*        key;
} sound_t;

sound_t* sound_new(FMOD::Sound* sound, const char* path)
{
  struct_malloc(sound_t, self);
  self->key = sen_strdup(path);
  self->sound = sound;
  return self;
}

void sound_delete(void* _self)
{
  sen_assert(_self);
  sound_t* self = (sound_t*) _self;
  sen_assert(self->key);
  free(self->key);
  if (self->sound) self->sound->release();
  free(self);
}

sound_t* sound_set(FMOD::Sound* sound, const char* path)
{
  sound_t* s = NULL;
  khiter_t j = kh_get(hmsp, mapSound, path);
  if (j != kh_end(mapSound))
  {
    s = (sound_t*) kh_val(mapSound, j);
    if (s->sound) s->sound->release();
    s->sound = sound;
  }
  else {
    s = sound_new(sound,path);
    kh_insert(hmsp, mapSound, s->key, (void*) s);
  }
  return s;
}

sound_t* sound_get(const char* path)
{
  sound_t* s = NULL;
  khiter_t j = kh_get(hmsp, mapSound, path);
  if (j != kh_end(mapSound))
    s = (sound_t*) kh_val(mapSound, j);
  return s;
}

void sound_rm(const char* path)
{
  sound_t* s = NULL;
  khiter_t j = kh_get(hmsp, mapSound, path);
  if (j != kh_end(mapSound))
  {
    s = (sound_t*) kh_val(mapSound, j);
    sound_delete(s);
    kh_del(hmsp, mapSound, j);
  }
}

void channel_set(FMOD::Channel* chan, unsigned int key)
{
  khiter_t j = kh_get(hmip, mapChannel, key);
  if (j != kh_end(mapChannel))
    kh_val(mapChannel, j) = (void*) chan;
  else
    kh_insert(hmip, mapChannel, key, (void*) chan);
}

FMOD::Channel* channel_get(unsigned int key)
{
  khiter_t j = kh_get(hmip, mapChannel, key);
  return j != kh_end(mapChannel) ?
    (FMOD::Channel*) kh_val(mapChannel, j) :
    NULL;
}

void channel_rm(unsigned int key)
{
  khiter_t j = kh_get(hmip, mapChannel, key);
  if (j != kh_end(mapChannel) )
    kh_del(hmip, mapChannel, j);
}

void ERRCHECKWITHEXIT(FMOD_RESULT result) {
  if (result != FMOD_OK) {
    _logfe("FMOD error! (%d) %s", result, FMOD_ErrorString(result));
  }
}

bool ERRCHECK(FMOD_RESULT result) {
  if (result != FMOD_OK) {
    _logfe("FMOD error! (%d) %s", result, FMOD_ErrorString(result));
    return 1;
  }
  return 0;
}


void
sen_audio_init()
{
  mapSound   = kh_init(hmsp);
  mapChannel = kh_init(hmip);

  FMOD_RESULT result;
  FMOD::ChannelGroup *masterChannelGroup;

  result = FMOD::System_Create(&pSystem);
  ERRCHECKWITHEXIT(result);

  result = pSystem->setOutput(FMOD_OUTPUTTYPE_ALSA);
  ERRCHECKWITHEXIT(result);

  result = pSystem->init(32, FMOD_INIT_NORMAL, 0);
  ERRCHECKWITHEXIT(result);

  result = pSystem->createChannelGroup("SEN Channel Group", &pChannelGroup);
  ERRCHECKWITHEXIT(result);

  result = pSystem->getMasterChannelGroup(&masterChannelGroup);
  ERRCHECKWITHEXIT(result);

  result = masterChannelGroup->addGroup(pChannelGroup);
  ERRCHECKWITHEXIT(result);

}

void
sen_audio_destroy()
{
  void* ps;
  kh_foreach_value(mapSound, ps, sound_delete(ps));
  kh_destroy(hmsp, mapSound);

  kh_destroy(hmip, mapChannel);


  FMOD_RESULT result;

  if (pBGMChannel != NULL) {
    result = pBGMChannel->stop();
    ERRCHECKWITHEXIT(result);
  }

  if (pMusic != NULL) {
    result = pMusic->release();
    ERRCHECKWITHEXIT(result);
  }

  result = pChannelGroup->release();
  ERRCHECKWITHEXIT(result);

  result = pSystem->close();
  ERRCHECKWITHEXIT(result);
  result = pSystem->release();
  ERRCHECKWITHEXIT(result);

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
  FMOD::Sound* pLoadSound;

  const char* aroot = sen_assets_get_root();

  char* rpath = (char*) malloc(strlen(aroot) + strlen("assets/") + strlen(path) + 1 );
  char* p = rpath;
  strcpy(p,aroot); p+= strlen(aroot);
  strcpy(p,"assets/"); p+= strlen("assets/");
  strcpy(p,path);

  pSystem->update();
  FMOD_RESULT result = pSystem->createSound(rpath, FMOD_LOOP_NORMAL, 0,
      &pLoadSound);
  if (ERRCHECK(result))
    _logfe("Sound [%s] could not be loaded", rpath);
  else
    sound_set(pLoadSound, path);

  free(rpath);

}

void
sen_sound_unload(const char* path)
{
  pSystem->update();
  sound_rm(path);
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

  FMOD::Channel* pChannel;
  FMOD::Sound* pSound = NULL;

  do {
    pSystem->update();
    sound_t* s = sound_get(path);

    if (s == NULL)
    {
      sen_sound_preload(path);
      s = sound_get(path);
    }

    if (s==NULL)
      break;

    pSound = s->sound;

    if (pSound==NULL)
      break;

    FMOD_RESULT result = pSystem->playSound(FMOD_CHANNEL_FREE, pSound, true, &pChannel);

    if (ERRCHECK(result))
    {
      printf("Sound [%s] could not be played", path);
      break;
    }

    pChannel->setChannelGroup(pChannelGroup);
    pChannel->setPan(pan);
    float freq = 0;
    pChannel->getFrequency(&freq);
    pChannel->setFrequency(pitch * freq);
    pChannel->setVolume(gain);

    pChannel->setLoopCount((bLoop) ? -1 : 0);
    result = pChannel->setPaused(false);

    channel_set(pChannel, iSoundChannelCount);

    return iSoundChannelCount++;

  } while (0);

  return 0;
}

void
sen_sound_pause(unsigned int id)
{
  pSystem->update();
  FMOD::Channel* channel = channel_get(id);
  if (channel) {
    channel->setPaused(true);
  }
}

void
sen_sound_pause_all()
{
  pSystem->update();
  void* ps;
  kh_foreach_value(mapChannel, ps, ((FMOD::Channel*)ps)->setPaused(true) );

}

void
sen_sound_resume(unsigned int id)
{
  pSystem->update();
  FMOD::Channel* channel = channel_get(id);
  if (channel) {
    channel->setPaused(false);
  }
}

void
sen_sound_resume_all()
{
  pSystem->update();
  void* ps;
  kh_foreach_value(mapChannel, ps, ((FMOD::Channel*)ps)->setPaused(false) );
}

void
sen_sound_stop(unsigned int id)
{
  pSystem->update();
  FMOD::Channel* channel = channel_get(id);
  if (channel) {
    channel->stop();
    channel_rm(id);
  }
}

void
sen_sound_stop_all()
{
  pSystem->update();
  void* ps;
  kh_foreach_value(mapChannel, ps, ((FMOD::Channel*)ps)->stop() );
  kh_clear(hmip, mapChannel);

}

float
sen_sound_get_vol()
{
  float fVolumn;
  pSystem->update();
  FMOD_RESULT result = pChannelGroup->getVolume(&fVolumn);
  ERRCHECKWITHEXIT(result);
  return fVolumn;
}

void
sen_sound_set_vol(float volume)
{
  pSystem->update();
  FMOD_RESULT result = pChannelGroup->setVolume(volume);
  ERRCHECKWITHEXIT(result);
}

void
sen_audio_update()
{
  pSystem->update();
}

