
local ffi = require "ffi"
local C   = ffi.C

local utils = require "sen.utils"
local super = require "sen.object"

ffi.cdef[[

typedef enum blend_func {
  bfDisable,
  bfPremultiAlpha,
  bfStraightAlpha,
  bfAdditive,
} blend_func ;

typedef struct node_t {
  object_t          super;
  void*         children;

  float             posX;
  float             posY;
  float             posZ;

  float             scaleX;
  float             scaleY;

  float             rotZ_X;
  float             rotZ_Y;

  float             anchorX;
  float             anchorY;

  int               updated;

  mat4              model;
  vec4              color;

  vec4              bbox;
  vec2              size;
  blend_func        blend;

  void* vt;
} node_t;

void
sen_node_add_child(void* self, void* node, int setParent);

void
sen_node_remove_child(void* _self, const char* name, void *obj, uint32_t child_id);

void
sen_node_init(void*       _self,
              const char* _name,
              void*       _parent);

void
sen_node_clean(void* _self);

node_t*
sen_node_new();

void
sen_node_destroy(void* self);  

void
sen_node_set_colorRGBA(void* _self, float r, float g, float b, float a); 
  
void
sen_node_move(void* _self,float dx, float dy);
void
sen_node_moveTo(void* _self,float x, float y);
void
sen_node_scale(void* _self,float sx, float sy);

void
sen_node_set_rotation(void* _self, float rotation);
void
sen_node_set_anchor(void* _self, float ax, float ay);
void
sen_node_Z(void* _self,float z);

void
sen_node_scaleM(void* _self,float mx, float my);
                   
]]

local CT    = ffi.typeof("node_t")
local CTP   = ffi.typeof("node_t*")
local CAST  = function(self)
  return ffi.cast(CTP, self)
end
      
local sen_node_class = {}
      
function sen_node_class.inherit(self)
  local self = super.inherit(self)
  local ref = CAST(self.ref)

  function self.addChild(child, makeParented)
    if child then
      C.sen_node_add_child(self.ref, child.ref,1)
    end
  end  
  
  function self.delChild(child)
    
    if child then
      if type(child) == "string" then
        C.sen_node_remove_child(self.ref,   child, nil, 0)
      else 
        C.sen_node_remove_child(self.ref,   child.name(), nil, 0)
        child = nil
      end  
    end
  end
  
  function self.clear()
    C.sen_node_remove_child(self.ref, nil, nil, 0)
  end
    
  function self.delete()
     local parent = CAST( super.cast(ref).parent )
     C.sen_node_remove_child(parent, self.name(), nil, 0)
  end
  

  function self.color()
    local c = ref.color
    return {r=c.x, g=c.y, b=c.z, a=c.w}
  end

  function self.setColor(r,g,b,a)
    local color
    if type(r) == "table" then
      color = r
    else 
      color = {r=r,g=g,b=b,a=a}
    end  
    if color.r == nil then color.r = color[1] or ref.color.x end
    if color.g == nil then color.g = color[2] or ref.color.y end
    if color.b == nil then color.b = color[3] or ref.color.z end
    if color.a == nil then color.a = color[4] or ref.color.w end

    if color.r >1 then color.r = color.r/255 end
    if color.g >1 then color.g = color.g/255 end
    if color.b >1 then color.b = color.b/255 end
    if color.a >1 then color.a = color.a/255 end

    C.sen_node_set_colorRGBA(self.ref,  color.r,  color.g,  color.b,  color.a)
  end

  function self.rotate(deg)
    C.sen_node_set_rotation(self.ref,deg)
  end
  
  function self.getBBox()
    return utils.bbox(ref.bbox)
  end

  function self.rot()
    return ref.rotZ_X
  end
  
  function self.posX()
    return ref.posX
  end

  function self.posY()
    return ref.posY
  end

  function self.width()
    return ref.bbox.z - ref.bbox.x 
  end
  function self.height()
    return ref.bbox.w - ref.bbox.y 
  end
  
  function self.move(dx, dy)
    C.sen_node_move(self.ref, dx, dy)
  end
  
  function self.moveTo(x, y)
    C.sen_node_moveTo(self.ref, x, y)
  end
  
  function self.setAnchor(x, y)
    C.sen_node_set_anchor(self.ref, x, y)
  end
  

  function self.scale(sx, sy, mult)
    if mult then
      C.sen_node_scaleM(self.ref,sx,sy);
    else
      C.sen_node_scale(self.ref, sx, sy)
    end  
  end
  
  function self.scaleX()
    return ref.scaleX
  end

  function self.scaleY()
    return ref.scaleY
  end
  
  function self.blend(blend)
    ref.blend = blend
  end

  function self.ZOrder(z)
    C.sen_node_Z(ref, z or 0);
  end
  
  ----[[
  function self.connect(signal_name, func, listener)
    
  
    C.sen_signal_connect(self.name(),
                         signal_name,
                         func,
                         super.cast(listener.ref))
  end
  --]]
  
  return self;
end

function sen_node_class.new(name)
  local ref = C.sen_node_new()
  C.sen_node_init(ref, name, nil)
  local self = sen_node_class.inherit({ ref = ref  })
  return self
end

setmetatable(sen_node_class, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})
      
     
return sen_node_class