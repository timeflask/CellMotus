
local ffi = require "ffi"
local C   = ffi.C


local utils = require "sen.utils"
local super = require "sen.node"

ffi.cdef[[

 void  *
 sen_quad_new(const char* name);

 void
 sen_quad_delete(void *self);
  
]]

--local CT    = ffi.typeof("sprite_t")
--local CTP  = ffi.typeof("sprite_t*")

local sen_quad_class = {}

function sen_quad_class.inherit(self)
  local self = super.inherit(self)
  local ref = self.ref
  
  return self;
end

function sen_quad_class.new(name)
  local ref = C.sen_quad_new(name)
  local self = sen_quad_class.inherit( { ref = ref } )
  return self
end  


setmetatable(sen_quad_class, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})  

return sen_quad_class