#ifndef __audio_H_
#define __audio_H_
#include "config.h"

SEN_DECL void
sen_audio_init();

SEN_DECL void
sen_audio_destroy();

SEN_DECL void
sen_audio_update();
//-----------------------------------------------------------------------------------------
SEN_DECL void
sen_music_preload(const char* path);

SEN_DECL void
sen_music_play(const char* path, int loop);

SEN_DECL void
sen_music_stop(int release);

SEN_DECL void
sen_music_pause();

SEN_DECL void
sen_music_resume();

SEN_DECL void
sen_music_rewind();

SEN_DECL int
sen_music_is_playing();

SEN_DECL float
sen_music_get_vol();

SEN_DECL void
sen_music_set_vol(float volume);

//-----------------------------------------------------------------------------------------
SEN_DECL void
sen_sound_preload(const char* path);

SEN_DECL void
sen_sound_unload(const char* path);

SEN_DECL unsigned int
sen_sound_play(const char* path);

SEN_DECL unsigned int
sen_sound_play_ex(const char* path,
                  int   bLoop,
                  float pitch,
                  float pan,
                  float gain);

SEN_DECL void
sen_sound_pause(unsigned int id);

SEN_DECL void
sen_sound_pause_all();

SEN_DECL void
sen_sound_resume(unsigned int id);

SEN_DECL void
sen_sound_resume_all();

SEN_DECL void
sen_sound_stop(unsigned int id);

SEN_DECL void
sen_sound_stop_all();

SEN_DECL float
sen_sound_get_vol();

SEN_DECL void
sen_sound_set_vol(float volume);

#endif
