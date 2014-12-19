local sen           = require "sen.core"
local base_cell     = require "game.cell"
local base_board    = require "game.board"
local lvlmanager    = require "game.lvlmanager"
local conf          = require "game.conf"
local scheduler     = sen.Scheduler()
local actionManager = sen.ActionManager
local audioPlayer   = sen.AudioPlayer


require "game.atlabel"
require "game.mlines"

local rs    = (require "resources").str

local camera = sen.camera()
local last_curr_maxn = 5

class "gcell" ("base_cell")

local rand = math.random
local pow = math.pow
local on_done = false
local function get_sprite(image, board)
  local s = sen.clsSprite(nil, image)
  if board then
    s.scale(board.scale, board.scale)
  end
  return s  
end

function gcell:gcell(x, y, col, row, board)
  self:base_cell(x,y,col,row,board)
  self.sprite.setColor(conf.cell_color_bg)
  self.sprite.setColor({a=0})
  
  self.sprites = {}
  self.areaCount = 0
  
  self.state = nil
end


local AREA_EFFECT_RAD = 4


function gcell:update_area_effect(dist,maxf, delay)
  local run_now = not delay
  
  local s = self.sprite
  local a = s.color().a

  if dist and maxf then 
    s.amax = conf.cell_color_bg[4]*(1-dist/maxf) -- self.areaCount > 0 and conf.cell_color_bg[4] or 0
  end
  
  if delay then return end   
   
  
  if math.abs(s.amax - a) > 0.005 then
    s.setColor({a=s.amax})
    --actionManager.stop(s, 'areaFade')
    --actionManager.run(s, conf.effect_fadeTransition(1.5,a>s.amax and 1/rand(4) or (1+rand(4)),s.amax), 'areaFade')
    --actionManager.run(s, conf.effect_fadeIn(1,1/rand(4),alpha), 'areaFade')  
  end  
end

function gcell:do_bounce(cell, dir, speed_rate)
  if (cell and level.is_item(cell.state)) then
     actionManager.run(cell.sprites["item"], conf.effect_dir_bounce(cell, dir,speed_rate))
     actionManager.run(cell.sprites["dir"], conf.effect_dir_bounce(cell, dir,speed_rate))
     cell:side_bounce(cell, dir, speed_rate+1)
  end
end

function gcell:side_bounce(cell, dir, speed_rate)
  local ldir = dir - 1
  local rdir = dir + 1
  if ldir == 0 then ldir = 6 elseif ldir == 7 then ldir = 1 end
  if rdir == 0 then rdir = 6 elseif rdir == 7 then rdir = 1 end
  self:do_bounce(cell:get_neighbor(ldir), ldir, speed_rate)
  self:do_bounce(cell:get_neighbor(rdir), rdir, speed_rate)
  self:do_bounce(cell:get_neighbor(dir), dir, speed_rate)
end


function gcell:setupItem(state, dir, bounce, prev_dir, from)
  local s = get_sprite(conf.image("cell"), self.board)
  local a = get_sprite(conf.image("arrow"), self.board) 
  

  s.setColor(state.color)
  s.moveTo( self.x , self.y )
  s.ZOrder(0.03)

  a.setColor(state.color)
  a.moveTo( self.x, self.y)
  a.ZOrder(0.05)
  a.rotate((2-state.dir)*60)
  a.scale(1.34,1.37,true)
  a.setAnchor(0,10)
  --[[
  a.setAnchor(0,30)
  --]]
  
 -- a.ZOrder(0.5)
    
  if (dir == nil) then
----[[

    actionManager.run(s,conf.effect_fade_in())
    actionManager.run(a,conf.effect_fade_in())
    --]] 
  else
    local speed_rate = bounce and bounce*3 or 2
    local opp_cell
    if from then
      opp_cell = from  
    else
      local opp_dir = (dir+3)%6
      if (opp_dir == 0) then opp_dir = 6 end 
      opp_cell =  self:get_neighbor(opp_dir)
    end

    if (prev_dir ~= nil and prev_dir~=state.dir) then
      actionManager.run(a, conf.effect_direction_change(prev_dir, state.dir, 1, 1/2))
      audioPlayer.playSound("rotate0.mp3")
    end 
          
    actionManager.run(s, conf.effect_slide_to(opp_cell, self, speed_rate ))
    actionManager.run(a, conf.effect_slide_to(opp_cell, self, speed_rate ))
   
    self:side_bounce(opp_cell, dir,speed_rate)    
  -- print(opp_cell.col)
   --rint(opp_cell.row)
   --print(dir)
  end
  
  self.sprites["item"] = s
  self.board.node.addChild(s)
  self.sprites["dir"] = a 
  self.board.node.addChild(a)
end

function gcell:setupPinned(state)
   local s = get_sprite(conf.image("pin"), self.board)
   s.moveTo( self.x , self.y )
   s.scale(1.1,1.1, true)
   s.setColor(1,1,1,0)
   s.ZOrder(0.06)
   --s.blend(3)

   self.sprites["pinned"] = s
   self.board.node.addChild(s)
   actionManager.run(s, conf.effect_blink(1.5,0.2,true))
end

local total_pinned = 0

function gcell:removeFromBoardItems()
   local items = self.board.items
   local found = false
   for _,v in ipairs(items) do
     if v == self then found =true table.remove(items, _) break end
   end
end

function gcell:addToBoardItems()
   local items = self.board.items
   local found = false
   for _,v in ipairs(items) do
     if v == self then found =true break end
   end
   if not found then
     table.insert(items, self)
     self.clear = false
   end  
end

function gcell:setEmpty()
   self:removeSprites()
   self.state = nil
   self:removeFromBoardItems()
end

function gcell:setItem(state, dir, bounce, from)
   if bounce then
      audioPlayer.playSound("impact0.wav")
   end
     
   if (level.is_item(self.state)) then
--     print(self.state.color)
     local c = self:get_neighbor(dir or state.dir)
     local tmp = self.state.pin or self.state.arrow
     self.state.pin = nil 
     self.state.arrow = nil
     
     c:setItem(self.state, dir or state.dir, bounce and bounce+1 or 1)     
     
     if (level.is_pin(tmp)) then
       self:setPin(tmp)
     elseif (level.is_arrow(tmp)) then
       self:setArrow(tmp)
     else
       self:setEmpty()
     end  
  --   self.state.dir = state.dir
     self.state = tmp
    
     --audioPlayer.playSound("impact0.wav")
   else
     
     self.board:adjust_cam(self, dir)
     self.board:update_area_effect()
   end

   if (level.is_empty(self.state)) then
     self:setupItem( state, dir, bounce, nil, from)
     self.state = state

   elseif level.is_pin(self.state) then
     self:setupItem(state, dir, bounce)
     
     local pin = self.state
     self.state = state
     self.state.pin = pin
     if (level.is_pinned(state)) then
       self:setupPinned(state)
       
       if bounce==nil and self.board:is_done() then
         self.board:playDone()
         --self.board:nextLevel(1)
       end
     end
     
   elseif level.is_arrow(self.state) then
     local prev_dir = state.dir 
     state.dir = self.state.dir
   
     self:setupItem(state, dir, bounce, prev_dir)
     
     local arrow = self.state
     self.state = state
     self.state.arrow = arrow
   end  
  
  
   self:addToBoardItems()
    --self:update_area_effect(1)
  if bounce == nil then
    self.board:items_adjust() 
  end 
end

function gcell:setPin(state, init)
   self:removeSprites()
   local s_item = get_sprite(conf.image("pin"), self.board) 
   s_item.moveTo( self.x , self.y )
   s_item.setColor(state.color)
   s_item.ZOrder(0.02)

   self.sprites["pin"] = s_item
   self.board.node.addChild(s_item)
   if (init) then
     s_item.setColor({a=0})
     actionManager.run(s_item,conf.effect_fade_in())
   end
   self.state = state
   --self:update_area_effect(-1)
   self:addToBoardItems()
   
end

function gcell:setArrow(state, init)
   self:removeSprites()
   local s = get_sprite(conf.image("swapper"), self.board) 
   s.moveTo( self.x , self.y )
   s.rotate((2-state.dir)*60)
   s.setColor(conf.swapper_color)
   s.ZOrder(0.02)

   self.sprites["swapper"] = s
   self.board.node.addChild(s)
   if init then
     s.setColor({a=0})
     actionManager.run(s,conf.effect_fadeIn1(conf.swapper_color[4]))
   end
   self.state = state

   self:addToBoardItems()
end

function gcell:updateGameState(map)
  ---[[
  local board = self.board--
  local m = map or board.current_level.map
  local state = m[self.col][self.row]
  if (state ~= self.state) then
    if level.is_empty(state) then
      self:setEmpty()
    elseif level.is_item(state) then
      ----[[ 
      if level.is_pin(state.pin) then
        self:setPin(state.pin, true)
      elseif level.is_arrow(state.arrow) then
        self:setArrow(state.arrow, true)
      end  --]]
      self:setItem(state)
    elseif level.is_pin(state) then
      self:setPin(state, true)
    elseif level.is_arrow(state) then
      self:setArrow(state, true)
    end
  --   self:update_area_effect()
  end 
  --]] 
end

function gcell:on_clean()
  self:removeSprites()
  self:removeSprites("@touch")
  actionManager.stop(self.sprite, 'areaFade')
  
  --self:removeSprites("_effect_matrix")
end

local cam_lock = false
local input_lock = false
--!!!
local need_adjusting = false
local need_trimming  = false
local need_refilling  = false

local function move_camera(x,y,on_end,speed,lock,rate)
  if cam_lock then return end
  
  actionManager.stop(camera)
  actionManager.run(camera,
  {
    name = "speed",
    speed = speed or 2, 
    action = {
      name = "seq",
      actions = {
        {
          name = "interval",
          duration = 1,
          x = x,
          y = y,
          rate = rate or 1/2,
          trigger = function(self, dt, conf)
            self.moveTo(conf.startX+conf.deltaX*math.pow(dt, conf.rate), 
                        conf.startY+conf.deltaY*math.pow(dt, conf.rate))
          end,
          start_trigger = function(self, conf)
            conf.startX = camera.posX()
            conf.startY = camera.posY()
            conf.deltaX = (-conf.x - camera.posX())
            conf.deltaY = (-conf.y - camera.posY())
          end
        },
--[[        {
          name = "interval",
          duration = 1,
          x = x,
          y = y,
          rate = 2,
          trigger = function(self, dt, conf)
            self.moveTo(conf.startX+conf.deltaX*math.pow(dt, conf.rate), 
                        conf.startY+conf.deltaY*math.pow(dt, conf.rate))
          end,
          start_trigger = function(self, conf)
            conf.startX = camera.posX()
            conf.startY = camera.posY()
            conf.deltaX = (-conf.x - camera.posX())
            conf.deltaY = (-conf.y - camera.posY())
          end
        },==]]
      },
    },
    end_trigger = function()
      if (on_end) then  on_end() end
      --need_adjusting = false
      
      cam_lock = false
    end, 
    start_trigger = function()
      cam_lock = lock or false
    end
  
  });
end


local touched_item = nil
local TAIL_MAX = 8

function gcell:setupTouchEffectTail()
  if touched_item == nil then return end
  local state = self.state
  if level.is_item(state) then
    local max = 1
    local c = self:get_neighbor(state.dir)
    while c and 
          not level.is_item(c.state) and 
          not level.is_arrow(c.state) and 
          max < TAIL_MAX do
      c:setupTouchEffect(max)    
      c = c:get_neighbor(state.dir)
      max = max + 1
    end
    ----[[
    if (c and max < TAIL_MAX) then
      c:setupTouchEffect(max,self.state)    
    end
    --]]
  end
end

local TOUCH_MAX_ALPHA = 0.15


function gcell:setupTouchEffect(dist, st)
  --if self.sprites["@touch"] ~= nil then return end
  self:removeSprites("@touch")
  local state = self.state
  
  local sel_state = (dist== nil or level.is_item(state))
  
  local s = sel_state and get_sprite(conf.image("selector"), self.board) or
                          get_sprite(conf.image("cellbg"), self.board) 
  s.moveTo( self.x , self.y )
  local col = sel_state and state.color or {1,1,1,0}
  local distance = dist or 1
  s.amax = sel_state and 0.3 or TOUCH_MAX_ALPHA/distance
  
  local scale = level.is_item(state) and 1.1 or 0.95
  s.setColor(col)
  s.setColor({a=0})
  s.scale(scale,scale, true)
  self.sprites["@touch"] = s
  self.board.node.addChild(s)
  
  
  local f = nil
  if dist== nil then
    f= function() self:setupTouchEffectTail() end 
  end  
 
  actionManager.run(s,
    conf.effect_fadeIn(1,1/2,s.amax,f), 'touchedItem')
end

function gcell:touchEffectIn()
  if touched_item ~= nil then return end 
  self:setupTouchEffect()
--  self:setupTouchEffectTail()
   
   touched_item = self
end

function gcell:on_touch_begin()
  local state = self.state
  
  if (state and state.locked) then
    return
  end
  
  if level.is_item(state) then
    self:touchEffectIn()
  end
  
end

function gcell:removeTouchEffect()
  local s = self.sprites["@touch"]
  if s and not actionManager.is_running(s, 'fadeOut') then
    s.parent = self.board.node
    actionManager.run(s,conf.effect_fadeOut(4,1/4,s.color().a, 
      function()
        self:removeSprites("@touch")
      end
    ))
  end
end

function gcell:touchEffectOut()

  self:removeTouchEffect()
    
  local state = self.state
  if level.is_item(state) then
    local max = 1
    local c = self:get_neighbor(state.dir)
    while c and 
          not level.is_item(c.state) and 
          not level.is_arrow(c.state) and 
          max < TAIL_MAX 
    do
      c:removeTouchEffect()
      c = c:get_neighbor(state.dir)
      max = max + 1
    end
    if (c and max < TAIL_MAX) then
      c:removeTouchEffect()
    end
    
  end
  
end

function gcell:is_outscreen()
  local board = self.board
  --[[
  if (self.y - board.hcell*0.5 < board.scene_bbox.b+board.bLines[1].height) then
    print (board.bLines[1].height)
    print(self.y - board.hcell*0.5 , board.scene_bbox.b+board.bLines[1].height)
  end 
  --]] 
  
  return 
     not board:cell_full_visible(self, 0.1) or
         self.y - board.hcell*0.5 < board.scene_bbox.b+board.bLines[1].height
end

function gcell:is_screen_blocked(to)
  local state = self.state
  if (state and state.locked) then
    return nil
  end
  

  if level.is_item(state) then

    local c2
    if to then
      c2 = to
    else
      c2 = self  
    end
    
    local prev = c2
    while c2 and level.is_item(c2.state) do
      prev = c2
      c2 = c2:get_neighbor(state.dir)
    end
    
    local target = c2 or prev
    
   -- print(self.board.scene_bbox.l, self.board.scene_bbox.t)
  --  local bb = self.board.scene_bbox
--    local nn = self.board.node
    local board = self.board
    --print(target.y+nn.posY(), bb.b)
    if target:is_outscreen() then
    --if not board:cell_full_visible(target, 0.1) or
      -- target.y - board.hcell*0.5 < bb.b+board.bLines[1].height
     --then
      --return prev
      --last_curr_maxn = board.curr_maxn
      board:rescale(false, 1.2)
      return nil
    --elseif target.x+nn.posX() < bb.l+self.board.wcell*0.5 and
      --     target.y+nn.posY() > bb.t-self.board.hcell*0.5 then
      --return prev     
    end
    
  end
  return nil
end

function gcell:show_blocker()
  self:removeSprites("@blocker")
  local s = self.state and 
    get_sprite(conf.image("cellbg"), self.board) or
    get_sprite(conf.image("selector"), self.board)
  s.moveTo( self.x , self.y )
  s.scale(1.1,1.1, true)
  s.setColor({1,0,0,0})
  self.sprites["@blocker"] = s
  self.board.node.addChild(s)
  actionManager.run(s,
    conf.effect_blink(2.5,0.2,false,
    function()
      self:removeSprites("@blocker")
    end
     ))
end

function gcell:on_tap()
  local state = self.state
  if (state and state.locked) then
    return
  end
  
  
----[[
  local block_cell = self:is_screen_blocked()
  if block_cell then
    block_cell:show_blocker()
    --return
  end
--]]
  if level.is_item(state) then
    self:touchEffectOut()    
    local c = self:get_neighbor(state.dir)
    if c==nil then return end
    self.board:updateTurnsQueue()  
    --c.state = self.state
    local tmp = self.state
    if (self.state.arrow ) then
      self:setArrow(self.state.arrow)
      self.state.arrow = nil
      tmp.arrow = nil
    elseif (self.state.pin) then   
      self:setPin(self.state.pin)
      self.state.pin = nil
      tmp.pin = nil
    else
      self:setEmpty()  
      --self:update_area_effect(-1)
    end
    
    c:setItem(tmp, state.dir)
    audioPlayer.playSound("move0.wav")
    
  end
  
end

function gcell:on_swipe(to)
  local state = self.state
  if (state and state.locked) then
    return false
  end
  ----[[
  local block_cell = self:is_screen_blocked(to)
  if block_cell then
    block_cell:show_blocker()
    --return true
  end
  --]]
  if to ~= nil and  level.is_item(state) then
    local c = self
    repeat 
      c = c:get_neighbor(state.dir)
    until c==nil or c==to or level.is_item(c.state)  or level.is_arrow(c.state)
    
    if c ~= to then
      return false
    end        

    self.board:updateTurnsQueue()  
    --c.state = self.state
    local tmp = self.state
    if (self.state.arrow ) then
      self:setArrow(self.state.arrow)
      self.state.arrow = nil
      tmp.arrow = nil
    elseif (self.state.pin) then   
      self:setPin(self.state.pin)
      self.state.pin = nil
      tmp.pin = nil
    else
      self:setEmpty()  
    end  
    c:setItem(tmp, state.dir, nil, self)
    audioPlayer.playSound("move0.wav")
    return true
  end
  return false
end


class "gboard" ("base_board")

function gboard:adjust_cam(cell, dir)
  if cam_lock then 
    return 
  end
  
  if not need_adjusting then
    return 
  end

  local EPSX = self.wcell/2
  local EPSY = self.hcell/2
  local cx, cy
  local mdist = 0
  local c = cell or nil
  if c == nil then
    for _,col in pairs(self.cells) do
      for j,v in pairs(col) do
        if v and level.is_item(v.state) then
          cx = -camera.posX() - v.x   
          cy = -camera.posY() - v.y
          local dist = math.sqrt(cx*cx+cy*cy)
          if mdist < dist then
            mdist = dist
            c = v
          end
        end   
      end
    end
  end    
      
  local bbox = self.scene_bbox

  local xmin =  math.min ( bbox.r - c.x, c.x - bbox.l)
  local ymin =  math.min ( bbox.t - c.y, c.y - bbox.b)

  if xmin < self.wcell/3 or ymin < self.hcell/3 then
    move_camera(c.x,c.y, 
      function() 
        need_refilling = true  
        
      end, 2, false, 1/3)
  end
  
end

local function touches_begin(node, data)
  local board = node.parent

  local td = sen.input_touch(data)

  local cell = board:touchToCell( td )
  if (cell) then
    print(cell.col, cell.row)
    cell:on_touch_begin()
  end  
  
  return 0
end


local function touches_end(node, data)
  if on_done then return end
  local board = node.parent
  local td = sen.input_touch(data)
  
  local dist = math.sqrt ( (td.point.x - td.start.x)^2+ 
                           (td.point.y - td.start.y)^2 )
  local c = board:touchToCell( td )
  
  --if c then
--    board:showLevel( tostring(c.col)..' '..tostring(c.row) )
--  end
    
--  if c== nil then return end
  if touched_item ~= nil then
    touched_item:touchEffectOut()
    local ret = false
    if not actionManager.is_running(touched_item,'touchedItem') then
      
      ret = touched_item:on_swipe(c)
    end
    touched_item=nil
    if ret then board:update_area_effect() return end
  end  
                             
  if board:is_tap(td)-- td.point.z < 0.4 and dist < board.wcell and c.state  then
  then
     board:tap(td)  
  elseif board:is_swipe(td) then
    --local dir = swipe2dir(td)
    local cell = board:touchToCell( td, true )
    if cell == nil then return end
    if not level.is_item(cell.state) then
      local cell2 = board:touchToCell( td )
      if cell2 ~= nil --and not level.is_item(cell2.state) 
      then 
          
        if  board:swipe_range(td,230,310)  then
          --board:nextLevel(0)
          board:bClick(2)
        elseif board:is_swipe_right(td) then
          --[[
          board.bLines[3]
          move_camera((-board.scene_bbox.r+board.scene_bbox.l),0,
          function()
            camera.moveTo(0,0)            
            scenes.lvlsScene.setCurrent()
          end
          ,2,true,2)
          --]]
          board:bClick(1)
        elseif board:is_swipe_up(td) then
          board:bClick(3)
        end  
      end
    elseif level.is_item(cell.state) then
      local c2 = board:touchToCell( td )
      if c2 == cell then
        board:tap(td)
      end
    --[[
      local d = cell.state.dir
      if swipe_range_dir(td,d,20) and dist > board.wcell/2 then 
        cell:on_swipe()
      else   
        cell:on_tap()
      end--]]  
    end
  end
  
  return 0
end

local function touches_move(node, data)
  local board = node.parent
  local td = sen.input_touch(data)
  
  local cell = board:touchToCell( td )
  
  if cell == nil then return end
  
  local state = cell.state
  
  for i,col in pairs(board.cells) do
    for i,v in pairs(col) do
      local s = v.sprites["@touch"]
      if s and v ~= touched_item and v~=cell then
        actionManager.stop(s, 'blink')
        s.setColor({a=s.amax})
      end  
    end
  end

  local s = cell.sprites["@touch"]
  if s and cell ~= touched_item 
     and not actionManager.is_running(s, 'blink')
  then
    actionManager.run(s, conf.effect_blink(2,math.max(s.amax, TOUCH_MAX_ALPHA)))
  end
  return 0
end

function gboard:undo()
  local q = self.tq
  local rm = sen.auto_table(2)
  if q:is_empty() then return end
  local p = q:pop_right()
  local cells = self.cells
  for i,c in pairs(cells) do
    for j,v in pairs(c) do
      if not level.cmp_state(v.state, p.map[i][j]) then
        if level.is_item(v.state) then
          --v:runMatrixEffect(10,false,1)
          v:setEmpty()
          rm[i][j] = true
        end  
        --v:runMatrixEffect(10,false,2)
      end 
    end
  end  
  
    
  for i,c in pairs(p.map) do
    for j,v in pairs(c) do
      if level.is_item(v) or rm[i][j] then
       -- cells[i][j]:runMatrixEffect(11,false,1)
        if not level.cmp_state(v, cells[i][j].state) then
          cells[i][j]:setEmpty()
          cells[i][j]:updateGameState(p.map)
        end  
      end 
    end
  end  
  
  if q:is_empty() then
    self.bLines[3]:enable(false)
  end
  
  local sdiff = p.maxn - self.curr_maxn
  if sdiff ~= 0 then
    self:rescale(false, sdiff)
  end
  --print "aaaaaaaaaaaaaaaaaa"
end

function gboard:bClick(c)
  local b = self.bLines[c]
  if b then b:doClick() end

  if c == 2 then
    self:nextLevel(0)
  end
  if c == 3 then
    self:undo()
  end     
  if c == 1 then
    for _,v in ipairs(self.bLines) do
      v:doClose(1, _==1)
    end
    move_camera(2*(-self.scene_bbox.r+self.scene_bbox.l),camera.posY(),
    function()
      camera.moveTo(0,0)            
      scenes.lvlsScene.setCurrent()
    end
    ,2,true,2)
  end     
  if c == 4 then
    for _,v in ipairs(self.bLines) do
      v:doClose(1)
    end
    move_camera(2*(-self.scene_bbox.r+self.scene_bbox.l),camera.posY(),
    function()
      camera.moveTo(0,0)            
      scenes.menuScene.setCurrent()
    end
    ,2,true,2)
  end     
end

function gboard:tap(td)
  for _,v in ipairs(self.bLines) do
    if v:checkPoint(td.point.x, td.point.y) then
      --board:click(_)
      self:bClick(_) 
      return
    end
  end  

  local cell = self:touchToCell( td )
  if (cell) then
    cell:on_tap()
  end  
end


function gboard:_update_area_effect(delay)
  local bbox = self.scene_bbox
  local maxf = math.min( (bbox.r - bbox.l)/2, (bbox.t - bbox.b)/2)   
  local cells = self.cells
  local items = self.items
  local total = 0
  local invis = 0
  for i,col in pairs(cells) do
    for j,v in pairs(col) do
      total = total + 1
      if self:cell_visible(v) then 
        --local dist = 10000000
        local dist = v:get_dist(cells[0][0])+self.wcell
        if v.state == nil then
          for _,item in ipairs(items) do
            local idist = v:get_dist(item)
            if dist > idist then
              dist = idist
            end
          end
           
        else
          dist = self.wcell-- v:get_neighbor(1):get_dist(v)  
        end
        
        v:update_area_effect(dist, maxf, delay)
      else
        invis = invis + 1  
      end  
    end
  end  
end

local function UpdateAreaOnce(node)
  node.parent:_update_area_effect()
--  scheduler.Remove(node, "update_area_effect")
  return 1
end

function gboard:update_area_effect(delay)
  --self:_update_area_effect()
  --self:_update_area_effect()
  scheduler.Schedule(self.node, UpdateAreaOnce, "update_area_effect",delay or 0.5)
end


function gcell:runMatrixEffect(dir, update, type)
  self:removeSprites("@matrix")
  --if self.sprites["@matrix"] then return end
  local direction = dir or 2
  local s = get_sprite(self.board.cell_image, self.board)  
  s.moveTo( self.x , self.y )
  s.blend(3)
  s.scale(0.97,0.97, true)
  --s.setColor({a=0})
 -- self.sprite.setColor({a=0})
  s.ZOrder(0.3)
  
  --s.cell = self
  --actionManager.run(s, conf.effect_matrix(self.state))
  
  local speed = 0
  if dir ==  2 or dir == 5 then
    speed =(self.board.scene_bbox.t - self.board.scene_bbox.b)/self.board.hcell
  else
    speed =(self.board.scene_bbox.r - self.board.scene_bbox.l)/self.board.wcell
  end
   
  
  s.parent = self
  self.sprites['@matrix'] = s
  actionManager.run(s, conf.effect_Matrix(speed,direction, update, type), 'matrixEffect')
  self.board.node.addChild(s)
end

function gboard:runMatrixEffect(dir, updade, type, max_index)
  
  if  actionManager.is_running(nil,'matrixEffect') then return end
  
  actionManager.stop(nil,'touchedItem')
  touched_item=nil
  for i,column in pairs(self.cells) do
    for j,v in pairs(column) do
      if v.state then
        if not self:cell_full_visible(v) then
          v:setEmpty()
        else
          v.state.locked = true
          v.state.clear = true
        end  
          v:touchEffectOut()
        
      end  
    end 
  end
  
  
  if dir == 2 or dir == 5 then
    local mj = max_index or (dir==2 and self.max_j-1) or (dir==5 and self.min_j+1)
    for i,column in pairs(self.cells) do
      local cell  = column[mj]
      if cell then
        cell:runMatrixEffect(dir, updade, 1)
      end
    end
  elseif dir == 4 or dir == 6 then
    local mi = max_index or (dir==4 and self.min_i+1) or (dir==6 and self.max_i-1)
    local col = self.cells[mi]
    if col then
      for j=self.max_j-1,self.min_j+1,-1 do
        local cell = col[j]
        if cell then
          cell:runMatrixEffect(dir, updade, 1)
        end
      end
    end   
  end    
  
  audioPlayer.playSound("matrix0.mp3")
  self:update_area_effect(2)
end

function gboard:is_done()
   local items = self.items
   

  local pinned = 0   
  for i,v in ipairs(items) do
    if level.is_pinned(v.state) then
        --print(sen.inspect(v.state))
        pinned  = pinned  + 1
    end    
  end  
  --print(self.current_level.npins, pinned)
  return self.current_level.npins == pinned 
end


local run_matrix = false

function gboard:clear()
  actionManager.stop(nil, 'matrixEffect')
  for _,c in pairs(self.cells) do
    for __,r in pairs(c) do
      actionManager.stop(r.sprite, 'areaFade')
      r:removeSprites()
      r:removeSprites("@matrix")
      r:removeSprites("@touch")
      r:removeSprites("@blocker")
      r.sprite.setColor{a=0}
      r:setEmpty()
    end
 end     

end

local yield  = coroutine.yield

local coro_wait = function(dt,secs)
  local m_elapsed = 0
  while (m_elapsed < secs) do
    m_elapsed = m_elapsed + dt
    yield()
  end
end

function gboard:showLevel(s)
                        
  --self.lvlLabel:moveTo(self.scene_bbox.l+10, self.scene_bbox.t-15)
  local sl = tostring(self.lvls.curr)                      
  self.lvlLabel:restart(
  {
    pointerColorIndex=s,
    pointerText = sl,
    Text='LEVEL '..sl,
  })                           

end

local playDoneCoro = function (ref, dt)
  --coro_wait(dt,1)
  local board = ref.parent
  
  for i,column in pairs(board.cells) do
    for j,v in pairs(column) do
      if v.state then
        v.state.locked = true
        v.state.clear = true
      end  
    end 
  end
    ---[[
  local items = board.items
  for _,item in ipairs(items) do
    if level.is_item( item.state ) then 
      local s = get_sprite(conf.image("cell"), board)
      --local s = item.sprites["item"]-- get_sprite(conf.image("cellbg"), board)
      s.moveTo(item.x, item.y)
      ref.addChild(s)
      s.setColor(item.state.color)
      --s.setColor(1,1,1,0)
      s.setColor({a=0})
      s.scale(1.3,1.3,true)
      s.blend(3)
      s.ZOrder(0.06)
      actionManager.run(s, 
      conf.effect_blink(2,0.5,false, function(self)
        self.delete()
      end
      )
      ,'blink_done')
      yield()
    end  
  end
  --  board:trim_indexes()
  --  board:trim_cells()
  --  board:refill_cells()
     
  while actionManager.is_running(nil, 'blink_done') do
    yield()
  end 
--]]
  board:nextLevel(1)
  on_done = false
  return 1
end

function gboard:playDone()
  on_done = true
  
  scheduler.ScheduleCoro(self.node, 
                         sen.clsCoro( playDoneCoro ), 
                        "done_level_coro",0.1)   
  
end


function gboard:updateTurnsQueue()
  local cells = self.cells
  local lvl = sen.auto_table(2)
  for i,c in pairs(cells) do
    for j,v in pairs(c) do 
      if v.state  then
        lvl[i][j] = level.copy_state(v.state)
      end  
    end
  end
  
  local data = {}
  data.map = lvl
  
  --[[
  
  local tq = self.tq
  local f = false
  local ref = nil
  for i in tq:iter_right() do
    f = true
    for _,c in pairs(i.map) do 
      for __,v in pairs(c) do
        if lvl[_] == nil or lvl[_][__] == nil or
           not sen.rcmp(lvl[_][__], v)
        then
          f = false break 
        end 
      end 
      if not f then break end
    end
    
    if f then
      print('----------------------------------------------')
      ref = i
      break
    end 
  end
  
  if ref~=nil then
    if ref.maxn ~= self.curr_maxn then
      self:rescale(false, ref.maxn-self.curr_maxn)
    end
    data.maxn = ref.maxn
    
    last_curr_maxn = self.curr_maxn
  else
    data.maxn = last_curr_maxn   
    last_curr_maxn = self.curr_maxn
  end  
  --]]
  data.maxn = last_curr_maxn   
  last_curr_maxn = self.curr_maxn
  
  self.tq:push_right(data)
  
  self.bLines[3]:enable(true)
  
end

function gboard:items_adjust()
  local tq = self.tq
  local f = false
  local ref = nil
  for i in tq:iter_right() do
    local map = i.map
    f = true
    for j,item in ipairs(self.items) do
      if map[item.col] == nil or 
         map[item.col][item.row] == nil or 
         not sen.rcmp(map[item.col][item.row], item.state) then
         f = false
         break          
      end    
    end
    if f then
      ref = i
      break 
    end
  end

  if ref and ref.maxn < self.curr_maxn then
    self:rescale(false, -1.2)
  end

end

function gboard:resetTurnsQueue()
  self.tq = nil
  self.tq = sen.clsDeque.new()
  self.bLines[3]:enable(false)
end

function gboard:nextLevel(step)
  --self:clear()
   --self:trim_cells()
  self.items = {}
  camera.moveTo(0,0)
  self.current_level = self.lvls:next(step)
  self:resetTurnsQueue()
  self:rescale(true)
     

  local dir = 2
  local update = true
  local type = step==nil and 1 or step==0 and 2 or step>=1 and 3

  self:showLevel( (step == 0 or step==nil) and 1 or self.lvls:is_top_progress() and 3 or 2 )  


  if (camera.posX() ~= 0 or camera.posY() ~= 0) then
    move_camera(0,0, function() 
--      need_trimming = true
  --    run_matrix = true

     -- self:trim_cells()
     -- self:refill_cells()
     -- need_refilling = false
      self:runMatrixEffect(dir, update, type)
             
    end, 2, true, 2)
  elseif not run_matrix then 
   -- self:trim_indexes()
   -- self:trim_cells()
    self:runMatrixEffect(dir, update, type)  
  end
  self:update_area_effect(2.5)
end

function gboard:_setLevel(tbl)
  self.items = {}
  self:clear()
  self.current_level = tbl
  self:resetTurnsQueue()
  self:rescale(true)
  self:showLevel(self.lvls:is_top_progress() and 3 or 2)  
  local dir = 4
  local update = true
  local type = 1

  if (camera.posX() ~= 0 or camera.posY() ~= 0) then
    move_camera(0,0, function() 

      --self:trim_cells()
     -- self:refill_cells()
     -- need_refilling = false
      self:runMatrixEffect(dir, update, type)
             
    end, 2, true, 2)
  else 
  -- self:trim_indexes()
   --self:trim_cells()
   --self:refill_cells()
    --need_refilling = false 
    
    self:runMatrixEffect(dir, update, type)  
  end
  self:update_area_effect(1.5)
end


function gboard:setLevel(lvl)
  self:_setLevel(self.lvls:set(lvl))
end

local last_bbox = {}

function gboard:items_bbox()
  local maxx, maxy, minx, miny = -10000000, -10000000, 1000000, 1000000
  for _,v in ipairs(self.items) do
    maxx = math.max(maxx, v.x) 
    maxy = math.max(maxy, v.y) 
    minx = math.min(minx, v.x) 
    miny = math.min(miny, v.y) 
  end 
  return {l=minx, b=miny, t=maxy, r=maxx}
end

function gboard:resize_rescale(y)
  local f = false
  --local bb = self.scene_bbox
  for _,v in ipairs(self.items) do 
     if v:is_outscreen() then
       f = true 
       break
     end
  end
  return f and 1.2 or 0
end

function gboard:rescale(trim, to)
  --self.wcell = self.wcell / self.node.scaleX() 
  --self.hcell = self.hcell / self.node.scaleY()
  local sb = self.scene.getBBox()
  self.lvlLabel:moveTo(sb.l+10, sb.t-15)
  
  local n = self.node
  if self.current_level==nil then return end
  
  camera.moveTo(0,0)
--  print(self.current_level.scale, self.node.scaleX(), self.node.scaleY())

   
  --self.scene_bbox.l = self.scene_bbox.l* self.node.scaleX()
  --self.scene_bbox.r = self.scene_bbox.r* self.node.scaleX()
  --self.scene_bbox.t = self.scene_bbox.t* self.node.scaleY()
  --self.scene_bbox.b = self.scene_bbox.b* self.node.scaleY()
  
  
  --print(self.lvlLabel.node.posX(),self.lvlLabel.node.posY() )
  --[[
  local x,y = self.current_level.sx*self.wcell,
              self.current_level.sy*self.hcell--]]
   local x,y
--  if #self.items > 0 then              
--  local ibb = self:items_bbox()              
  -- x,y = (ibb.r-ibb.l) / 2,(ibb.t-ibb.b) / 2
  --else
   x,y = self.current_level.sx*self.wcell,
              self.current_level.sy*self.hcell
              --]]
  --end            
    

  
  last_curr_maxn = trim and self.current_level.maxn or self.curr_maxn 

  self.curr_maxn =  trim and self.current_level.maxn or
                    self.curr_maxn + (to or 0) 
   
  local adj = 0
  local sc               
  local sx,sy 
  local i = 0
--  repeat
     self.curr_maxn = self.curr_maxn + adj 
     sc = 7 / (self.curr_maxn)               
     sx,sy = sc, sc 
  
  
    local b = self.scene_bbox
     b.b = (sb.b - y) / sy
     b.t = (sb.t - y) / sy
     b.r = (sb.r - x) / sx
     b.l = (sb.l - x) / sx
     
  --   if to then break end
     
    -- if adj ~= 0 then break end
     
     --adj = self:resize_rescale()
   --  if adj < 0 and self.curr_maxn <= 5 then
  --     break
  --   end 
     --if adj == 0 then break end
     
  --until false   
   
                
  local ef = function()
    if trim then
      self:trim_indexes()
      self:trim_cells()
    end  
    self:refill_cells()
  end
  
  ef()
  
  actionManager.stop(n, 'board_move')
  actionManager.stop(n, 'board_scale')
  actionManager.run(n, conf.effect_moveTo(x,y,2,1/2), 'board_move')
  actionManager.run(n, conf.effect_scaleTo(sx,sy,2,1/2), 'board_scale')
end

function gboard:gboard(scene)
  last_bbox = scene.getBBox()
  
  local sf = math.min (last_bbox.r - last_bbox.l, last_bbox.t - last_bbox.b ) / (128*7)
  --sen.printf("%f", sf)
  self:base_board(scene, "game_board", gcell, conf.image("cellbg"), 0.755, 1.005, sf)
  last_bbox = scene.getBBox()
  self.node.parent = self
  self.lvls = lvlmanager 
  self.items = {}
  self.current_level = nil 
  self.scene = scene
  
  --self.lnode = sen.clsNode()
  --scene.addChild(self.lnode)
  self.lvlLabel = atLabel(scene)  
  --self.lvlLabel:moveTo(self.scene_bbox.l+10, self.scene_bbox.t-15)
  
  self.bLines = {}
  self.bLines[1] = buttonLine(scene, 
  {LabelTitle=rs.bLevels,
   DescLabelTitle='',
   width_p = 20,
   shift_p = 80,
   wait_click = true,
   pquadColor = conf.cell_colors[2],
    rico="ico_sr"
  })
  
  self.bLines[2] = buttonLine(scene, 
  {LabelTitle=rs.bReset,
   DescLabelTitle='',
   width_p = 40,
   shift_p = 40,
   pquadColor = conf.cell_colors[4],
    rico="ico_sd"
  })

  self.bLines[3] = buttonLine(scene, 
  {LabelTitle=rs.bUndo,
   DescLabelTitle='',
   width_p = 40,
   shift_p = 0,
   pquadColor = conf.cell_colors[3],
    rico="ico_su",
    sound="button0.mp3"
  })

  
  
end

local function key_down(a,b)
  local board = a.parent
  local code = sen.keyCode(b)
  
  
  if code == 0x04 then
    board:bClick(1)
  end
  
  if code==49 then
    board:_setLevel(board.lvls:dev())
  end

  if code == 0x52 then
    board:bClick(4)
  end
end

function gboard:resetButtons()
  local bbox = sen.vp_box()
  local w  = (bbox.t-bbox.b)/16 ---self.hcell*0.5
 --print(vpb,w)
  local bs = bbox.b + w/2 + 2 -- 0-- vpb.b + w
  local i = 0
  for _,v in ipairs(self.bLines) do
    v:moveTo(0,bs)
    v.node.ZOrder(0.96)
    
    v:restart(
      {
        --Top = ts,
        Height = w,
        --DescLabelTitle = _==2 and (rs.bLevel..(curr>0 and curr or progress)) or 
          --               _==3 and (rs.bLevel..( progress)) or
        --''
        --lineColor = {0x2a,0x26,0x2c,0xFF}
        
      }
    )
    --bs = bs + w + 1
    i = i + 1
  end  

end

function gboard:start()
  sen.connect("input", "touchesEnd", touches_end, self.node)
  sen.connect("input", "touchesBegin", touches_begin, self.node)
  sen.connect("input", "touchesMove", touches_move, self.node)
  sen.connect("input", "keyDown", key_down, self.node)
--  camera.moveTo(0,0)
  if (  self.current_level == nil ) then 
     self:setLevel(self.lvls.curr)
  end  
  
  self:resetButtons()
end

function gboard:stop()
  sen.disconnect(self.node, "touchesBegin", "input")
  sen.disconnect(self.node, "touchesEnd", "input")
  sen.disconnect(self.node, "touchesMove", "input")
  sen.disconnect(self.node, "keyDown", "input")
end


function gboard:resize(b)
  self:_resize(b)
  --self.lvlLabel:moveTo(b.l+10, b.t-15)
  --self:resize_rescale()

  self:rescale()
  --self:rescale()--refill_cells()
  self:update_area_effect()

  self:resetButtons()
  --self:nextLevel(0)
  
end

return gboard