
require "class"
local ffi = require "ffi"
local C   = ffi.C
local utils   = require "sen.utils"


ffi.cdef[[
   typedef struct object_t {
    char             name[32];
    uint32_t         uid;
    struct object_t* parent;
  } object_t;
  void
  sen_object_init(void* _self, const char* _name, void* _parent);
  void
  sen_object_set_name(void* _self, const char* _name);
  
typedef int (*signal_callback_t)(object_t*, void* data, object_t*, const char*);
  


void
sen_signal_connect(const char*       emitter_name,
                   const char*       signal_name,
                   signal_callback_t proc,
                   struct object_t*  listener);
void
sen_signal_connect_name(const char*       emitter_name,
                        const char*       signal_name,
                        signal_callback_t proc,
                        const char*       listener_name);
void
sen_signal_disconnect(object_t*         listener,
                      const char*       signal_name,
                      const char*       emitter_name);

void
sen_signal_disconnect_name(const char*       listener_name,
                           const char*       signal_name,
                           const char*       emitter_name);                        
                          
void
sen_signal_connect_lua(const char*       emitter_name,
                       const char*       signal_name,
                       struct object_t*  listener);

]]
 
local CT   = ffi.typeof("object_t")
local CTP  = ffi.typeof("object_t*")
local CAST = function(self)
  return ffi.cast(CTP, self)
end

local sen_object_class = {
  ct  = ffi.typeof("object_t"),
  ctp  = ffi.typeof("object_t*"),
  cast = CAST,
}

function sen_object_class.inherit(self)
  local self = self or {}

  function self.uid(id)
    if id then
      CAST(self.ref).uid = id
    end 
    return CAST(self.ref).uid
  end
  
  function self.name(name)
    if name then
      C.sen_object_set_name(self.ref, name)
    end 
    return ffi.string( CAST(self.ref).name );
  end

 
  return self
end

setmetatable(sen_object_class, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})



local slots = utils.auto_table(2)
--[[
local singal_wprapper = ffi.cast("signal_callback_t", function (listener, data, emitter, sig)
  local slot = slots[ffi.string(listener.name)][ffi.string(sig)]
  local ret = 0
  if (slot ~= nil) then
    ret = slot.func(  slot.self, data, emitter )
  end
  return ret or 0  
end)
--]]

--local CTP  = ffi.typeof("object_t*")

function LUA_sen_signals_callback(self, data, emitter, sig)
  local listener = ffi.cast(CTP,self)
  local slot = slots[ffi.string(listener.name)][sig]
  local ret = 0
  if (slot ~= nil) then
    ret = slot.func(  slot.self, data, emitter )
  end
  return ret or 0  
--  print(ffi.string(listener.name))
  --return 1
end

local function connect(emitter, signal, func, listener)
  if ( slots[listener.name()][signal] == nil) then
    slots[listener.name()][signal] = {self=listener, func=func} 
    --C.sen_signal_connect(emitter, signal, singal_wprapper, CAST(listener.ref))
    C.sen_signal_connect_lua(emitter, signal, CAST(listener.ref))
  end                              
end                                
                                   
                                   

local function dc(listener, signal, emitter)
  if ( slots[listener.name()][signal] ~= nil) then
    slots[listener.name()][signal] = nil
    C.sen_signal_disconnect(CAST(listener.ref), signal, emitter) 
  end    
    
end

sen_object_class.connect = connect
sen_object_class.disconnect = dc

return sen_object_class
