
local ffi = require "ffi"
local C   = ffi.C
local settingsManager = require "sen.settings"

ffi.cdef[[
  
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
    
]]
 
local function MusicManagerClosure()
  local m_search_path = "audio/"
  local m_enabled = true
  
  -- public
  local _ = {}
  
  function _.preloadMusic(path)
    C.sen_music_preload(m_search_path..path)
  end
   
  function _.playMusic(path, loop)
    C.sen_music_play(m_search_path..path, loop or 0)
  end

  function _.stopMusic()
    C.sen_music_stop(0)
  end
   
  function _.pauseMusic()
    C.sen_music_pause()
  end

  function _.resumeMusic()
    C.sen_music_resume()
  end

  function _.rewindMusic()
    C.sen_music_rewind()
  end

  function _.isMusicPlaying()
    return C.sen_music_is_playing() > 0
  end

  function _.setMusicVol(vol)
    C.sen_music_set_vol(vol)
  end

  function _.getMusicVol()
    return C.sen_music_get_vol()
  end

  function _.preloadSound(path)
    C.sen_sound_preload(m_search_path..path)
  end

  function _.unloadSound(path)
    C.sen_sound_unload(m_search_path..path)
  end

  function _.playSound(path, loop, pitch, pan, gain)
    local ret = -1
    if m_enabled and settingsManager.get("sounds_enabled", true) then
      ret = C.sen_sound_play_ex(m_search_path..path, loop or 0, pitch or 1.0, pan or 0.0, gain or 1.0)
    end
    return ret  
  end

  function _.pauseSound(id)
    if id then
      C.sen_sound_pause(id)
    else
      C.sen_sound_pause_all()
    end  
  end

  function _.resumeSound(id)
    if id then
      C.sen_sound_resume(id)
    else
      C.sen_sound_resume_all()
    end  
  end

  function _.stopSound(id)
    if id then
      C.sen_sound_stop(id)
    else
      C.sen_sound_stop_all()
    end  
  end

  function _.setSoundsVol(vol)
    C.sen_sound_set_vol(vol)
  end

  function _.getSoundsVol()
    return C.sen_sound_get_vol()
  end

  function _.enable()
    m_enabled = true
  end
  
  function _.disable()
    m_enabled = false
  end
                    
  return _;
end

return MusicManagerClosure() 
