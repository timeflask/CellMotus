#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "audio.h"
#include "khash.h"
#include "asset.h"
#include <stdio.h>
#include "stdlib.h"

#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:NOAudio"
//------------------------------------------------------------------------- mci


void
sen_audio_init()
{
}

void
sen_audio_destroy()
{
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
}

void
sen_sound_unload(const char* path)
{
}

unsigned int
sen_sound_play(const char* path)
{
}

unsigned int
sen_sound_play_ex(const char* path,
                  int   bLoop,
                  float pitch,
                  float pan,
                  float gain)
{
  return 0;
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

