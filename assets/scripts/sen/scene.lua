
local ffi = require "ffi"
local C   = ffi.C

local utils = require "sen.utils"
local super = require "sen.node"

ffi.cdef[[

  typedef struct scene_t {
    node_t super;
    const void* sig_resize;
    
  } scene_t;
  
  scene_t*
  sen_scene_new(const char* name);
  
  void
  sen_scene_delete(void* self);
  
  void
  sen_set_scene(scene_t* scene);
  
  void
  sen_set_scene_name(const char* name);
  
  scene_t*
  sen_get_scene();
  
]]

local CT    = ffi.typeof("scene_t")
local CTP  = ffi.typeof("scene_t*")

local sen_scene_class = {}

local current = nil
local prev = nil

function sen_scene_class.inherit(self)
  local self = super.inherit(self)
  
  function self.setCurrent()
    if (current and current.onBG) then
      current.onBG()
    end
    prev = current
    C.sen_set_scene(self.ref)
    current = self
    if (current.onFG) then
      current.onFG()
    end
  end
  
  function self.Prev()
    return prev
  end
  
  return self;
end

function sen_scene_class.new(name)
  local ref = ffi.gc(C.sen_scene_new(name), C.sen_scene_delete)
  local self = sen_scene_class.inherit( { ref = ref } )
    
  return self
end

function sen_scene_class.getCurrent()
  return sen_scene_class.inherit( { ref = C.sen_get_scene() } )
end


function sen_scene_class.setCurrentByName(name)
  C.sen_set_scene_name(name)
end
  
setmetatable(sen_scene_class, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})        
      
return sen_scene_class