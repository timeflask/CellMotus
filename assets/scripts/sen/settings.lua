
local ffi = require "ffi"
local C   = ffi.C
local utils = require "sen.utils"
local inspect = require "sen.inspect"

ffi.cdef[[
  
char*
sen_settings_get_str(const char* name, const char* default_str);

void
sen_settings_set_str(const char* name, const char* str);
    
]]
 
local function SettingsrClosure(name)
  
  -- public
  local _ = {}
  
  local m_name = name or '.game'

  local function getString(key, defValue)
    local p = ffi.gc(C.sen_settings_get_str(key, defValue or ''), ffi.C.__free)
    return ffi.string(p)
  end
  
  local function setString(key, value)
    C.sen_settings_set_str(key, value)
  end

  
  local m_settings = loadstring('return '..getString(m_name, [[
  {
    progress = 1,
    curr = 1,
    music_vol = 0.1,
    sounds_vol = 0.3,
  }
  ]]))()
    
  
  local function commit()
     local str = inspect(m_settings)
     setString(m_name, str)    
  end
   
  function _.set(key, value)
    m_settings[key] = value
    commit()
  end
  
  function _.get(key, def)
    local res = m_settings[key] 
    if res ~= nil then return res else return def end
    return res~=nil and res or def    
  end

  function _.copy()
    local ret = {}
    for k,v in pairs(m_settings) do
      ret [k] = v
    end  
    return ret
  end
                      
  function _.reset(s)
    for k,v in pairs(s) do
      _.set(k,v)
    end  
  end
                          
                     
  return _;
end

return SettingsrClosure() 
