#include "macro.h"
#include "logger.h"
#include "utils.h"
#include "audio.h"
#include "hrtimer.h"
#include "khash.h"
#include "asset.h"
#include "khash.h"
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
static alBufferDataStaticProcPtr  alBufferDataStaticProc = NULL;
#endif
/*
@interface sound_t : NSObject 
{
	ALuint          bufferID;		
	ALuint          sourceID;
	ALdouble        duration;
	ALfloat         volume;		
	ALfloat         pitch;		
	
	ALenum          error;				
	ALvoid         *data;	

	NSMutableArray *temp;
	NSString       *path;
}

@property (nonatomic, readonly) ALenum error;
@property (nonatomic, readonly) ALdouble duration;
@property (nonatomic) ALfloat volume;
@property (nonatomic) ALfloat pitch;
@property (nonatomic, copy, readonly) NSString *path;

- (id) initLoad:(NSString *)file loop:(BOOL)loops;

- (BOOL) play;
- (BOOL) stop;
- (BOOL) pause;
- (BOOL) rewind;
- (BOOL) is_playing;
- (BOOL) is_anyPlaying;
@end
*/
//------------------------------------------------------------------------- openAL Player

typedef struct sound_t 
{
	ALuint          bufferID;		
	ALuint          sourceID;
	ALdouble        duration;
	ALfloat         pitch;		
	ALenum          error;				
	ALvoid         *data;	
	char           *path;
} sound_t;

static sound_t* sound_new( const char* path );
static sound_t* sound_ref( sound_t* s );
static void sound_delete( sound_t* self);
static int sound_playing( sound_t* self);
static void sound_play( sound_t* self);
static void soundMap_collect();
static sound_t* soundMap_get(unsigned int key);
static void soundMap_set(const char* path);

static int isDevicePlayingMusic = 0;
static int isInterrupted  = 0;
static khash_t(hmip)* mapSound = NULL;
static ALdouble minDuration = 1000000.0f;

static void interruption_begin ();
static void interruption_end ();
static void interruption_callback (void *data, UInt32 state);
static void openAL_shutdown();
static void openAL_init();
static void clear_sounds(int refs);
static unsigned int hash_string(const char *s);


//========================================================================= SENAUDIO
//------------------------------------------------------------------------- SENAudio init - destroy

void sen_audio_init()
{
  mapSound = kh_init(hmip);


  OSStatus status = AudioSessionInitialize(NULL, NULL, 
                                           interruption_callback, NULL);
  if (status != kAudioSessionNoError) {
    _logfe("AudioSessionInitialize FAILED");
    return;
  }
  
  interruption_end();

  //sound_t *s = sound_new("assets/audio/move0.wav");  
//  sound_play(s);
}

void
sen_audio_destroy()
{
  clear_sounds(0);
  kh_destroy(hmip, mapSound);

  openAL_shutdown();

  //clear_sounds();
}

void
sen_audio_update()
{
  soundMap_collect();
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
  unsigned int key = 0;
  if (!path || !path[0]) return;
  key = hash_string(path);
  if ( soundMap_get(key) ) return;
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
  unsigned int key = (unsigned int)hash_string(path);
  sen_sound_preload(path);
  p = soundMap_get(key);
  if (p) 
    sound_play(p);
  return key;
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


//========================================================================= IMPLEMETATION
//------------------------------------------------------------------------- openAL support: IMPLEMETATION
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

//------------------------------------------------------------------------- openAL Sound : IMPLEMENTATION
/*
@interface sound_t()
@property (nonatomic, retain) NSMutableArray *temp;
@property (nonatomic, copy) NSString *path;
-(BOOL) load:(NSString *)file loop:(BOOL)loops;
+(NSString*)fullPath:(NSString*)relPath;
@end

@implementation sound_t
@synthesize error, temp, duration, path;

#pragma mark -
#pragma mark get full path

+(NSString*) fullPath:(NSString*) relPath
{
  sen_assert(path);
     
  if(([relPath length] > 0) && ([relPath characterAtIndex:0] == '/'))
    return relPath;
    
  NSMutableArray *parts = [NSMutableArray arrayWithArray:[relPath pathComponents]];
  NSString *fname = [parts lastObject];
    
  [parts removeLastObject];
  NSString *fdir = [NSString pathWithComponents:parts];
    
  NSString *fullpath = [[NSBundle mainBundle] pathForResource:fname ofType:nil inDirectory:fdir];
  if (fullpath == nil)
    fullpath = relPath;
    
  return fullpath;    
}

#pragma mark -
#pragma mark init - load - dealloc

- (id) initLoad:(NSString *)file loop:(BOOL)loops
{
  _logfi("init sound with file %s",  [file UTF8String]);
	self = [super init];
  self.path = file;
  data = NULL;
  if (!self) 
    return nil;

  if(![self load:file loop:loops])
    return nil;
  
  self.temp = [NSMutableArray array];
  self.volume = 1.0;
  self.pitch = 1.0;		
  
  return self;
}

- (void) dealloc
{
  
  _logfi("deallocing sound %s, sourceID:%u, bufferID:%u",  [path UTF8String],  sourceID, bufferID);
  
	alDeleteSources(1, &sourceID);
	
	for(NSNumber *tmp in temp)
	{
		NSUInteger srcID = [tmp unsignedIntegerValue];
		alDeleteSources(1, &srcID);
	}
	alDeleteBuffers(1, &bufferID);
	
	if(data) free(data);
	
}

#pragma mark -
#pragma mark load file
-(BOOL) load:(NSString *)file loop:(BOOL)loops
{		
	ALenum  format;
	ALsizei size;
	ALsizei freq;
	error = AL_NO_ERROR; alGetError(); 

  #undef check_exit
  #define check_exit(code) do{ code; if ((error = alGetError()) != AL_NO_ERROR) { _logfe("OpenAL Error[0x%X]: %s", error, #code); return NO; }	} while(0)
  
	NSURL *fileURL = [NSURL fileURLWithPath:[sound_t fullPath:file]];

	if (!fileURL)
	{
    _logfw("%s not Found", [file UTF8String]);
		return NO;
	}
	
	data = getOpenALAudioData(CFBridgingRetain(fileURL), &size, &format, &freq, &duration);
  if (!data) return NO;

  check_exit( alGenBuffers(1, &bufferID) );
  
  if (alBufferDataStaticProc == NULL) 
    alBufferDataStaticProc = (alBufferDataStaticProcPtr) alGetProcAddress((const ALCchar*) "alBufferDataStatic");    
  
  check_exit( alBufferDataStaticProc(bufferID, format, data, size, freq) );
  check_exit( alGenSources(1, &sourceID) );
  check_exit( alSourcei(sourceID, AL_BUFFER, bufferID) );
	
	if (loops)
    check_exit( alSourcei(sourceID, AL_LOOPING, AL_TRUE) );
	
	return YES;	
}

#pragma mark -
#pragma mark play

- (void) deleteTemp
{
	NSUInteger tmpID = [[temp objectAtIndex:0] unsignedIntegerValue];
	[temp removeObjectAtIndex:0];
	alDeleteSources(1, &tmpID);	
}

- (BOOL) play
{
	if([self is_playing]) 
	{
		NSUInteger tmpID;
		alGenSources(1, &tmpID);
		alSourcei(tmpID, AL_BUFFER, bufferID);
		alSourcePlay(tmpID);
		[temp addObject: [NSNumber numberWithUnsignedInteger:tmpID]];
		
		[self performSelector:@selector(deleteTemp)
      withObject:nil
      afterDelay:(duration * pitch) + 0.1];
		
		return ((error = alGetError()) != AL_NO_ERROR);
	}
	alSourcePlay(sourceID);
	return ((error = alGetError()) != AL_NO_ERROR);
}

- (BOOL) stop
{
	alSourceStop(sourceID);
	return ((error = alGetError()) != AL_NO_ERROR);
}

- (BOOL) pause
{
	alSourcePause(sourceID);
	return ((error = alGetError()) != AL_NO_ERROR);
}

- (BOOL) rewind
{
	alSourceRewind(sourceID);
	return ((error = alGetError()) != AL_NO_ERROR);
}

- (BOOL) is_playing
{	
	ALenum state;	
  alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
  return (state == AL_PLAYING);
}

- (BOOL) is_anyPlaying
{
	return [self is_playing] || [temp count] > 0;
}

#pragma mark -
#pragma mark volume and stuff

- (ALfloat) volume
{
	return volume;
}

- (void) setVolume:(ALfloat)newVolume
{
	volume = MAX(MIN(newVolume, 1.0f), 0.0f); 
	alSourcef(sourceID, AL_GAIN, volume);	
	
	for(NSNumber *tmpID in temp)
    alSourcef([tmpID unsignedIntegerValue], AL_GAIN, volume);
}

- (ALfloat) pitch
{
	return pitch;
}

- (void) setPitch:(ALfloat)newPitch
{
	pitch = newPitch;
	alSourcef(sourceID, AL_PITCH, pitch);	
	for(NSNumber *tmpID in temp)
		alSourcef([tmpID unsignedIntegerValue], AL_PITCH, pitch);
}
@end
*/

//===================================================================================================
//
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
	ALCdevice	*device = NULL;

	device = alcOpenDevice(NULL); 
	if(!device) return;
		
	context = alcCreateContext(device, NULL);
	if(!context) return;
	
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

static const char* get_fpath(const char* path) {
  sen_assert(path);
  char* assets_sub = strstr(path, "assets/" );
  if (assets_sub == path)
    path += strlen("assets/");
     
  NSMutableString* relative_path = 
      [[NSMutableString alloc] initWithString:@"/assets/"];
  
  [relative_path appendString:
    [[NSString alloc] initWithCString:path encoding:NSASCIIStringEncoding]];
  return
    [[[NSBundle mainBundle] pathForResource:relative_path ofType:nil]
     cStringUsingEncoding:NSASCIIStringEncoding];
}

static void clear_sounds(int refs)
{
  void* ps;
  kh_foreach_value(mapSound, ps, 
    sound_t* s = (sound_t*)ps; 
    if (refs && s->data) continue;
    sound_delete(s);
  );
  kh_clear(hmip, mapSound);
}

static unsigned int hash_string(const char *s)
{
	unsigned int h = (unsigned int)*s;
	if (h) for (++s ; *s; ++s) h = (h << 5) - h + (unsigned int)*s;
	return h;
}

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

static sound_t* sound_ref( sound_t* s )
{
  struct_malloc(sound_t, self);
  self->data = NULL;
  self->path = NULL;
  self->pitch = s->pitch;
  self->duration = s->duration;
  self->bufferID = s->bufferID;
	alGenSources(1, &self->sourceID);
	alSourcei(self->sourceID, AL_BUFFER, self->bufferID);
  return self;
}

static void sound_delete( sound_t* self)
{
  if (self->path) free(self->path);
  if (self->error != AL_NO_ERROR)
 	  alDeleteSources(1, &(self->sourceID) );
  if (self->data) {
  	alDeleteBuffers(1, &(self->bufferID) );
    free(self->data);	
  }
  free(self);
}

static int sound_playing( sound_t* self)
{
	ALenum state;	
  alGetSourcei(self->sourceID, AL_SOURCE_STATE, &state);
  return (state == AL_PLAYING);
}

static void sound_play( sound_t* self)
{
  if (!sound_playing(self))
	  alSourcePlay(self->sourceID);
}


static void soundMap_collect()
{
}

static sound_t* soundMap_get(unsigned int key)
{
  khiter_t j = kh_get(hmip, mapSound, key);
  return j != kh_end(mapSound) ?
    (sound_t*) kh_val(mapSound, j) :
    NULL;
}

static void soundMap_set(const char* path)
{
  sound_t* s = NULL;
  unsigned int key = hash_string(path);
  khiter_t j = kh_get(hmip, mapSound, key);
  if (j == kh_end(mapSound))
  {
    s = sound_new(path);
    if (s->duration < minDuration) minDuration = s->duration;
    kh_insert(hmip, mapSound, key, (void*) s);
  }
}
