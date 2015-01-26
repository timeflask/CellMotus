-- Hello julia test
local sen    = require "sen.core"
GAME_VERSION_MAJOR  = 0
GAME_VERSION_MINOR  = 9
local src = sen.screen()

return {

  atlas_size = 1024,

  fonts = {
    mecha_s = {"mecha.ttf",  src.fonts.small},
    --mecha_sm = {"mecha.ttf", 15 + src.font_factor}, 
    mecha_m = {"mecha.ttf",  src.fonts.medium},
    mecha_b = {"mecha.ttf",  src.fonts.big},
    
  }, 
  
  images = {
    cellbg128       = "cell128bg.png", 
                                       
                                         
                                       
                                       
    cell128         = "cell128.png",
    swapper128      = "swapper128.png",
    pin128          = "pin128.png",
    arrow128        = "arrow128.png",
    selector128     = "selector128.png",
    cell11128     = "cell11.png",
    arrowB128     = "arrowB128.png",

    ico_sl = "ico_sl.png",
    ico_su = "ico_su.png",
    ico_sr = "ico_sr.png",
    ico_sd = "ico_sd.png",
    ico_tf = "TF-32x32.png",
  },
  
  shaders = {
  },

  audio = {
    music = {
    },
    
    sounds = {
      impact0 = "impact0.wav",
      move0 = "move0.wav",
      rotate0 = "rotate0.wav",
      button0 = "button0.wav",
      button1 = "button1.wav",
      matrix0 = "matrix0.wav", 
      
    }
  },
  
  str = {
    bOk = "OK",
    bCancel = "CANCEL",
    bDefault = src.baby and "DEFAULT" or "SET DEFAULT",
    bUndo = "UNDO",
    bReset = "RESET",
    bBack = "BACK",
    bContinue = "CONTINUE",
    bProgress = "PROGRESS",
    bLevel = "LEVEL ",
    bMenu = "MENU",
    bLevels = "LEVELS",
  
    lsLoading = "LOADING... %.f%%",
    
    msTitle = "CELL MOTUS",
    msTitle2 = "puzzle game",
    msStart = "START",
    msStart2 = "[choose level] screen",
    msSettings = "SETTINGS",
    msSettings2 = "sounds, colors et cetera",
    msContinue = "CONTINUE",
    msContinue2 = "game from current level",
    msQuit = "QUIT",
    
    lvsTitle = "CHOOSE LEVEL",
    
    osTitle            = "SETTINGS",
    
    osSounds           = "SOUNDS",
    osSoundsDesc       = "ON/OFF ANNOYING SOUNDS",

    osRandomColors     = "RANDOM COLORS",
    osRandomColorsDesc = "shuffle items colors on reset",
    
    osSkipMenus        = "SKIP MENUS",
    osSkipMenusDesc    = "switch to game screen after loading",
    
    osShowTrails        = "SHOW TRAILS",
    osShowTrailsDesc    = "mark possible waypoints",
    
    osKeepScreenOn     = "KEEP SCREEN ON",
    osKeepScreenOnDesc = "...when game on foreground",
    
    osAboutTitle       = "ABOUT",
    osAboutL1          = "CELL MOTUS PUZZLE GAME v."..GAME_VERSION_MAJOR.."."..GAME_VERSION_MINOR, 
    osAboutL2          = "COPYRIGHT [c] 2014-2015 TIME FLASK",
    osAboutL3          = "win, mac, linux, ios, android > time-flask.com",
    
    utf8test = "",
  },
  
}


