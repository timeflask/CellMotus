local ffi = require "ffi"
local sen = require "sen.core"
local gboard = require "game.gboard"
local scheduler = sen.Scheduler()

local scene = nil 

local function ResizeOnce(a,b)
  local bbox = scene.getBBox()
  scene.gboard:resize( bbox )
  --scheduler.Remove(scene, "game_scene_resize")
  return 1
end  

local function onResize(a,b,c,d)
  --print("FUCK ?")
  scheduler.Schedule(scene, ResizeOnce, "game_scene_resize",0.3)

  return 0
end  

return 
function ()
  scene = sen.clsScene("sgame")
  scene.gboard = gboard(scene)
  
  scene.onFG = function ()
    --print("GAME FG")
    sen.connect("view", "view_change", onResize, scene.gboard.node)
    local bbox = sen.vp_box() -- scene.getBBox()
    scene.gboard:resize(bbox)
    scene.gboard:start()
  --  scene.connect("resize", onResize, scene)
  end
  
  scene.onBG = function ()
    --print("GAME BG")
    sen.disconnect(scene.gboard.node, "view_change", "view")
    scene.gboard:stop()
  end
    
      
  scene.setColor( 0x15, 0x11, 0x17, 0xFF )
  
--  scene.connect("resize", onResize, scene)
  return scene
end

