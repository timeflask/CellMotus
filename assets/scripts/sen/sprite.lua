
local ffi = require "ffi"
local C   = ffi.C


local utils = require "sen.utils"
local super = require "sen.node"

ffi.cdef[[

  
  void *
  sen_sprite_new(const char* name,
                 const char* texture_name);
  
  void
  sen_sprite_delete(void *self);  
]]

--local CT    = ffi.typeof("sprite_t")
--local CTP  = ffi.typeof("sprite_t*")

local sen_sprite_class = {}

function sen_sprite_class.inherit(self)
  local self = super.inherit(self)
  local ref = self.ref
  
  return self;
end

function sen_sprite_class.new(name, image_name)
  local ref = C.sen_sprite_new(name, image_name)
  local self = sen_sprite_class.inherit( { ref = ref } )
  return self
end  

function sen_sprite_class.create(name, image_name)
  local ref = ffi.gc(C.sen_sprite_new(name, image_name), C.sen_sprite_delete) 
  local self = sen_sprite_class.inherit( { ref = ref } )
  return self
end  

function sen_sprite_class.imageBBox(image_name)
  local sprite = sen_sprite_class.create(nil, image_name)
  local sprite_box = sprite.getBBox()
  sprite = nil
  return sprite_box
end  


setmetatable(sen_sprite_class, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})  

return sen_sprite_class 