#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "audio.h"
#include "hrtimer.h"
#include "khash.h"
#include "asset.h"
#include "khash.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>

#import <Foundation/Foundation.h>
#import <OpenAL/al.h>
#import <OpenAL/alc.h>

#if SEN_PLATFORM==SEN_PLATFORM_IOS
#import <OpenAL/oalStaticBufferExtension.h>
#endif

#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/ExtendedAudioFile.h>


#undef SEN_LOG_TAG
#define SEN_LOG_TAG "SEN:AppleAudio"

//========================================================================= FORWARDS
//------------------------------------------------------------------------- openAL support oalTouch copypasta
static void* getOpenALAudioData( CFURLRef inFileURL, ALsizei *outDataSize,
                                 ALenum *outDataFormat, ALsizei* outSampleRate,
                                 ALdouble* duration);

//------------------------------------------------------------------------- openAL Sound

#if SEN_PLATFORM == SEN_PLATFORM_IOS
//static alBufferDataStaticProcPtr  alBufferDataStaticProc = NULL;
#endif

typedef struct sound_t
{
  ALuint          bufferID;
  //ALuint          sourceID;
  ALenum          error;
  ALvoid         *data;
  vector_t       *sources;
  char           *full_path;
  char           *path;
  double duration;
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


static int isDevicePlayingMusic = 0;
static int isInterrupted  = 0;
static void interruption_begin ();
static void interruption_end ();
static void interruption_callback (void *data, UInt32 state);



//========================================================================= SENAUDIO
//------------------------------------------------------------------------- SENAudio init - destroy

void sen_audio_init()
{
  mapSound = kh_init(hmsp);


  OSStatus status = AudioSessionInitialize(NULL, NULL,
                                           interruption_callback, NULL);
  if (status != kAudioSessionNoError) {
    _logfe("AudioSessionInitialize FAILED");
    return;
  }

  interruption_end();
}

void
sen_audio_destroy()
{
  soundMap_clear();
  kh_destroy(hmsp, mapSound);
  openAL_shutdown();
}

void
sen_audio_update()
{
}


//------------------------------------------------------------------------- SENAudio bg music
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

//------------------------------------------------------------------------- SENAudio sounds
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
}void
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



//===================================================================================================
static void interruption_begin ()
{
  _logfi("  .begin interruption");
  sen_music_stop(1);
  sen_sound_stop_all();
  openAL_shutdown();

  UInt32 cat = isDevicePlayingMusic ? kAudioSessionCategory_MediaPlayback :
                                      kAudioSessionCategory_UserInterfaceSoundEffects;
	AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(cat), &cat);
  AudioSessionSetActive(YES);	AudioSessionSetActive(NO);
}

static void interruption_end ()
{
	UInt32 isPlaying;
	UInt32 psize = sizeof(isPlaying);

  openAL_init();

	OSStatus err = AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying,
  &psize, &isPlaying);

#ifdef SEN_DEBUG
	if(err)
    _logfe("AudioSessionGetProperty Error:%d", err);
  else
  _logfi("_OtherAudioIsPlaying = %u", isPlaying);
#else
  UNUSED(err);
#endif

  isDevicePlayingMusic = 	isPlaying ? 1 : 0;
  UInt32	cat = isDevicePlayingMusic ? kAudioSessionCategory_AmbientSound :
                                        kAudioSessionCategory_SoloAmbientSound;

  AudioSessionSetProperty(kAudioSessionProperty_AudioCategory,
                            sizeof(cat), &cat);
	AudioSessionSetActive(YES);
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

static void interruption_callback (void *data, UInt32 state)
{
  if (state == kAudioSessionBeginInterruption)
	{
    _logfi("Audio Interruption ON");
		interruption_begin();
    isInterrupted = 1;
  }
	else if (state == kAudioSessionEndInterruption && isInterrupted)
	{
    _logfi("Audio Interruption OFF");
    interruption_end();
    isInterrupted = 0;
  }
}
/*
static sound_t* sound_new( const char* path )
{
  struct_malloc(sound_t, self);
  sen_assert(path);

  #if SEN_PLATFORM == SEN_PLATFORM_IOS
  self->path = strdup(sen_assets_get_full_path(path));
  #else
  self->path = strdup(path);
  #endif

  self->data = NULL;

  ALenum  format;
  ALsizei size;
  ALsizei freq;

  self->error = AL_NO_ERROR; alGetError();

  #undef check_exit
  #define check_exit(code) do{ code; if ((self->error = alGetError()) != AL_NO_ERROR) { _logfe("OpenAL Error[0x%X]: %s", self->error, #code); return self; }	} while(0)

  NSURL *fileURL = [NSURL fileURLWithPath:[[NSString alloc] initWithCString:self->path encoding:NSASCIIStringEncoding]];

  if (!fileURL)
  {
    _logfw("%s not Found", self->path);
    return self;
  }

  self->data = getOpenALAudioData(CFBridgingRetain(fileURL), &size, &format, &freq, &(self->duration) );
  if (!self->data) return self;

  check_exit( alGenBuffers(1, &(self->bufferID) ) );

  if (alBufferDataStaticProc == NULL)
    alBufferDataStaticProc = (alBufferDataStaticProcPtr) alGetProcAddress((const ALCchar*) "alBufferDataStatic");

    check_exit( alBufferDataStaticProc(self->bufferID, format, self->data, size, freq) );
    check_exit( alGenSources(1, & (self->sourceID) ) );
    check_exit( alSourcei(self->sourceID, AL_BUFFER, self->bufferID) );

    //	if (loops)
    //  check_exit( alSourcei(self->sourceID, AL_LOOPING, AL_TRUE) );

    return self;
}
*/

static sound_t* sound_new( const char* path )
{
  ALenum  format;
  ALsizei size;
  ALsizei freq;
  //ALboolean loop = AL_FALSE;
  ALuint sourceID;

  struct_malloc(sound_t, self);
  sen_assert(path);

  #if SEN_PLATFORM == SEN_PLATFORM_IOS
  self->full_path = strdup(sen_assets_get_full_path(path));
  #else
  self->full_path = strdup(path);
  #endif

  //self->full_path = NULL;sen_assets_get_full_path(path);
  self->path = sen_strdup(path);
  self->sources = vector_new(sizeof(ALuint));

  _logfi("New sound : %s", self->full_path);
  #undef check_exit
  #define check_exit(code) do{ code; if ((self->error = alGetError()) != AL_NO_ERROR) { _logfe("OpenAL Error[0x%X]: %s", self->error, #code); return self; }	} while(0)

  self->error = AL_NO_ERROR; alGetError();

  #undef check_exit
  #define check_exit(code) do{ code; if ((self->error = alGetError()) != AL_NO_ERROR) { _logfe("OpenAL Error[0x%X]: %s", self->error, #code); return self; }	} while(0)

  NSURL *fileURL = [NSURL fileURLWithPath:[[NSString alloc] initWithCString:self->full_path encoding:NSASCIIStringEncoding]];

  if (!fileURL)
  {
    _logfw("%s not Found", self->path);
    return self;
  }

  self->data = getOpenALAudioData(CFBridgingRetain(fileURL), &size, &format, &freq, &(self->duration) );
  if (!self->data) return self;

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


static void* getWaveAudioData(CFURLRef inFileURL,
                              ALsizei *outDataSize,
                              ALenum *outDataFormat,
                              ALsizei* outSampleRate,
                              ALdouble* duration)
{
  OSStatus err = noErr;
  UInt64 size = 0;
  void* data = NULL;
  AudioStreamBasicDescription format;
  UInt32 psize = sizeof(format);
  AudioFileID id = 0;
#undef check_exit
#define check_exit(error, errmsg) \
do{if(error){_logfe(errmsg,error); \
if(data) free(data); \
if (id) AudioFileClose(id);return NULL;}}while(0)
#define check_exit2(code, errmsg) do{err=code;check_exit(err,errmsg); } while (0)

  check_exit2(AudioFileOpenURL(inFileURL, kAudioFileReadPermission, 0, &id),
              "AudioFileOpenURL FAILED, Error = %ld");

  check_exit2(AudioFileGetProperty(id, kAudioFilePropertyDataFormat, &psize, &format),
              "AudioFileGetProperty(kAudioFileProperty_DataFormat) FAILED, Error = %ld");

  check_exit( format.mChannelsPerFrame > 2,
             "Unsupported Format, channel count is greater than stereo");

  check_exit( format.mFormatID != kAudioFormatLinearPCM || !TestAudioFormatNativeEndian(format),
             "Unsupported Format, must be little-endian PCM");

  check_exit( format.mBitsPerChannel != 8 && format.mBitsPerChannel != 16,
             "Unsupported Format, must be 8 or 16 bit PCM");

  psize = sizeof(size);

  check_exit2(AudioFileGetProperty(id, kAudioFilePropertyAudioDataByteCount, &psize, &size),
              "AudioFileGetProperty(kAudioFilePropertyAudioDataByteCount) FAILED, Error = %ld");

  UInt32 size32 = (UInt32)size;  data = malloc(size32);

  check_exit( data==NULL, "Not enough memory for audio data" );

  check_exit2(AudioFileReadBytes(id, false, 0, &size32, data),
              "AudioFileReadBytes FAILED, Error = %ld");

  *outDataSize = (ALsizei)size32;
  *outDataFormat = format.mBitsPerChannel == 16 ?
  ((format.mChannelsPerFrame > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16) :
  ((format.mChannelsPerFrame > 1) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8);
  *outSampleRate = (ALsizei)format.mSampleRate;

  double soundDuration; psize = sizeof(soundDuration);

  check_exit2(AudioFileGetProperty(id, kAudioFilePropertyEstimatedDuration, &psize, &soundDuration);,
              "AudioFileGetProperty(kAudioFilePropertyEstimatedDuration) FAILED, Error = %ld");

  return data;
}

static void* getOpenALAudioData(CFURLRef inFileURL,
                                ALsizei *outDataSize,
                                ALenum *outDataFormat,
                                ALsizei* outSampleRate,
                                ALdouble* duration)
{
  CFStringRef ext = CFURLCopyPathExtension(inFileURL);
  if (ext && CFStringCompare (ext,(CFStringRef)@"wav", kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
    CFRelease(ext);
    return getWaveAudioData(inFileURL, outDataSize, outDataFormat, outSampleRate, duration);
  }
  return NULL;
}
