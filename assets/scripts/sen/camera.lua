
local ffi = require "ffi"
local C   = ffi.C


local utils = require "sen.utils"
local super = require "sen.node"

ffi.cdef[[
void*
sen_camera();
]]


local sen_camera_class = {}

function sen_camera_class.inherit(self)
  local self = super.inherit(self)
  local ref = self.ref
  
  return self;
end

function sen_camera_class.new()
  local ref = C.sen_camera()
  local self = sen_camera_class.inherit( { ref = ref } )
  return self
end  

setmetatable(sen_camera_class, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})  

return sen_camera_class