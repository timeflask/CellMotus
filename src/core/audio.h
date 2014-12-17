#ifndef __audio_H_
#define __audio_H_
#include "config.h"

void
sen_audio_init();

void
sen_audio_destroy();

void
sen_audio_update();
//-----------------------------------------------------------------------------------------
void
sen_music_preload(const char* path);

void
sen_music_play(const char* path, int loop);

void
sen_music_stop(int release);

void
sen_music_pause();

void
sen_music_resume();

void
sen_music_rewind();

int
sen_music_is_playing();

float
sen_music_get_vol();

void
sen_music_set_vol(float volume);

//-----------------------------------------------------------------------------------------
void
sen_sound_preload(const char* path);

void
sen_sound_unload(const char* path);

unsigned int
sen_sound_play(const char* path);

unsigned int
sen_sound_play_ex(const char* path,
                  int   bLoop,
                  float pitch,
                  float pan,
                  float gain);

void
sen_sound_pause(unsigned int id);

void
sen_sound_pause_all();

void
sen_sound_resume(unsigned int id);

void
sen_sound_resume_all();

void
sen_sound_stop(unsigned int id);

void
sen_sound_stop_all();

float
sen_sound_get_vol();

void
sen_sound_set_vol(float volume);

#endif
