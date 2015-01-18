local ffi    = require "ffi"
local sen    = require "sen.core"
local res    = require "resources"
local conf          = require "game.conf"

local newMenuScene = require "game.smenu"
local newGameScene = require "game.sgame"
local newLevelsMenu = require "game.slevels"
local newOptionsScene = require "game.soptions"

local format = string.format
local floor  = math.floor
local yield  = coroutine.yield

local scheduler     = sen.Scheduler()
local tmpManager    = sen.Resmanager("temp", 256)
--local tmp2Manager    = sen.Resmanager("temp2", 256)
local resManager    = sen.Resmanager("gameAtlas0", res.atlas_size)
local audioPlayer   = sen.AudioPlayer
local actionManager = sen.ActionManager
local settingsManager = sen.SettingsManager
local scene         = nil
local screen_size = sen.screenSizeName() 
scenes = {
  menuScene = 0,
  optsScene  = 0,
  gameScene = 0,
  lvlsScene = 0,
} 

local lb1       = nil
local ico       = nil
local lb2       = nil
local progress  = 0
local max_objects 

local incProgress = function (i)
  progress = progress + (i or 1)
  lb1.setText(format(res.str.lsLoading, 100.0 * progress / max_objects))
  yield() 
end 

local function fieldsNum(tb)
  local i = 0
  for k,v in pairs(tb) do 
    i = i + 1
  end
  return i   
end



local char_anim = {'\\', '|', '/', '-' }
local c_char_anim = 1
local function lb2_update(self, dt)
  lb2.setText(char_anim[c_char_anim])
  local b = lb1.getBBox()
  lb2.moveTo(-40, 0)
  c_char_anim = c_char_anim + 1
  if (c_char_anim > 4) then c_char_anim = 1 end
  return 0
end

local LOADING_DELAY = 0.05
local loading_func = function (ref, dt)

  -- fonts
  local fonts = res.fonts
  -- images
  local images = res.images
  --shaders
  local shaders = res.shaders
  
  local music = res.audio.music
  local sounds = res.audio.sounds

  max_objects = fieldsNum(fonts) + 
                fieldsNum(images) + 
                fieldsNum(shaders) + 
                fieldsNum(music) + 
                fieldsNum(sounds) + 
                fieldsNum(scenes)  + 2

                
  for k,v in pairs(fonts) do
    resManager.loadFont(k, v[1], v[2], res.alphabet)
    incProgress()                 
  end
   
  for k,v in pairs(images) do
    resManager.loadTexture(k, v)
    incProgress()                 
  end

  resManager.loadLabelShaders()
  incProgress()     
  resManager.loadSpriteShaders()
  incProgress()     

  for k,v in pairs(music) do
    audioPlayer.preloadMusic(v)
    incProgress()                 
  end
  --audioPlayer.playMusic("loop1.wav", 1)

  for k,v in pairs(sounds) do
    audioPlayer.preloadSound(v)
    incProgress()                 
  end

  scenes.menuScene = newMenuScene()
  incProgress()
  scenes.optsScene = newOptionsScene()
  incProgress()
  scenes.gameScene = newGameScene()
  incProgress()
  scenes.lvlsScene = newLevelsMenu()
  incProgress()
  
  if settingsManager.get("skip_menu", false) then
    scenes.gameScene.setCurrent()
--    scenes.gameScene.gboard:setLevel(curr>0 and curr or prog)
  else
    scenes.menuScene.setCurrent()
  end 
  
  
  --scenes.optsScene.setCurrent()
  --scenes.gameScene.setCurrent()
  --scenes.lvlsScene.setCurrent()

  
  scheduler.Remove(scene, "loading_scene_resize")
  scheduler.Remove(scene, "loading_coro")
  scheduler.Remove(scene, "lb2_updater")
  
  lb1.delete()
  lb2.delete()
  tmpManager.collect()
  scene = nil
  --sen.senReload()
end

local function ResizeOnce(a,b)
  scheduler.Remove(scene, "loading_scene_resize")
  return 0
end  

local function onResize(a,b,c,d)
  scheduler.Schedule(scene, ResizeOnce, "loading_scene_resize", 1)
  return 0
end  

return
function()
  local scr = sen.screen()
  --print ("================================================================", 
  --scr.width, scr.height, scr.name, scr.dpi,  scr.width_dp, scr.height_dp, scr.baby)
  
  audioPlayer.setMusicVol(settingsManager.get('music_vol', 0.1))
  audioPlayer.setSoundsVol(settingsManager.get('sounds_vol', 0.3))
 -- audioPlayer.setSoundsVol(1)
  --audioPlayer.playMusic("loop0.wav", 1)
  
  scene = sen.clsScene("sloading")
  scene.setColor(conf.smenu_bg_color)
  sen.connect("view", "resize", onResize, scene)

  
  tmpManager.loadFont("mecha16", "mecha.ttf", 10)
  tmpManager.loadLabelShaders()
  tmpManager.loadSpriteShaders()
  
  lb1 = sen.clsLabel(nil, "mecha16", format(res.str.lsLoading, progress))
  local lbbox = lb1.getBBox()
  lb1.move( floor(  ( lbbox.left - lbbox.right) / 2.0 )  , 0 )

  lb2 = sen.clsLabel(nil, "mecha16", "-")


  scene.addChild(lb1)
  scene.addChild(lb2)
----[[
  scheduler.ScheduleCoro(scene, 
                         sen.clsCoro( loading_func ), 
                         "loading_coro", LOADING_DELAY)
                         
  scheduler.Schedule(scene, lb2_update, "lb2_updater", 0.1)                         
--]]
  scene.setCurrent()
  
  return scene
end, scenes


