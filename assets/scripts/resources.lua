--todo: clear comment chats

return {
  atlas_size = 512,

  fonts = {
    mecha_s = {"mecha.ttf", 10},
    mecha_sm = {"mecha.ttf", 13}, 
    --mecha_ms = {"mecha.ttf", 16}, --d. care dynamic atlas size, dont go mental on fonts 
    mecha_m = {"mecha.ttf", 20},
    mecha_b = {"mecha.ttf", 40},
    
  }, 
  
  images = {
    -- d. todo: jeez, why fucking 128x srsly ? 48x mips  enough for this shit
    -- n. hmmmm no 
    cellbg128       = "cell128bg.png", --m. cuz hexagon is way too complicated shape ?
                                        -- d. let it be! invisible gradients everywhere
                                         
                                       --m. there is no...
                                       
    cell128         = "cell128.png",
    swapper128      = "swapper128.png",
    pin128          = "pin128.png",
    arrow128        = "arrow128.png",
    selector128     = "selector128.png",

    --lh              = "lh.png", --d. rly ? use fucking Quad node, "horizontal line" my ass
                                   -- n.   gradients ?
                                  --d. fuck it, cant see shit anyway
                                  
                                 --m. buy glasses :p
              --[[                     
    ico_triangle    = "ico_triangle.png",
    ico_last    = "ico_last.png",
    ico_settings    = "ico_settings.png",
    ico_quit    = "ico_quit.png",
    ico_start    = "ico_start.png",
    --]]
    --bg0 = "bg0.png",
   ico_sl = "ico_sl.png",
   ico_su = "ico_su.png",
   ico_sr = "ico_sr.png",
   ico_sd = "ico_sd.png",
  },
  
  shaders = {
  },

  audio = {
    music = {
    -- loop0 = "loop0.wav",
     -- loop1 = "loop1.wav", -- d. fuck this samba rumba crap music, 1st thing to disable in options for any sane user (except deafs mb) 
                            --n. cba with music, just remove 
                             -- d. bieber can do bettter. shame
    },
    
    sounds = {
      impact0 = "impact0.wav",
      move0 = "move0.wav",
     -- matrix0 = "matrix0.wav", -- d. god, this is retarded, call it "earsbleed0" 
      rotate0 = "rotate0.mp3",
      button0 = "button0.mp3",
      button1 = "button1.mp3",
      matrix0 = "matrix0.mp3", --m. better ?
      
    --  matrix1 = "matrix1.mp3", -- d. annoying :/
                                  --  m. we need moar samples and randomize it imo

     -- matrix2 = "matrix2.wav", -- n. just no :) 
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
    msSettings2 = "sounds, colors, et cetera",
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
    
    -- n. plz split "about text" to lines for sfx
    
    osAboutL1          = "CELL MOTUS GAME v.".."0.9.132", --d. do manually, gonna add ver auto increment or not
    osAboutL2          = "COPYRIGHT [c] TIME FLASK 2014",
    osAboutL3          = "PLATFORM: ",
    
  },
  
}


