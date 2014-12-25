local sen           = require "sen.core"
local base_cell     = require "game.cell"
local base_board    = require "game.board"
local lvlmanager    = require "game.lvlmanager"
local conf          = require "game.conf"
local scheduler     = sen.Scheduler()
local actionManager = sen.ActionManager
local audioPlayer   = sen.AudioPlayer
local settingsManager = sen.SettingsManager
local lvlmanager    = require "game.lvlmanager"

require "game.atlabel"
require "game.mlines"

local rs    = (require "resources").str

local camera = sen.camera()
local cam_lock = false

class "lcell" ("base_cell")
class "lboard" ("base_board")

local scene = nil 
local rand = math.random
local pow = math.pow
local lvls_count = lvlmanager:count()
local progress   = settingsManager.get('progress',1)
local curr       = lvlmanager.curr
local vpb        = sen.vp_box()
function lcell:lcell(x, y, col, row, board)
  self:base_cell(x,y,col,row,board)
  self.sprite.setColor(conf.cell_color_bg)
  self.sprite.setColor({a=0})
  self.nodes = {}
end


function lcell:on_clean()
  self:removeSprites()
  self:removeNodes()
  self:removeNodes('@matrix')
end

local lcell_scale_x = 1.1
local lcell_scale_y = 0.8
local left_x = 0
local right_x = 0

function lcell:setLvlAlpha(a)
  local lvl = self.level
  if lvl == nil then return end
 
  if self.nodes["border"] then
    self.nodes["border"].setColor({a=
      lvl == curr and curr~=progress and conf.lvl_curr_bg[4]*a or
      lvl == progress and conf.lvl_progress_bg[4]*a or
      lvl < progress and conf.lvl_finished_bg[4]*a or
      0.06*a
    
    })
  end
  if self.nodes["bg"] then
    self.nodes["bg"].setColor(
    { a= 
      lvl == curr and curr~=progress and conf.lvl_curr_bg[4]*a or
      lvl == progress and conf.lvl_progress_bg[4]*a or
      lvl < progress and conf.lvl_finished_bg[4]*a or
      conf.lvl_locked_bg[4]*a
    })
    --self.nodes["bg"].setColor({a=1})
  end
  if self.nodes["lvl"] then
    self.nodes["lvl"].setColor(
    { a= 
      lvl < progress and a or 
      lvl==progress and a or 
      lvl>progress and 0.07*a 
    })
  end
  if self.nodes["lh"] then
    self.nodes["lh"].setColor(
    { a= lvl < progress and 0.2*a or 0.06*a 
    })
  end
end

function lcell:setupFlashing()
   if self.nodes["blinkCurrent"] then return end
   local s = sen.clsSprite(nil, conf.image("cellbg"))
   s.moveTo( self.x , self.y )
   s.scale(lcell_scale_x,lcell_scale_y)
   s.setColor(1,1,1,0)
   s.ZOrder(0.03)  
   --s.blend(3)

   self.nodes["blinkCurrent"] = s
   self.board.node.addChild(s)
   actionManager.run(s, conf.effect_blink(2,0.1,true), 'blinkCurrentLevel')
end

function lcell:fadeInLevel(s)
  local lvl = self.level
  if lvl == nil then return end

 
  self.sprite.parent = self 
  actionManager.run(self.sprite, 
  {
    name = "speed",
    speed =  s or 1 , 
    action = {
      name = "interval",
      duration = 1,
      rate = (rand(6)),
      amax = 1,
      trigger = function(self, dt, conf)
        local cell = self.parent
        cell:setLvlAlpha(conf.amax*pow(dt, conf.rate))
      end,
      end_trigger=function(self, conf)
        local cell = self.parent
       -- if cell.nodes["lh"] then
        --  cell.nodes["lh"].setColor({a=0.1})
       -- end  
        if curr > 0 and lvl==curr or lvl == progress and curr < 1 then
          cell:setupFlashing()
        end              
      end,
      start_trigger = function(self, conf)
        local cell = self.parent
        cell:setLvlAlpha(0)
      end      
    },

  }  
  ,'lvlfadeIn')
  
  
end

function lcell:setupLevel(lvl)
  self.level = lvl
  
  ----[[
 
  local s = self.nodes["border"]
  if s == nil then
    s = sen.clsSprite(nil, conf.image("selector"))
    s.ZOrder(-0.01)
    self.nodes["border"] = s
    self.board.node.addChild(s)
  end  
  
  s.moveTo( self.x , self.y )
  s.setColor({1,1,1,0.1})
  s.scale(lcell_scale_x,lcell_scale_y)
  
  local bg = self.nodes["bg"]
  if bg == nil then
    bg = sen.clsSprite(nil, conf.image("cellbg"))
    self.nodes["bg"] = bg
    bg.ZOrder(-0.02)  
    self.board.node.addChild(bg)
  end  
  
  bg.moveTo( self.x , self.y )
  bg.setColor(
    lvl == curr and curr~=progress and conf.lvl_curr_bg or
    lvl == progress and conf.lvl_progress_bg or
    lvl < progress and conf.lvl_finished_bg or
    conf.lvl_locked_bg
  )
  
  bg.scale(lcell_scale_x,lcell_scale_y)


  local lb = self.nodes["lvl"]
  if lb == nil then
    local font_name = "mecha_b"
    if self.board.wcell  < 50 then
      font_name = "mecha_s"
    elseif self.board.wcell < 90 then
      font_name = "mecha_m"
    end  
    --print(font_name)
    lb = sen.clsLabel(nil, font_name, tostring(lvl))
    self.nodes["lvl"] = lb
    lb.ZOrder(0.0)
   -- lb.blend(3)  
    self.board.node.addChild(lb)
  end  

  local lbbox = lb.getBBox()
  lb.moveTo(self.x, self.y)
  lb.move( 
    math.floor(  ( lbbox.left - lbbox.right) / 2.0 )  , 
    -math.floor(  ( lbbox.top - lbbox.bottom) / 2.0 )  
    )
  
  lb.setText(tostring(lvl))
  lb.setColor ( 
    lvl < progress and {1,1,1,1} or 
    lvl==progress and {1,1,1,1} or 
    lvl>progress and {1,1,1,0.07} 
  )  --[[
  local lh = self.nodes["lh"]
  if lh == nil then
    lh = sen.clsSprite(nil, 'lh')
    self.nodes["lh"] = lh
    self.board.node.addChild(lh)
  end  
  
  lh.moveTo( self.x+self.board.wcell*lcell_scale_x/2+7, self.y )
  
  lh.setColor({1,1,1,0.1})
  local b = lh.getBBox()
  local sh = b.r-b.l
  
  lh.scale(self.board.wcell*lcell_scale_x/sh/2.4,0.15)
  lh.setAnchor(-8,0)
  --]]
  local lh = self.nodes["lh"]
  if lh == nil then
    lh = sen.clsQuad()
    self.nodes["lh"] = lh
    self.board.node.addChild(lh)
  end  
  
  lh.moveTo( self.x+self.board.wcell*1.1/2-1, self.y )
  
  lh.setColor({1,1,1,0})
  --lh.setColor(conf.lvl_finished_bg)
  --lh.setColor({a=0})
  
  lh.scale(self.board.wcell*0.44+1,2)
  lh.setAnchor(-0.5,0)
  
  self:setLvlAlpha(0)
end

local scroll_to = 0

function lcell:runMatrixEffect(dir, update, type, col)
  if self.nodes['@matrix'] then self:removeNodes("@matrix")  end
  local bbox = self.board.scene_bbox
  local ch = self.board.hcell
  --[[
  if self.y < scroll_to + bbox.b +ch/2  or self.y > scroll_to + bbox.t-ch/2 then
     local i = self.col
     local j = self.row
     while true do
       local cell = self.board.cells[i][j] 
       if cell == nil then break end
       if cell.level then 
         cell:fadeInLevel()
       end
       i = i + 1
     end     
     return 
  end 
  print("MATRIX", scroll_to)
  --]]
  local direction = dir or 4
  local s = sen.clsSprite(nil, self.board.cell_image)
  s.scale(self.sprite.scaleX(),  self.sprite.scaleY())
  s.moveTo( self.x , self.y )
--  s.blend(3)
  --if self.level then
--    s.scale(0.97*lcell_scale_x,0.97*lcell_scale_y)
  --else
    --s.scale(1,1)
--  end
  --s.ZOrder(0.1)
  
  --s.cell = self
  --actionManager.run(s, conf.effect_matrix(self.state))
  local speed = (self.board.scene_bbox.r - self.board.scene_bbox.l)/self.board.wcell 
  s.parent = self
  self.nodes['@matrix'] = s
  actionManager.run(s, 
  
{
       name = "speed",
       speed = speed*1.5,
       
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
             rate = 2*rand(),
             col = col or {1,1,1,0.2},
             amax = 0.1,
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
                 conf.amax = 0.2
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
                   this:fadeInLevel(2)
--                   this:setEmpty()
  --                 this:updateGameState()
                 end  
                 if (cell ~= nil) then
                   cell:runMatrixEffect(conf.dir, conf.update, conf.color_type, conf.col)
                 end
               end  
             end
           },
           {       
              name = "interval",
              duration = 2*rand(),
              rate = (2*rand()),
              amax = 0.1,
              trigger = function(self, dt, conf)
                self.setColor({a=(1-pow(dt, conf.rate))*conf.amax})
              end,
              start_trigger = function(self, conf)
               local cell = self.parent
               if cell and cell.level then
                 conf.amax = 0.2
               end 
              end
           },
         }
       }
  }  
  
  , 'matrixEffect')
  self.board.node.addChild(s)
end


function lcell:endLine()
  self:removeNodes("lh")
end

----------------------------------------------------------------------------------
local scroll = 0
local scroll_min = 0
local scroll_max = 0
local lshift = 0
local scrLock =false

local function trim_scroll(s)
  --print(scroll_min)
  local trimmed = 0
  local ret = s
  if -s < scroll_min then
    trimmed = 1
    ret = -scroll_min
  elseif -s > scroll_max then
    trimmed = 2
    ret = -scroll_max 
  end  
  
  return ret, trimmed
      
end

local function touches_begin(node, data)

  if not scrLock then
    scene.img_su.setColor {a=0.2}
    scene.img_sd.setColor {a=0.2}
  end    
  
  if scene.on_click then return 1 end
  local board = node.parent

  local td = sen.input_touch(data)

  local cell = board:touchToCell( td )
  if not cam_lock then 
    actionManager.stop(camera)
    scroll =  camera.posY()
  end  
  
  return 0
end


local function move_camera(x,y,speed,rate,on_end)
  if cam_lock then return end
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
        cam_lock = true
        conf.startX = camera.posX()
        conf.startY = camera.posY()
        conf.deltaX = (-conf.x - camera.posX())
        conf.deltaY = (-conf.y - camera.posY())
      end,
      end_trigger = function(self,conf)
        if conf.on_end then
          conf.on_end(self)
        end  
        cam_lock = false
      end
    }}, 'camera')
end

local function LevelChoosenCoro(node, dt)
  actionManager.wait(nil, 'camera')
  actionManager.wait(nil, 'matrixEffect')
  actionManager.wait(nil, 'lvlfadeIn')
  actionManager.stop(nil,'blinkCurrentLevel')
  
  local scene = node
  local board = scene.board
  local self  = board.choosen
  scene.on_click = true
--[[
  for _,v in ipairs(board.bLines) do
    v:doClose(1)
  end
      --]]
  local lvl = self.level
  local bbox = board.scene_bbox
  
  for _,c in pairs(board.cells) do
    for __,v in pairs(c) do
      if v ~= self then
        v:setLvlAlpha(0.2)
        if v.level and (v.level == lvl-1)   then
          local d2 = v.nodes["lh"]
          if d2 then
            d2.setColor({a=0})
          end
           --v:removeNodes ("lh")
        end
      else  
        local d2 = v.nodes["lh"]
        if d2 then
          d2.setColor({a=0})
        end
      end
      local d1 = v.nodes["blinkCurrent"]
      if d1 then
        d1.setColor({a=0})
      end  
        
    end
  end  
  
  board.bbg.setColor({a=0})
  --scene.delChild("scroll_up")
  --scene.delChild("scroll_down")
    
  audioPlayer.playSound("button0.mp3")
  local tar1 = self.nodes["bg"]
  local tar2 = self.nodes["border"]
  
  move_camera(self.x,camera.posY(),1,2,nil)
  --actionManager.wait(nil, 'camera')
  actionManager.run ( nil, 
  {
    name = "speed",
    speed =  1.8, 
    action = {
      name = "interval",
      duration = 1,
      rate =  1/3,
      sx = tar1.scaleX(),
      sy = tar1.scaleY(),
      trigger = function(self, dt, cfg)
        local fx = ( 1+conf.btime(dt)*0.5 )
        local x, y = cfg.sx*fx, cfg.sy*fx  
        tar1.scale(x, y)
        tar2.scale(x, y)
      end,
      start_trigger = function(self, conf)
      end,
      end_trigger = function(self, conf)
        cam_lock = false
        actionManager.stop(camera)
        move_camera(2*(bbox.r-bbox.l),camera.posY(),4,2,
          function ()
            camera.moveTo(0,0)
            scenes.gameScene.setCurrent()
            scenes.gameScene.gboard:setLevel(lvl)
          end
        )
      end
     }
   }, 'scale_choosen')

  actionManager.wait(nil, 'scale_choosen')
  actionManager.wait(nil, 'camera')
  scene.on_click = false
  return 1
end

function lcell:playLevelChoosen()
  scene.on_click = true
  scene.img_su.setColor {a=0}
  scene.img_sd.setColor {a=0}  

  scene.board.choosen = self
  scheduler.Remove(scene, 'lcc')
  scheduler.ScheduleCoro(scene, 
                         sen.clsCoro(LevelChoosenCoro), 
                         'lcc',0.1)   
       --[[                  
  for _,v in ipairs(self.board.bLines) do
    v:doClose(1)
  end  --]]
end

local click_coro = function (node, dt)
  local board = node.board
  local c = board.click_target
  local b = board.bLines[c]
  local bbox = node.board.scene_bbox
  --scene.delChild("scroll_up")
  --scene.delChild("scroll_down")
   
  b:doClick()
  scheduler.Wait(b.node)
  sen.CoroWait(dt,0.2)
  if c == 2 then
      for _,v in ipairs(board.bLines) do
        v:doClose(1)
      end
  
      actionManager.stop(camera)
      move_camera(2*(bbox.r-bbox.l),camera.posY(),2,2,
        function ()
          scene.on_click = false
          camera.moveTo(0,0)
          scenes.gameScene.setCurrent()
          scenes.gameScene.gboard:setLevel(curr>0 and curr or progress)
        end
      )
    
  end
  
  if c == 1 then

      for _,v in ipairs(board.bLines) do
        v:doClose(1)
      end
      actionManager.stop(camera)
      move_camera(2*(bbox.l-bbox.r),camera.posY(),2,2,
        function ()
          scene.on_click = false
          camera.moveTo(0,0)
          scenes.menuScene.setCurrent()
        end
      )
  end  
  
  if c == 3 then
      for _,v in ipairs(board.bLines) do
        v:doClose(1)
      end
      actionManager.stop(camera)
      move_camera(2*(bbox.r-bbox.l),camera.posY(),2,2,
        function ()
          scene.click = false
          camera.moveTo(0,0)
          scenes.gameScene.setCurrent()
          scenes.gameScene.gboard:setLevel(progress)
        end
      )
  end   
  
  actionManager.wait(nil,'camera')
  
  
  return 1 
end


function lboard:doClick(c)
  scene.on_click = true
  --print('----------------------------------------------',scene.on_click)
  scene.img_su.setColor {a=0}
  scene.img_sd.setColor {a=0}  
  self.click_target = c
  scheduler.Remove(self.node, 'bclick_coro')
  scheduler.ScheduleCoro(self.node, 
                         sen.clsCoro(click_coro), 
                         'bclick_coro',0.1)   
end

local function touches_end(node, data)
  if scene.on_click then return 1 end
  local board = node.parent
  local td = sen.input_touch(data)
   
  --[[
  td.point.x = td.point.x - lshift 
  td.start.x = td.start.x - lshift 
  td.prev.x  = td.prev.x - lshift 
  --]]
  
  local dist = math.sqrt ( (td.point.x - td.start.x)^2+ 
                           (td.point.y - td.start.y)^2 )
           
  local bbox = board.scene_bbox
  
  if board:is_tap(td) then
    for _,v in ipairs(board.bLines) do
      if v:checkPoint(td.point.x, td.point.y) then
        board:doClick(_) 
        return 1      
      end
    end
    
    local c = board:touchToCell( td )  
  
    if (c and c.level and not cam_lock) then
      --print(c.level)
      if c.level <= progress then
        c:playLevelChoosen()
        return 1
      end  
    end
  end
  
  if board:is_swipe(td) then
    if board:is_swipe_left(td) then
      board:doClick(2)
      return 1
    end
    
    if board:is_swipe_right(td) then
      board:doClick(1)
      --[[
      board.bLines[1]:doClick()
      for _,v in ipairs(board.bLines) do
        v:doClose(-1)
      end  
      actionManager.stop(camera)
      move_camera(2*(bbox.l-bbox.r),camera.posY(),2,2,
        function ()
          camera.moveTo(0,0)
          scenes.menuScene.setCurrent()
        end
      )
      --]]
      return 1
    end

    local t = td.point.z < 0.05 and 0.05 or td.point.z  
    
    local dy =  (td.point.y-td.start.y)  / t 

    if scrLock==false then
      actionManager.stop(camera)
      actionManager.run(camera,
      {
        name = "speed",
        speed = math.max(4*t, 0.4), 
        action = {
              name = "interval",
              duration = 1,
              x = 0,
              y = dy,
              rate = 1/2,
              trigger = function(self, dt, conf)
                local y =  conf.startY+conf.deltaY*math.pow(dt, conf.rate) 
                self.moveTo(camera.posX(), y)
              end,
              start_trigger = function(self, conf)
                conf.startY = camera.posY()
                local ty = trim_scroll( camera.posY() + conf.y)
                conf.deltaY = ty - camera.posY()
                if conf.y > 0 then
                  scene.img_su.setColor {a=0.0}
                else
                  scene.img_sd.setColor {a=0.0}
                end
              end,
              end_trigger = function(self, conf)
                scroll =  camera.posY()
                if not scrLock then
                  scene.img_su.setColor {a=0.08}
                  scene.img_sd.setColor {a=0.08}
                end   
              end
            }
       })
    else
       scroll =  camera.posY()
    end
    
    return 1
  end
                               
  if not scrLock then
    scene.img_su.setColor {a=0.08}
    scene.img_sd.setColor {a=0.08}
  end   
  
  return 0
end

local function touches_move(node, data)
  if scene.on_click then return 1 end
  local board = node.parent
  local td = sen.input_touch(data)
 -- print( td.point.y - td.start.y )
  
  --sen.printf("%s", sen.inspect(scrLock))
  
  if scrLock==false then
    local s, trimmed = trim_scroll(scroll + td.point.y - td.start.y)
    if trimmed > 0 then
      if td.point.w > board.wcell/2 then board:show_scroll_blocker(trimmed) end
    else  
      camera.moveTo(camera.posX(), scroll + td.point.y - td.start.y)
    end
    
  else 
       --sen.printf("AAAAAAAAAAAAAAAAAAAAAA")
  end
  --camera.moveTo(camera.posX(), s)

  
  return 0
end

function lboard:runScrollMatrix(top)
  local i,j = self:cell_index(0,-scroll)
  local cell = self.cells[i][j]
  if cell and cell.level == nil then
     cell:runMatrixEffect(2, false, 1, {0,1,0,0.01})
  end 
  local c = cell:get_neighbor(4 - cell.col%2)
  while c  and c.x < right_x-lshift do 
    --if c.level == nil then
      c:runMatrixEffect(2, false, 1, {0,1,0,0.01})
   -- end  
    c = c:get_neighbor(4 - c.col%2)
  end
  local c = cell:get_neighbor(6 + cell.col%2)
  while c  and c.x > left_x+lshift-self.wcell do 
    --if c.level == nil then
      c:runMatrixEffect(2, false, 1, {0,1,0,0.01})
   -- end  
    c = c:get_neighbor(6 + c.col%2)
  end
  --print(scroll)
end

function lboard:fadeInLevels()
  local bbox = self.scene_bbox
  local h = (bbox.t-bbox.b)/2  
  local ch = self.hcell
 for _,c in pairs(self.cells) do
   for __,v in pairs(c) do
   ----[[
       if v.y < scroll_to - h -ch/2  or v.y > scroll_to +h+ch/2 then
         if v.level  then
           v:setLvlAlpha(1)-- fadeInLevel()
           if curr > 0 and v.level==curr or v.level == progress and curr < 1 then
             v:setupFlashing()
           end
           v.matrix = nil         
         end
       
--         if v.level then v:fadeInLevel() end
       else
         
       end

   end
 end  

end

function lboard:scrollTo(s)
  --if cam_lock then return end
  local this = self
  actionManager.stop(camera)
  actionManager.run(camera,
  {
    name = "speed",
    speed = 5, 
    action = {
       name = "interval",
       duration = 1,
       x = 0,
       y = s,
       rate = 1/2,
       trigger = function(self, dt, conf)
         self.moveTo(camera.posX(), 
                     conf.startY+conf.deltaY*math.pow(dt, conf.rate))
       end,
       start_trigger = function(self, conf)
          cam_lock = true
          conf.startX = camera.posX()
          conf.startY = camera.posY()
          conf.deltaX =  -camera.posX()
          conf.deltaY = -conf.y - camera.posY()
       end,
       end_trigger = function()
         this:fadeInLevels()
         ----[[
         audioPlayer.playSound("matrix0.mp3")
         for _,c in pairs(this.cells) do
           for __,v in pairs(c) do
             if v.matrix then
               v:runMatrixEffect(v.matrix,true,1)
             end
           end
         end 
         cam_lock = false
         --print ('0000000000000000000000000000000000000') 
         scene.on_click = false
         --]] 
       end
    }}, 'camera')
end

function lboard:show_scroll_blocker(trimmed)
  local running = actionManager.is_running(nil, 'scrollBlocker')
  if running then return end
  local  hr1 = self.hr1-- sen.clsQuad()
  --hr1.parent = self.node
  hr1.setColor(conf.scroll_block_color)
  --hr1.ZOrder(0.98)
  vpb        = sen.vp_box()
  ----[[
  if trimmed < 2 then
    --hr1.moveTo( self.scene_bbox.l,scroll_min - (self.scene_bbox.t-self.scene_bbox.b)/2+4 )
    hr1.moveTo( 0,vpb.b+4)
  else
    --hr1.moveTo( self.scene_bbox.l,scroll_max+ (self.scene_bbox.t-self.scene_bbox.b)/2-4 )
    hr1.moveTo( 0,vpb.t-4)
  end
  --]]
  hr1.scale( 2*(self.scene_bbox.r - self.scene_bbox.l), 4 )
  
  --hr1.moveTo( 0, 0 )
  --hr1.scale( 100, 4 )
  --self.scroll_blocker = hr1
  --self.node.addChild(hr1)
  ----[[
  actionManager.run(hr1,
  {
    name = "speed",
    speed = 1, 
    action = {
      name = "interval",
      duration = 1,
      rate = 1/2,
      astart =  1, 
      trigger = function(self, dt, conf)
        self.setColor( {a= conf.astart*(1-pow(dt, conf.rate)) } )
      end,
      start_trigger = function(self, conf)
        self.setColor({a=conf.astart})
      end,
      end_trigger = function(self, conf)
        --self.parent.delChild(self)
      end
    },
  }  
, "scrollBlocker")
  --]] 
end

function lboard:clear() 
  for _,v in pairs(self.cells) do
    for __,r in pairs(v) do
        r:removeNodes()
        r:removeNodes('@matrix')
        r.matrix = nil
        r.level = nil
       -- r:clean()
       -- r = nil
    end
  end
  
  --actionManager.dump()
  --self.cells = sen.auto_table(2)
end
 
function lboard:fill_levels()
  
    
   progress =settingsManager.get('progress',1)
   curr =lvlmanager.curr
   lvls_count = lvlmanager:count()
     
  local c = self.cells[self.min_i][self.max_j-1]
  
  scroll_max = c.y 
  left_x = c.x
  right_x = c.x
 
  local l2r = self.scene.Prev() ~= scenes.gameScene
  --print(l2r)
  
--  local row_len = 0
--  local row_min, row_max = 0,0 
--  local row_lt = math.ceil ((self.scene_bbox.t - self.scene_bbox.b) / self.hcell / 2)
  
  local ci = c 
  local cc = 1
  local tc = 1
  local cj = c
  local cl = c
  scroll_min = 0
  lshift = self.scene_bbox.l - left_x + self.wcell*0.75
  scroll_to = scroll_max
  while true do
    
    local ni = cc%2 
    if ni > 0 then
      c:setupLevel(tc)
      if curr > 0 then
        if tc == curr then scroll_to = c.y end
      else
        if tc == progress then scroll_to = c.y end
      end
     -- c:fadeInLevel() 
      cj = c
    end
    
    local i,j = base_cell.get_ij(4-ni, c.col, c.row)
    local x,y, visible = self:cell_coords(i,j)
    if self.cells[i][j] == nil then
      if x < self.scene_bbox.r -  lshift - self.wcell/3 then 
      
        c =  self.cell_class( x, y, i, j, self )
        c.sprite.setColor({a=0})
        self.ncells = self.ncells + 1
        self.cells[i][j] = c
                
      else
          
        c = nil  
      end
    else
      
      c = self.cells[i][j]
      --if  i > self.max_i then
      if  x > self.scene_bbox.r -  lshift -  self.wcell/3 then 
        c = nil
          
      end  
    end
    
    if c==nil  then
      cj:endLine()
  --    if (row_len == 0) then 
    --    row_len = tc-1
      --  row_min = math.floor(  )  
      --end
        
      if not l2r then
        --cj:runMatrixEffect(6,true,1)
        cj.matrix = 6
      end
      c = ci:get_neighbor(2)
      if c==nil then
        local i,j = base_cell.get_ij(2, ci.col, ci.row)
        local x,y, visible = self:cell_coords(i,j)
        c = self.cell_class( x, y, i, j, self )
        c.sprite.setColor({a=0})
        self.ncells = self.ncells + 1
        self.cells[i][j] = c
      end
      ci = c
      cc = 1
      scroll_min = c.y - self.hcell/3--*lcell_scale_y
      if l2r then
       -- cl:runMatrixEffect(4,true,1)
       cl.matrix = 4
      end  
      cl = c 
    else
      cc = cc + 1
      if right_x<cj.x then right_x = cj.x end  
    end
    
    tc = tc + ni
    
    if tc > lvls_count then
      if l2r then 
      --  cl:runMatrixEffect(4,true,1)
        cl.matrix = 4
      else
       -- cj:runMatrixEffect(6,true,1)
         cj.matrix = 6
      end  
      cj:endLine()   
      break 
    end 
  end
  
 -- print('--------------------------------------   ', row_len)
  local sh = (self.scene_bbox.t-self.scene_bbox.b)/2
  scrLock = math.abs(scroll_max - scroll_min )+2*self.hcell <=   (self.scene_bbox.t-self.scene_bbox.b)
  scroll_min = scroll_min + sh - self.hcell
  scroll_max = scroll_max - sh + self.hcell

  
  local sh = ( (self.scene_bbox.r-self.scene_bbox.l) - (right_x-left_x) ) / 2
  
 -- print(sh)
  --print(left_x)
  
  
  
  --lshift = sh+self.wcell/2
  --camera.move(0, -scroll_max)
  lshift = math.max(sh,self.wcell*3/2)
  self.node.moveTo(0,0)
  self.node.move(lshift, 0)
  --self.scene_bbox.l - left_x + self.wcell
  --camera.moveTo(0 , 0)
  --scroll_to = trim_scroll(-scroll_to)
  --scene.delChild("scroll_up")
  --scene.delChild("scroll_down")

  local su = scene.img_su
  local sd = scene.img_sd  
  su.moveTo(scene.getBBox().r-8, su.height()/2)
  sd.moveTo(scene.getBBox().r-8, -sd.height()/2)

  cam_lock = true
  if not scrLock then
    if scroll_to > scroll_max then
      scroll_to = scroll_max
    elseif   scroll_to < scroll_min then
      scroll_to = scroll_min
    end
    self:scrollTo(scroll_to)
    
    su.setColor {a=0.08}
    sd.setColor {a=0.08}
    
  else
    su.setColor {a=0}
    sd.setColor {a=0}
    
    self:scrollTo(scroll_max)
    --camera.moveTo(0 , 0)  
  end  
  --camera.move(0, -scroll_to)
  scroll = -scroll_to
             
--[[
actionManager.run(camera, 

  {
    name="interval",
    duration=1,
    forever=true,
    
    trigger = function(self,dt)
    
       print ('CAAAAAAAAAAAAAAM',  camera.posX(), camera.posY()) 
    end
  }
)

--]]
  --self:fadeInLevels()
  --sen.printf("%f %f %f %f %f",scroll_min,scroll_max , scroll_to, (self.scene_bbox.t-self.scene_bbox.b), tonumber(scrLock))
end
     
function lboard:lboard(scene)
  local last_bbox = scene.getBBox()
  local scr = sen.screen()
    
  local sf = math.min (last_bbox.r - last_bbox.l, last_bbox.t - last_bbox.b ) / (128*(scr.baby and 4.5 or 6))
  lcell_scale_x = lcell_scale_x * sf
  lcell_scale_y = lcell_scale_y * sf
  self:base_board(scene, "l_board", lcell, conf.image("cellbg"), 0.7655, 1.005, sf)
  self.node.parent = self
  self.scene = scene
  self.items = {}
  
  self.titleLabel = atLabel(scene)
  
  self.bLines = {}
  self.bLines[1] = buttonLine(scene, 
  {LabelTitle=rs.bMenu,
   DescLabelTitle='',
   width_p = 20,
   shift_p = 80,
   pquadColor = conf.cell_colors[2],
   rico="ico_sr"
  })
  
  self.bLines[2] = buttonLine(scene, 
  {LabelTitle=rs.bContinue,
   DescLabelTitle='',
   width_p = 40,
   shift_p = 40,
   pquadColor = conf.cell_colors[1],
   rico="ico_sl"
  })

  self.bLines[3] = buttonLine(scene, 
  {LabelTitle=rs.bProgress,
   DescLabelTitle='',
   width_p = 40,
   shift_p = 0,
   pquadColor = conf.cell_colors[4],
  })
  
  local bbg = sen.clsQuad("bbg")
  bbg.setColor(conf.slevels_bg_color)
  bbg.setColor({a=0.95})
  bbg.ZOrder(0.96)
  --bbg.blend(4)
  scene.addChild(bbg)
  self.bbg = bbg
  self.bbgh = self.bbg.height()
  
  local  hr1 = sen.clsQuad()
  hr1.setColor({a=0})
  hr1.ZOrder(0.98)
  self.hr1 = hr1
  scene.addChild(hr1)

   local su = sen.clsSprite("scroll_up", "ico_su")
   local sd = sen.clsSprite("scroll_down", "ico_sd")
   su.ZOrder(0.96)
   su.setColor{a=0}
   local f = 20/su.height()
   su.scale(f,f)
    
   sd.ZOrder(0.96)
   sd.setColor{a=0}
   local f = 20/sd.height()
   sd.scale(f,f)
  
   scene.img_su = su
   scene.img_sd = sd
   scene.addChild(su)
   scene.addChild(sd) 
end

function lboard:showTitle()
  local bbox = self.scene_bbox
  local c = self.cells[self.min_i][self.max_j-1]
  self.titleLabel:moveTo(bbox.l+12, c.y+self.hcell-self.hcell/7)
  self.titleLabel:restart(
  {
    pointerColorIndex=2,
    pointerText = '21',
    Text=rs.lvsTitle..'   ',
    quadColorT = {1,1,1,0},
    labelColorT = {1,1,1,0.5},
    pointerColorT = conf.cell_colors[2], 
  }
  )
 vpb        = sen.vp_box()
  local scr = sen.screen()
  local w  = (bbox.t-bbox.b)/16 * ( scr.baby and 1.3 or 1 ) --self.hcell*0.6
 
  --local w  = (bbox.t-bbox.b)/16,32 ---self.hcell*0.5
 --print(vpb,w)
  local bs = vpb.b + w/2 + 2 -- 0-- vpb.b + w
  local i = 0
  for _,v in ipairs(self.bLines) do
    v:moveTo(0,bs)
    v.node.ZOrder(0.97)
    
    v:restart(
      {
        --Top = ts,
        Height = w,
        DescLabelTitle = _==2 and (rs.bLevel..(curr>0 and curr or progress)) or 
                         _==3 and (rs.bLevel..( progress)) or
        ''
        --lineColor = {0x2a,0x26,0x2c,0xFF}
        
      }
    )
    --bs = bs + w + 1
    i = i + 1
  end  

  self.bbg.scale(bbox.r-bbox.l,1/self.bbgh*(w+2))
  self.bbg.moveTo(0,bs-2)
  self.bbg.setColor({a=0.95})
  --scene.delChild("bbg")
end

local last_bbox = nil

function lboard:reset()
--  camera.moveTo(0 , 0)
  --local b = self.scene_bbox
--  if not last_bbox or  ( last_bbox.r-last_bbox.l ~= b.r-b.l or
--        last_bbox.t-last_bbox.b ~= b.t-b.b)  then
    self:clear()
    
    self.node.moveTo(0,0)
    
    self:trim_indexes()
    self:trim_cells()
    self:refill_cells()
    self:fill_levels()
    self:showTitle()
  --else
    --self:scrollTo(-scroll)  
  --end  
  last_bbox = self.scene_bbox
end

local function key_down(a,b)
  local board = a.parent
  local code = sen.keyCode(b)
----[[  
  if code == 0x04 or code == 0x52 then
    board:doClick(1)
  end
  --]]
end

local scroll_accum_x = 0
local scroll_accum_y = 0
local last_time  = os.time()
local function input_scroll(a,b)
  if on_click or actionManager.is_running('camera') then return end
  
  local board = a.parent
  local scroll = sen.input_scroll(b)
  local vb = sen.vp_box()

  local diff=os.difftime(os.time(),last_time)
  scroll_accum_x = diff > 0 and 0 or ( scroll_accum_x +  scroll.x )
  local wf = (vb.r-vb.l)/32
  if scroll_accum_x < -wf then
    board:doClick(1)
    scroll_accum_x = 0
    return 1
  end  
  if scroll_accum_x > wf then
    board:doClick(2)
    scroll_accum_x = 0
    return 1
  end  
  last_time  = os.time()
  
  if scrLock then return end
  
--  actionManager.stop(camera)
  
  local s = -(scroll.y<0 and -1 or 1) * (vb.t-vb.b) / 20 * math.abs(scroll.y)
  --s = s > scroll_max and scroll_max or s>scroll_min and scroll_min or s
  camera.move( 0, s)
  local y = camera.posY()
  y,t = trim_scroll(y)-- y < -scroll_max and -scroll_max or y>-scroll_min and -scroll_min or y
  camera.moveTo( 0, y)
  scroll =  camera.posY()
  if t>0 then
    board:show_scroll_blocker(t)
  end  
  return 0
end

function lboard:start()

--  camera.moveTo(0,0)
  self:reset()
  
  sen.connect("input", "touchesEnd", touches_end, self.node)
  sen.connect("input", "touchesBegin", touches_begin, self.node)
  sen.connect("input", "touchesMove", touches_move, self.node)
  sen.connect("input", "keyDown", key_down, self.node)
  sen.connect("input", "scroll", input_scroll, self.node)
  scroll_accum_x = 0
  scroll_accum_y = 0
  
end

function lboard:stop()
  sen.disconnect(self.node, "touchesBegin", "input")
  sen.disconnect(self.node, "touchesEnd", "input")
  sen.disconnect(self.node, "touchesMove", "input")
  sen.disconnect(self.node, "keyDown", "input")
  sen.disconnect(self.node, "scroll", "input")
  
  self:clear()  
end


function lboard:resize(b)
 -- if  ( last_bbox.r-last_bbox.l ~= b.r-b.l or
  --      last_bbox.t-last_bbox.b ~= b.t-b.b)  then
    self:_resize(b)
   -- self:trim_cells()
    self:reset()
    
 -- end

  --self:_resize(b)
  --self:reset()
  --print( sen.inspect(self) )
  
end

--
--
--
  
----------------------------------------------------------------------------------
local function ResizeOnce(a,b)
  --scheduler.Remove(scene, "levels_scene_resize")
  local bbox = scene.getBBox()
  vpb        = sen.vp_box()
  scene.board:resize(bbox)
  return 1
end  

local function onResize(a,b,c,d)
  scheduler.Schedule(scene, ResizeOnce, "levels_scene_resize", 0.5)
--  sen.connect("view", "resize", ResizeOnce, scene)
  return 0
end  

local function onBoxResize(a,b,c,d)
 -- print "BOX RESIZE"
  
  --local bbox = scene.getBBox()
--  scene.board:_resize(bbox)
  return 0
end  

return 
function ()
  scene = sen.clsScene("levelsChoose")
  scene.board = lboard(scene)
  
  scene.onFG = function ()
--    print("LEVELS FG")
    sen.connect("view", "view_change", onResize, scene.board.node)
    local bbox = scene.getBBox()
    scene.board:_resize(bbox)
    scene.board:start()
    scene.on_click = true
  end
  
  scene.onBG = function ()
--    print("LEVELS BG")
    sen.disconnect(scene.board.node, "view_change", "view")
    scene.board:stop()
  end
  
  
  scene.setColor( conf.slevels_bg_color )
  
 -- scene.connect("resize", onBoxResize, scene)
  
  return scene
end

