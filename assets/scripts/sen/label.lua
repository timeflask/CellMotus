
local ffi = require "ffi"
local C   = ffi.C

local utils = require "sen.utils"
local super = require "sen.node"

ffi.cdef[[
   typedef struct label_t {
    node_t      super;
    void*       font;
    void*       buff;
    const void* program;
    void*       text;
    size_t      text_length;
  } label_t;

  label_t *
  sen_label_new(const char* name,
                const char* font_name,
                const char* utf8_text);

  void
  sen_label_delete(void *self);

  void
  sen_label_set_text(void* self, const char* utf8_text);  
    
]]

local sen_label_class = {}


function sen_label_class.inherit(self)
  local self = super.inherit(self)
  local ref = self.ref
  
  function self.setText(text)
    if text then
      C.sen_label_set_text(ref, text)
    end
  end
  return self;
end

function sen_label_class.new(name, font, text)
  local ref = C.sen_label_new(name, font, text or "UNTITLED ["..name.."]")
  local self = sen_label_class.inherit( { ref = ref } )
  return self
end  

function sen_label_class.create(name, font, text)
  local ref = ffi.gc(C.sen_label_new(name, font, text or "UNTITLED ["..name.."]"), C.sen_label_delete) 
  local self = sen_label_class.inherit( { ref = ref } )
  return self
end  
      
setmetatable(sen_label_class, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})      
      
return sen_label_class
