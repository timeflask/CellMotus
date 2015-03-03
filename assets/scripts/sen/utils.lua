
local ffi = require "ffi"
local inspect = require "sen.inspect"
ffi.cdef[[
  void
  sen_logi(const char* tag, const char* fmt, ...);

  void
  sen_loge(const char* tag, const char* fmt, ...);

  void __free (void* ptr);

  typedef union
  {
    float data[2];
    struct {
          float x;
          float y;
      };
    struct {
          float s;
          float t;
      };
  } vec2;

  typedef union
  {
    float data[16];
    struct {
          float m00, m01, m02, m03;
          float m10, m11, m12, m13;
          float m20, m21, m22, m23;
          float m30, m31, m32, m33;
      };
  } mat4;

  typedef union
  {
    float data[4];
    struct { float x; float y; float z;float w; };
    struct {
          float x_;
          float y_;
          float width;
          float height;
      };
    struct {
          float r;
          float g;
          float b;
          float a;
      };
    struct {
          float red;
          float green;
          float blue;
          float alpha;
      };
    struct {
          float s0;
          float t0;
          float s1;
          float t1;
      };
  } vec4;

  typedef union
  {
    float data[3];   /**< All compoments at once    */
    struct {
          float x;     /**< Alias for first component */
          float y;     /**< Alias fo second component */
          float z;     /**< Alias fo third component  */
      };
    struct {
          float r;     /**< Alias for first component */
          float g;     /**< Alias fo second component */
          float b;     /**< Alias fo third component  */
      };
    struct {
          float red;   /**< Alias for first component */
          float green; /**< Alias fo second component */
          float blue;  /**< Alias fo third component  */
      };
  } vec3;

  typedef struct input_point {

    double x;
    double y;
    double z;
    double w;

  } input_point;

  typedef struct touch_t {
    int    id;
    int    has_start;
    input_point   start;
    input_point   point;
    input_point   prev;
  }touch_t;

  typedef struct scroll_t {double x; double y;} scroll_t;

  float sen_fps();

   void sen_reload();
   void sen_touch_to_world(touch_t* t, touch_t* res);

   int sen_platform_dpi();

   const char*
   sen_platform_screen_size_name();

   const vec4*
   sen_view_get_viewport();

   void sen_exit();

   const char*
   sen_platform_name();

]]

local SEN_LOG_TAG = "SEN:Lua"
local C   = ffi.C

local vec4p  = ffi.typeof("vec4*")
local vec4  = ffi.typeof("vec4")

local function bbox(ref)
  local bbox = ffi.cast(vec4p, ref)
  return
    {
      left   = bbox.x,
      bottom = bbox.y,
      right  = bbox.z,
      top    = bbox.w,

      l      = bbox.x,
      b      = bbox.y,
      r      = bbox.z,
      t      = bbox.w,
    }
end

local sen_log =  function (fmt, ...)
  C.sen_logi(SEN_LOG_TAG, fmt, ...)
end

stdPrint = print

print = function(...)
  sen_log(inspect({...}))
--  for arg,_ in ipairs({...}) do
    -- write arg to any file/stream you want here
  --end
end

local function meta_merge(t1, t2)
   for k,v in pairs(t2) do
     if k == '__index' then
       for n,m in pairs(t2.__index) do
           if (t1.__index[n] == nil) then
             t1.__index[n]=m
           end
       end
     else
       if (t1[k] == nil) then
         t1[k]=v
       end
     end
   end
   return t1
end

local sen_size_name = function()
  return ffi.string(C.sen_platform_screen_size_name())
end

local function ripairs(t)
    local ci = 0
    local remove = function()
        t[ci] = nil
    end
    return function(t, i)
        i = i+1
        ci = i
        local v = t[i]
        if v == nil then
            local rj = 0
            for ri = 1, i-1 do
                if t[ri] ~= nil then
                    rj = rj+1
                    t[rj] = t[ri]
                end
            end
            for ri = rj+1, i do
                t[ri] = nil
            end
            return
        end
        return i, v, remove
    end, t, ci
end

local function newAutotable(dim)
    local MT = {};
    for i=1, dim do
        MT[i] = {__index = function(t, k)
            if i < dim then
                t[k] = setmetatable({}, MT[i+1])
                return t[k];
            end
        end}
    end

    return setmetatable({}, MT[1]);
end

local touchp = ffi.typeof("touch_t*")
local function to_touchp(voidp)
  local res = ffi.new("touch_t")
  C.sen_touch_to_world(ffi.cast(touchp, voidp), res)
  return res
  --return ffi.cast(touchp, voidp)
end

local scrollp = ffi.typeof("scroll_t*")
local scrollt = ffi.typeof("scroll_t")
local function to_scroll(voidp)
  return ffi.cast(scrollp, voidp)
end

local function keyToCode(d)
  local i = ffi.cast("struct{int num;} *",d)
  return i.num
end

local function vp_box()
  local v = C.sen_view_get_viewport()
  local w = (v.z-v.x)/2
  local h = (v.w-v.y)/2
  local v2 = ffi.new(vec4)
  v2.x = -w
  v2.y = -h
  v2.z = w
  v2.w = h
  return bbox(v2)
end
--local FONT_SCALE_FACTOR = 300
local BABY_SIZE = 360
--local BABY_FONTSCALE = 2
local FONT_MIDDLE = 30
g_screen = nil
local function _lazy_init_screen()
  local v = C.sen_view_get_viewport()
  local d = C.sen_platform_dpi() 
  if (d==0) then d = 160  end
  --d=450
  --print("------------------------------------------------------")
  --print(d)
  --print("------------------------------------------------------")
 -- d=440
  local wdp = v.z*d/160
  local hdp =v.w*d/160
  local minwh = math.min(wdp,hdp) 
  local maxwh = math.max(wdp,hdp)
  local baby = math.min(wdp,hdp) < BABY_SIZE
  local max2 = math.min(v.z, v.w)
 -- local font_factor = (baby and BABY_FONTSCALE or math.ceil(max2/FONT_SCALE_FACTOR))
  return {width=v.z, height=v.w, name=sen_size_name(), dpi=d,
    width_dp = wdp, height_dp =hdp, baby = baby,
    minwh = minwh, maxwh = maxwh, 
    --font_factor = font_factor,
    
    fonts = {
      small = math.max(12, math.ceil(max2 / FONT_MIDDLE * 0.6 + 4*d/160)), 
      medium = math.max(16,math.ceil(max2 / FONT_MIDDLE  + 4*d/160)), 
      big =  math.max(30,math.ceil(max2 / FONT_MIDDLE * 1.5  + 4*d/160)),
      height = {} 
    }
  }
end
local function screen()
  if g_screen == nil then
    g_screen = _lazy_init_screen()
  end  
  return g_screen
end

local yield  = coroutine.yield

local coro_wait = function(dt,secs)
  local m_elapsed = 0
  while (m_elapsed < secs) do
    m_elapsed = m_elapsed + dt
    yield()
  end
end

local platform_name = function()

  local ret = ffi.string( C.sen_platform_name() )
  local scr = screen()
  if scr.baby then
    ret = ret..' BABY SCREEN'
  end

  return ret
end

local level=0

local function hook(event)
 local t=debug.getinfo(3)
 io.write(level," >>> ",string.rep(" ",level))
 if t~=nil and t.currentline>=0 then io.write(t.short_src,":",t.currentline," ") end
 t=debug.getinfo(2)
 if event=="call" then
  level=level+1
 else
  level=level-1 if level<0 then level=0 end
 end
 if t.what=="main" then
  if event=="call" then
   io.write("begin ",t.short_src)
  else
   io.write("end ",t.short_src)
  end
 elseif t.what=="Lua" then
-- table.foreach(t,print)
  io.write(event," ",t.name or "(Lua)"," <",t.linedefined,":",t.short_src,">")
 else
 io.write(event," ",t.name or "(C)"," [",t.what,"] ")
 end
 io.write("\n")
end

--debug.sethook(hook,"cr")
level=0
local function recursive_compare(t1,t2)
  if t1==t2 then return true end
  if (type(t1)~="table") then return false end
  local mt1 = getmetatable(t1)
  local mt2 = getmetatable(t2)
  if( not recursive_compare(mt1,mt2) ) then return false end

  for k1,v1 in pairs(t1) do
    local v2 = t2 and t2[k1] or nil
    if( not recursive_compare(v1,v2) ) then return false end
  end
  for k2,v2 in pairs(t2) do
    local v1 = t1 and t1[k2] or nil
    if( not recursive_compare(v1,v2) ) then return false end
  end

  return true
end
return
{
  printf = sen_log,
  meta_merge = meta_merge,
  ripairs = ripairs,
  fps = C.sen_fps,
  auto_table = newAutotable,
  bbox = bbox,
  input_touch = to_touchp,
  input_scroll = to_scroll,
  dpi = C.sen_platform_dpi(),
  screenSizeName = sen_size_name,
  vp_box = vp_box,
  screen = screen,
  keyCode = keyToCode,
  doExit = C.sen_exit,
  coro_wait = coro_wait,
  platform_name = platform_name,
  rcmp = recursive_compare,
}
