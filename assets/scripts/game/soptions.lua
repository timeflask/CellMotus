local sen           = require "sen.core"
local base_cell     = require "game.cell"
local base_board    = require "game.board"
local lvlmanager    = require "game.lvlmanager"
local conf          = require "game.conf"
local scheduler     = sen.Scheduler()
local actionManager = sen.ActionManager
local audioPlayer   = sen.AudioPlayer
local settingsManager = sen.SettingsManager

local rs    = (require "resources").str

local camera = sen.camera()
local scene = nil 
local bbox = {}
local cam_lock = false
require "game.atlabel"
require "game.mlines"
require "game.animbg"

class "ocell" ("base_cell")

local on_click = false
local rand = math.random
local pow = math.pow
local prev_settings =   settingsManager.copy()

local function get_sprite(image, board)
  local s = sen.clsSprite(nil, image)
  if board then
    s.scale(board.scale, board.scale)
    board.node.addChild(s)
  end
  return s  
end

function ocell:ocell(x, y, col, row, board)
  self:base_cell(x,y,col,row,board)
  self.sprite.setColor(conf.cell_color_bg)
  self.sprite.setColor({a=0.0})
  
  self.nodes = {}
end

function ocell:runMatrixEffect(dir, update, col)
  if self.nodes['@matrix'] then return end
  local board = self.board
  local bbox = board.scene_bbox
  local ch = board.hcell

  local direction = dir or 4
  local s = get_sprite(board.cell_image, board)
  s.moveTo( self.x , self.y )
  s.ZOrder(0.1)
  
  local speed = (bbox.t - bbox.b)/ch * 1.5
  s.parent = self
  self.nodes['@matrix'] = s
  actionManager.run(s, 
  
  {
     name = "speed",
     speed = speed,
       
     end_trigger = function (self, conf)
       local cell = self.parent
       if (cell) then
         cell:removeNodes("@matrix")
       end 
     end,           
        
     action = {
         name = "seq",
         actions = {
           {       
             name = "interval",
             duration = rand(),
             rate = 2,
             col = col or conf.cell_color_bg,
             amax = 0.2,
             update = update,
             dir = direction,
             trigger = function(self, dt, conf)
                self.setColor({a=pow(dt, conf.rate)*conf.amax})
             end,
             
             start_trigger = function (self, conf)
               self.setColor(conf.col)
               self.setColor({a=0})
               local cell = self.parent
               conf.amax = conf.col[4]
               if cell and cell.level then
                 conf.amax = 1
               end 
             end,

             end_trigger = function (self, conf)
               local this = self.parent
               if this ~= nil then
                 local dir = (conf.dir == 4 or conf.dir == 3) and conf.dir-(this.col)%2 or
                             (conf.dir == 6 and  conf.dir+(this.col)%2) or
                             conf.dir
                 local cell = this:get_neighbor(dir)
                 if conf.update then
                   --this:setLvlAlpha(1)
--                   this:setEmpty()
  --                 this:updateGameState()
                 end  
                 if (cell ~= nil) then
                   cell:runMatrixEffect(conf.dir, conf.update, conf.col)
                 end
               end  
             end
           },
           {       
              name = "interval",
              duration = 1.5+rand(),
              rate = 1/(2+rand()),
              amax = 0.3,
              trigger = function(self, dt, conf)
                self.setColor({a=(1-pow(dt, conf.rate))*conf.amax})
              end,
              start_trigger = function(self, conf)
               local cell = self.parent
               if cell and cell.state then
                 conf.amax = 1
               end 
              end
           },
         }
       }
  }  
  
  , 'matrixEffect')
 -- self.board.node.addChild(s)
end




class "oboard" ("base_board")


local function touches_begin(node, data)
  if cam_lock then return 1 end
  local board = node.parent
  local td = sen.input_touch(data)
  local cell = board:touchToCell( td )
  --actionManager.stop(camera)
  
  return 0
end

local function move_camera(x,y,speed,rate,on_end)
  
  actionManager.stop(camera)
  actionManager.run(camera,
  {
    name = "speed",
    speed = speed or 2, 
    action = {
      name = "interval",
      duration = 1,
      x = x,
      y = y,
      on_end = on_end,
      rate = rate or 1/2,
      trigger = function(self, dt, conf)
--        self.moveTo(conf.startX+conf.deltaX*pow(dt, conf.rate), 
  --                  conf.startY+conf.deltaY*pow(dt, conf.rate))
        self.moveTo(conf.startX+conf.deltaX*pow(dt, conf.rate), 
                    y)
      end,
      start_trigger = function(self, conf)
        conf.startX = camera.posX()
        conf.startY = camera.posY()
        conf.deltaX = (-conf.x - camera.posX())
        conf.deltaY = (-conf.y - camera.posY())
        cam_lock = true
      end,
      end_trigger = function(self,conf)
        if conf.on_end then
          conf.on_end(self)
        end  
        cam_lock = false
      end
    }}, 'camera')
end

local function touches_end(node, data)
  if on_click or cam_lock then return 1 end
  
  local board = node.parent
  local td = sen.input_touch(data)
  
  --local board = node.parent
--  local td = sen.input_touch(data)
  
  
  if board:is_tap(td) then
    local ty = td.point.y
    for _,v in pairs(board.oLines) do
      local vy = v.node.posY()
      local h2 = (v.node.cfg.Height or 100) / 2
      if (ty<=vy+h2 and ty>=vy-h2) then
        v:doClick()
        break
      end  
    end
    
    for _,v in ipairs(board.bLines) do
      if v:checkPoint(td.point.x, td.point.y) then
        board:click(_) 
        break
      end
    end
    return 1  
  end
  
  
  local bbox = board.scene_bbox

  if board:is_swipe(td) then

    if board:is_swipe_left(td) then
      board:click(3)
      return 1 
    end
    if board:is_swipe_right(td)  then
       board:click(1)
    
      return 1 
    end
      
    --board:runMatrix(6)
    --[[ 
      local is_left = board:is_swipe_left(td)
      for _,v in pairs(board.oLines) do
        v:doClose(is_left and -1 or 1)
      end
      actionManager.stop(camera)
      move_camera(2*(is_left and (bbox.r-bbox.l) or (bbox.l-bbox.r)),camera.posY(),is_left and 1 or 2,6,
        function ()
          camera.moveTo(0,0)
          scenes.menuScene.setCurrent()                
        end
      )
      return 1
    end
    --]]
    
    if board:is_swipe_down(td) then
      board:click(2)
      return 1 
    end
    
  end  
--  board:runMatrix()
  --        camera.moveTo(0,0)
    --      scenes.gameScene.setCurrent()
      --    scenes.gameScene.gboard:setLevel(1)
  --scene.

  
--  board:runMatrix()
          --camera.moveTo(0,0)
          --scenes.gameScene.setCurrent()
          --scenes.gameScene.gboard:setLevel(1)
  --scene.  
  return 1
end

local function touches_move(node, data)
  local board = node.parent
  local td = sen.input_touch(data)
  
  return 0
end

local click_coro = function (node, dt)
  local board = node.board
  local c = board.click_target
  local b = board.bLines[c]
  
  b:doClick()
  scheduler.Wait(b.node)
  
  if c == 2 then
    
    for _,v in pairs(board.oLines) do
      local def = v.node.cfg.setting_def
      local key = v.node.cfg.settings_key
      if settingsManager.get(key, def) ~= def then
        v:doClick()
      end
    end
  end
  
  if c == 1 then
      for _,v in pairs(board.oLines) do
        local def = v.node.cfg.setting_def
        local key = v.node.cfg.settings_key
        if prev_settings[key] == nil then
          prev_settings[key] = def
        end
        if settingsManager.get(key, def) ~= prev_settings[key] then
          v:doClick()
          scheduler.Wait(v.node)
        end
      end
      settingsManager.reset(prev_settings)

      local is_left = true
      for _,v in pairs(board.oLines) do
        v:doClose(-1)
      end
      for _,v in ipairs(board.bLines) do
        v:doClose(1)
      end
      actionManager.stop(camera)
      move_camera(2*(is_left and (bbox.r-bbox.l) or (bbox.l-bbox.r)),camera.posY(),is_left and 1 or 2,6,
        function ()
          camera.moveTo(0,0)
          scenes.menuScene.setCurrent()                
        end
      )
  end  
  
  if c == 3 then
      local is_left = true
      for _,v in pairs(board.oLines) do
        v:doClose(-1)
      end
      for _,v in ipairs(board.bLines) do
        v:doClose(1)
      end
      actionManager.stop(camera)
      move_camera(2*(is_left and (bbox.r-bbox.l) or (bbox.l-bbox.r)),camera.posY(),is_left and 1 or 2,6,
        function ()
          camera.moveTo(0,0)
          scenes.menuScene.setCurrent()                
        end
      )
  end   
  
  actionManager.wait(nil,'camera')
  
  on_click = false
  return 1 
end

function oboard:click(c)
  on_click = true
  self.click_target = c
  scheduler.Remove(scene, 'bclick_coro')
  scheduler.ScheduleCoro(scene, 
                         sen.clsCoro(click_coro), 
                         'bclick_coro',0.1)   
end

function oboard:reset()
  --self:runMatrix(6)
 -- self.titleLabel:restart()
 local ts = bbox.t-self.hcell/6
  self.titleLabel:moveTo(bbox.l+10, ts)
  self.titleLabel:restart(
  {
    pointerColorIndex=3,
    pointerText = '21',
    Text=rs.osTitle..'   ',
    quadColorT = {1,1,1,0},
    labelColorT = {1,1,1,0.5},
    pointerColorT = conf.cell_colors[1],    
  })                           
local scr = sen.screen()
  local w  = (bbox.t-bbox.b)/11  * ( scr.baby and 1.5 or 1 )  --self.hcell*0.7
   ts = ts-24-w/2 --bbox.t - self.hcell
  local i = 0
  for _,v in pairs(self.oLines) do
    v:moveTo(0,ts)
    v:restart(
      {
        --Top = ts,
        Height = w,
        bgColor = {1,1,1,0.05+ (i%2)*0.01},
        
      }
    )
    ts = ts - w - 1
    i = i + 1
  end  
  --[[
  self.aboutLabel:moveTo(self.scene_bbox.l+10, ts)
  self.aboutLabel:restart(
  {
    pointerColorIndex=2,
    pointerText = '21',
    Text=rs.osAboutTitle..'   ',
    quadColorT = {1,1,1,0}
  })                           
--]]
  local lb = self.node.abl1
  lb.moveTo(self.scene_bbox.l+10, ts) -- ts-35
  lb.setColor({a=0})
  actionManager.run(lb, conf.effect_fadeIn(1+rand(),rand()*2,0.4))
  --[[
  lb = self.node.abl2
  lb.moveTo(self.scene_bbox.l+10, ts-45)
  lb.setColor({a=0})
  actionManager.run(lb, conf.effect_fadeIn(1+rand(),rand()*2,0.4))

  lb = self.node.abl3
  lb.moveTo(self.scene_bbox.l+10, ts-55)
  lb.setColor({a=0})
  actionManager.run(lb, conf.effect_fadeIn(1+rand(),rand()*2,0.4))
   --]]
   
  local w  = (bbox.t-bbox.b)/16 * ( scr.baby and 1.3 or 1 ) --self.hcell*0.6
  local bs = bbox.b + w/2+2
  local i = 0
  for _,v in ipairs(self.bLines) do
    v:moveTo(0,bs)
    v:restart(
      {
        --Top = ts,
        Height = w,
        bgColor = {1,1,1,0.05+ (i%2)*0.01},
        
      }
    )
    --bs = bs + w + 1
    i = i + 1
  end  
  
  --self.abg:reset()
end

function oboard:oboard(scene)
  local last_bbox = scene.getBBox()
  local sf = math.min (last_bbox.r - last_bbox.l, last_bbox.t - last_bbox.b ) / (128*6)

  self:base_board(scene, "game_board", ocell, conf.image("cellbg"), 0.755, 1.005, sf)
  self.node.parent = self
  
  self.titleLabel = atLabel(self.node)
  --self.aboutLabel = atLabel(self.node)
  
  local  lb1 = sen.clsLabel(nil, "mecha_s", rs.osAboutL1)
  lb1.setColor({a=0})
  self.node.abl1 = lb1
  self.node.addChild(lb1)
  --[[
  local  lb2 = sen.clsLabel(nil, "mecha_s", rs.osAboutL2)
  lb2.setColor({a=0})
  self.node.abl2 = lb2
  self.node.addChild(lb2)
  local  lb3 = sen.clsLabel(nil, "mecha_s", rs.osAboutL3..sen.platformName())
  lb3.setColor({a=0})
  self.node.abl3 = lb3
  self.node.addChild(lb3)
==]]
  
  self.oLines = {}
  
      
  self.oLines[0] = optLine(scene, 
    {LabelTitle=rs.osSounds, settings_key="sounds_enabled", setting_def=true,
     DescLabelTitle=rs.osSoundsDesc,
    })
    
  self.oLines[1] = optLine(scene, 
    {LabelTitle=rs.osRandomColors, settings_key="random_colors", setting_def=true,
     DescLabelTitle=rs.osRandomColorsDesc,
    })

  self.oLines[2] = optLine(scene, 
  {LabelTitle=rs.osShowTrails, settings_key="trails", setting_def=true,
   DescLabelTitle=rs.osShowTrailsDesc,
  })
  
  self.oLines[3] = optLine(scene, 
  {LabelTitle=rs.osSkipMenus, settings_key="skip_menu", setting_def=false,
   DescLabelTitle=rs.osSkipMenusDesc,
  })
  if string.match(string.lower(sen.platformName()), "android") then 
    self.oLines[4] = optLine(scene, 
    {LabelTitle=rs.osKeepScreenOn, settings_key="keep_screen_on", setting_def=false,
     DescLabelTitle=rs.osKeepScreenOnDesc,
    })
  end  
  
  self.bLines = {}
  self.bLines[1] = buttonLine(scene, 
  {LabelTitle=rs.bCancel,
   DescLabelTitle='',
   width_p = 33.3,
   shift_p = 66.6,
   pquadColor = conf.cell_colors[4],
   rico = "ico_sr"
   
  })
  
  self.bLines[2] = buttonLine(scene, 
  {LabelTitle=rs.bDefault,
   DescLabelTitle='',
   width_p = 33.3,
   shift_p = 33.3,
   pquadColor = conf.cell_colors[2],
   rico = "ico_sd"
  })

  self.bLines[3] = buttonLine(scene, 
  {LabelTitle=rs.bOk,
   DescLabelTitle='',
   width_p = 33.3,
   shift_p = 0,
   pquadColor = conf.cell_colors[3],
    rico = "ico_sl"
   
  })
    
  --self.abg = animBG(self)
end

local function key_down(a,b)
  local board = a.parent
  local code = sen.keyCode(b)
----[[  
  if code == 0x04 or code == 0x52 then
    board:click(3)
  end
  --]]
end

local scroll_accum_x = 0
local scroll_accum_y = 0
local last_time  = os.time()
local function input_scroll(a,b)
  if on_click then return end
  local board = a.parent
  local scroll = sen.input_scroll(b)
  local vb = sen.vp_box()
  
  local diff=os.difftime(os.time(),last_time)
  scroll_accum_x = diff > 0 and 0 or ( scroll_accum_x +  scroll.x )
  scroll_accum_y = diff > 0 and 0 or ( scroll_accum_y +  scroll.y )
  local wf = (vb.r-vb.l)/32
  local hf = (vb.t-vb.b)/32
  if scroll_accum_x < -wf then
    board:click(1)
    scroll_accum_x = 0
    return 1
  end  
  if scroll_accum_x > wf then
    board:click(3)
    scroll_accum_x = 0
    return 1
  end  
  if scroll_accum_y <-hf then
    board:click(2)
    scroll_accum_y = 0
    return 1
  end  
  last_time  = os.time()
  return 0
end

function oboard:start()
  prev_settings =   settingsManager.copy()
  self:reset()
  sen.connect("input", "touchesEnd", touches_end, self.node)
  sen.connect("input", "touchesBegin", touches_begin, self.node)
  sen.connect("input", "touchesMove", touches_move, self.node)
  sen.connect("input", "keyDown", key_down, self.node)
  sen.connect("input", "scroll", input_scroll, self.node)
   scroll_accum_x = 0
  scroll_accum_y = 0 
end

function oboard:stop()
  sen.disconnect(self.node, "touchesBegin", "input")
  sen.disconnect(self.node, "touchesEnd", "input")
  sen.disconnect(self.node, "touchesMove", "input")
  sen.disconnect(self.node, "keyDown", "input")
  sen.disconnect(self.node, "scroll", "input")

  --self.abg:stop()
end

function oboard:resize(b)
 self:_resize(b)
 self:reset()
end

function oboard:runMatrix(dir)

  if  actionManager.is_running(nil,'matrixEffect') then return end
  

  if dir == 2 or dir == 5 then
    local mj = (dir==2 and self.max_j-1) or (dir==5 and self.min_j+1)
    for i,column in pairs(self.cells) do
      local cell  = column[mj]
      if cell then
        cell:runMatrixEffect(dir, updade, 1)
      end
    end
  elseif dir == 4 or dir == 6 then
    local mi = (dir==4 and self.min_i+1) or (dir==6 and self.max_i-1)
    local col = self.cells[mi]
    if col then
      for j=self.max_j-1,self.min_j+1,-1 do
        local cell = col[j]
        if cell then
          cell:runMatrixEffect(dir)
        end
      end
    end   
  end    
  
  
--[[
  for _,c in pairs(self.cells) do
    local cell = c[self.max_j-1]
    if cell then
      cell:runMatrixEffect(2)
    end 
    --c[self.max_j-2]:runMatrixEffect(2)
    --print(sen.inspect(c[0]))  
  end
  --]]
  --print(self.min_i, self.min_j, self.max_i, self.max_j)
end



local function ResizeOnce(a,b)
  bbox = scene.getBBox()
  scene.board:resize(bbox)
  
  return 1
end  

local function onResize(a,b,c,d)
  scheduler.Schedule(scene, ResizeOnce, "menu_scene_resize", 0.1)
  return 0
end  

  

return 
function ()
  scene = sen.clsScene("mmenu")
  scene.board = oboard(scene)
  
  scene.onFG = function ()
   -- print("MENU FG")
    sen.connect("view", "view_change", onResize, scene.board.node)
    bbox = scene.getBBox()
    scene.board:_resize(bbox)
    scene.board:start()
  end
  
  scene.onBG = function ()
   -- print("MENU BG")
    sen.disconnect(scene.board.node, "view_change", "view")
    scene.board:stop()
  end
  
 
  
  scene.setColor( conf.smenu_bg_color )
  
  --scene.connect("resize", onResize, scene)
  return scene
end

