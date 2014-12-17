package.path = 'assets/scripts/sen/?.lua;' .. package.path

local ffi = require "ffi"
local C   = ffi.C

local sen_utils = require "sen.utils"
local sen_object = require "sen.object"
local sen_node = require "sen.node"
local sen_scene = require "sen.scene"
local sen_sprite = require "sen.sprite"
local sen_label = require "sen.label"
local sen_coro = require "sen.rcoro"
local sen_camera = require "sen.camera"
local sen_quad = require "sen.quad"

require "sen.class"


local sen = { 
  printf         = sen_utils.printf,
  fps            = sen_utils.fps,
  auto_table     = sen_utils.auto_table,
  bbox           = sen_utils.bbox,
  connect        = sen_object.connect,
  input_touch    = sen_utils.input_touch,
  disconnect     = sen_object.disconnect,
  senReload      = C.sen_reload,
  inspect        = require "sen.inspect",
  dpi            = sen_utils.dpi,
  screenSizeName = sen_utils.screenSizeName,
  ripairs        = sen_utils.ripairs,
  vp_box         = sen_utils.vp_box,
  keyCode        = sen_utils.keyCode,
  doExit         = sen_utils.doExit,
  CoroWait       = sen_utils.coro_wait,
  platformName   = sen_utils.platform_name,
  rcmp           = sen_utils.rcmp,
  
  Resmanager      = require "sen.resmanager",
  AudioPlayer     = require "sen.audio",
  Scheduler       = require "sen.scheduler",
  ActionManager   = require "sen.actions",
  SettingsManager = require "sen.settings",
   
  clsNode       = sen_node, 
  clsScene      = sen_scene,
  clsLabel      = sen_label,
  clsSprite     = sen_sprite,
  clsQuad       = sen_quad,
  clsCoro       = sen_coro,
  camera        = sen_camera,
  clsDeque      = require "sen.deque",
  
   
  --clsNode       = ffi.metatype(sen_node.CT, sen_node.META ),
  --clsNodeCast   = sen_node.CAST,
  --clsScene      = ffi.metatype(sen_scene.CT, sen_scene.META ),
  --clsLabel      = ffi.metatype(sen_label.CT, sen_label.META ),
  --clsSprite     = ffi.metatype(sen_sprite.CT, sen_sprite.META ),
  
  --clsTest = sen_scene.clsScene,
   
}

return sen 