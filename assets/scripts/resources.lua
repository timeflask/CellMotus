-- Hello julia test
local sen    = require "sen.core"
local src = sen.screen()
return {
  atlas_size = 512,

  fonts = {
    mecha_s = {"mecha.ttf", 10 + (src.baby and 2 or 0)},
    mecha_sm = {"mecha.ttf", 15 + (src.baby and 2 or 0)}, 
    mecha_m = {"mecha.ttf", 20 + (src.baby and 2 or 0)},
    mecha_b = {"mecha.ttf", 40},
    
  }, 
  
  images = {
    cellbg128       = "cell128bg.png", 
                                       
                                         
                                       
                                       
    cell128         = "cell128.png",
    swapper128      = "swapper128.png",
    pin128          = "pin128.png",
    arrow128        = "arrow128.png",
    selector128     = "selector128.png",

    ico_sl = "ico_sl.png",
    ico_su = "ico_su.png",
    ico_sr = "ico_sr.png",
    ico_sd = "ico_sd.png",
  },
  
  shaders = {
  },

  audio = {
    music = {
    },
    
    sounds = {
      impact0 = "impact0.wav",
      move0 = "move0.wav",
      rotate0 = "rotate0.mp3",
      button0 = "button0.mp3",
      button1 = "button1.mp3",
      matrix0 = "matrix0.mp3", 
      
    }
  },
  
  str = {
    bOk = "OK",
    bCancel = "CANCEL",
    bDefault = "SET DEFAULT",
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
    
    osAboutTitle       = "ABOUT",
    
    osAboutL1          = "CELL MOTUS GAME v.".."0.9", 
    osAboutL2          = "COPYRIGHT [c] TIME FLASK 2014",
    osAboutL3          = "PLATFORM: ",
    
    utf8test = "",
  },
  
}


