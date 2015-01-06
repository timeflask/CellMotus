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
local rs    = (require "resources").str

local camera = sen.camera()
local scene = nil 
local bbox = {}
local wshift = 0
local on_click = false
local lvls_count = lvlmanager:count()
local progress = settingsManager.get('progress',1)
local curr = lvlmanager.curr

require "game.mlines"
require "game.animbg"

class "mcell" ("base_cell")

local rand = math.random
local pow = math.pow

local function get_sprite(image, board, node)
  local s = sen.clsSprite(nil, image)
  if board then
    s.scale(board.scale, board.scale)
    local nd = node or board.mnode 
    nd.addChild(s)
  end
  return s  
end

function mcell:mcell(x, y, col, row, board)
  self:base_cell(x,y,col,row,board)
  self.sprite.setColor(conf.cell_color_bg)
  self.sprite.setColor({a=0.0})
  
  self.nodes = {}
  self.node = sen.clsNode()
  self.node.cell = self
  self.board.node.addChild(self.node)
end


local click_map = {
  function(cell)
    camera.moveTo(0,0)
    scenes.lvlsScene.setCurrent()
  end,
  function(cell)
    camera.moveTo(0,0)
    scenes.gameScene.setCurrent()
    scenes.gameScene.gboard:setLevel(curr>0 and curr or progress)
  end,
  function(cell)
    camera.moveTo(0,0)
    scenes.optsScene.setCurrent()
  end,
  function(cell)
    camera.moveTo(0,0)
    sen.doExit()
  end,
}

class "mboard" ("base_board")

local function touches_begin(node, data)
--  local board = node.parent
 -- local td = sen.input_touch(data)
 -- local cell = board:touchToCell( td )
--  actionManager.stop(camera)
  
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
      end,
      end_trigger = function(self,conf)
        if conf.on_end then
          conf.on_end(self)
        end  
      end
    }}, 'camera')
end

local function bclick_coro(node, dt)
  --audioPlayer.playSound("button0.wav")
  local board = node.board
  local c = board.click_target
  local v = board.bLines[c]
  v:doClick()
  local is_left = c ~= 3
  --v:doClose(-1)
  ----[[
  for _,vv in ipairs(board.bLines) do
    if _~=c then 
      vv:doClose(is_left and -1 or 1, false)
    end  
  end
  scheduler.Wait(v.node)
  v:doClose(is_left and -1 or 1, true)
--]] 
  --click_map[c]()
  --move_camera(-100,0,0.1,2, click_map[c])
  ----[[ 
      move_camera(2*(is_left and (bbox.r-bbox.l) or (bbox.l-bbox.r)),camera.posY(),1,4 ,
        click_map[c]
      )--]]
  actionManager.wait(nil,'camera')      
  on_click = false
  return 1 
end

function mboard:playClick(c)
  on_click = true
  self.click_target = c
  scheduler.Remove(self.mnode, 'bclick_coro')
  scheduler.ScheduleCoro(self.mnode, 
                         sen.clsCoro(bclick_coro), 
                         'bclick_coro',0.1) 
                         
--]]
-- c:click()

end

local function touches_end(node, data)
  if on_click then return end
  local board = node.parent
  local td = sen.input_touch(data)
   --print( board.bStart.nodes['ico'].color().a)
  local c = board:touchToCell( td )
  
  if board:is_tap(td) then          
  --  print(c.button)
    
    local ty = td.point.y
    for _,v in ipairs(board.bLines) do
      local vy = v.node.posY()
      local h2 = v.height / 2
      if (ty<=vy+h2 and ty>=vy-h2) then
        board:playClick(_)
        break
      end  
    end
      
      
--    board:playClick(c)
    --c:click()
    return 1
  end  
  
  --local bbox = board.scene_bbox

  if board:is_swipe(td) then
--    if board:is_swipe_down(td) then
  --    board:runMatrix()
    --end

    if board:is_swipe_left(td) then
      board:playClick(2)
      return 1
    end
    
    if board:is_swipe_right(td) then
      board:playClick(3)
      return 1
    end
----[[
    if board:is_swipe_down(td) then
      board:playClick(1)
      return 1
    end
--]]
--[[
    if board:is_swipe_up(td) then
      board:playClick(1)
      return 1
    end
--]]
  end  
--  board:runMatrix()
  --        camera.moveTo(0,0)
    --      scenes.gameScene.setCurrent()
      --    scenes.gameScene.gboard:setLevel(1)
  --scene.
  return 1
end

local function touches_move(node, data)
  --local board = node.parent
  --local td = sen.input_touch(data)
  
  return 0
end

function mboard:fill_part(i,j,col)
  local c = self.cells
  c[i][j].sprite.setColor( col ) 
  c[i][j+1].sprite.setColor( col ) 
  c[i-1][j].sprite.setColor( col  )
  c[i+1][j].sprite.setColor( col )
end

function mboard:fill_bg(i,j)
  local c = self.cells
  local cc = conf.cell_colors
  
  self:fill_part(i,j,{1,1,1,})
  self:fill_part(i+2,j-1,cc[2])
  self:fill_part(i-2,j-1,cc[3])

  self:fill_part(i,j-3,cc[4])
  
end

function mboard:mboard(scene)
  --local 
  bbox = scene.getBBox()
  local bw = bbox.r - bbox.l
  local bh =  bbox.t - bbox.b
  local sf = math.min (bw, bh ) / (128*6)

  self:base_board(scene, "game_board", mcell, conf.image("cellbg"), 0.765, 1.015, sf)
  self.node.parent = self
  wshift = self.wcell
  
  local mnode = sen.clsNode()
  self.mnode = mnode
  mnode.board = self
  scene.addChild(mnode)
  
  self.titleLabel = atLabel(self.mnode)
  
  self.titleLabel2 = sen.clsLabel(nil, "mecha_s", rs.msTitle2)
  self.titleLabel2.setColor(1,1,1,0.3)
  self.mnode.addChild(self.titleLabel2)
  --self.aboutLabel = atLabel(self.mnode)

  self.bLines = {}
  self.bLines[1] = buttonLine(self.mnode, 
    {LabelTitle=rs.msStart, 
     DescLabelTitle=rs.msStart2,
     pquadColor = conf.cell_colors[2],
     rico="ico_sd",
     sound = "button0.wav"
    })

  self.bLines[2] = buttonLine(self.mnode, 
    {LabelTitle=rs.msContinue, 
     DescLabelTitle=rs.msContinue2,
     pquadColor = conf.cell_colors[3],
     rico="ico_sl",
     sound = "button0.wav"
     
    })
      
  self.bLines[3] = buttonLine(self.mnode, 
    {LabelTitle=rs.msSettings, 
     DescLabelTitle=rs.msSettings2,
     pquadColor = conf.cell_colors[1],
     rico="ico_sr",
     sound = "button0.wav"
     
    })

  self.bLines[4] = buttonLine(self.mnode, 
    {LabelTitle=rs.msQuit, 
     DescLabelTitle=rs.msQuit2,
     pquadColor = conf.cell_colors[4],
     bottom = true,
     --rico="ico_sd",
     sound = "button0.wav"
     
    })
    
  --self:fill_bg(0,2)
  --[[ 
  for _,c in pairs(self.cells) do
    for __, v in pairs(c) do
      v.sprite.setColor({a=0.01}) 
    end
  end 
  --]]    
  --self.cells[0][0].sprite.setColor(conf.cell_colors[1])
  --self.cells[0][0].sprite.setColor({a=0.04})
  
    --[[
  local s = get_sprite("bg0")
  --s.ZOrder(-0.5)
  s.setColor({a=0.01})
  local b = s.getBBox()
  local w = b.r-b.l
  local h = b.t-b.b
  --s.blend(4)
  s.scale(1/w*(bw)/2, 1/h*(bh)*0.7)
  s.moveTo(bbox.r-w/2, -h/6)
  scene.addChild(s)
  --]]  
  --self.abg = animBG(self)--:reset()
  
  --[[
  local s = get_sprite("ico_game")
  s.scale(2.5,2.5)
  s.setColor({a=0.05})
  local b = s.getBBox()
  local w = b.r-b.l
  local h = b.t-b.b
  s.moveTo(bbox.l+85, bbox.t-45)
  scene.addChild(s)
  --]]
end

local function bborder(c)
  for i=1,6 do
    local cc = c:get_neighbor(i)
    if cc then cc.sprite.setColor({a=0.005}) end 
  end
end
 
function mboard:update_area(f)
  local cells = self.cells
  for _,c in pairs(cells) do
    for __, v in pairs(c) do
      local d = v:get_dist(cells[0][0])/(bbox.r-bbox.l)
        v.sprite.setColor({a=d*(f or 0.03)})
    end
  end    
  if f and f > 0 then
    bborder(self.bStart)
    bborder(self.bOptions)
    bborder(self.bContinue)
    bborder(self.bQuit)
  end  
  --self.bg:reset()
end


function mboard:reset()

  bbox =   self.scene_bbox
  local ts =  bbox.t-self.hcell/6
  self.titleLabel:moveTo(bbox.l+10, ts)
  self.titleLabel:restart(
  {
    pointerColorIndex=2,
    pointerText = '21',
    Text=rs.msTitle..'   ',
    quadColorT = {1,1,1,0},
    labelColorT = {1,1,1,0.5},
    pointerColorT = conf.cell_colors[2],
    qw = 26
  })   
  
  ts = ts - 24
  self.titleLabel2.moveTo(bbox.l+19, ts)
  local scr = sen.screen()
  local w  = (bbox.t-bbox.b)/10 * ( scr.baby and 1.5 or 1 ) 
  --local w  = self.hcell/2
  ts = ts - w/2-self.hcell/12
  local bs =  bbox.b + w/2 + 2
  local i = 0
  for _,v in pairs(self.bLines) do
    ts = ts - (v.node.cfg.margin_top or 1)
    local is_bottom = v.node.cfg.bottom
    v:moveTo(0,is_bottom and bs or ts)
    v:restart(
      {
        --Top = ts,
        Height = w,
        bgColor = {1,1,1,0.05+ (i%2)*0.01},
        
      }
    )
    --print(v.node.cfg.margin_top)
    if is_bottom then
      bs = bs + w
    else
      ts = ts - w
    end
    i = i + 1
  end 

 --self.abg:reset()

    
--[[ 
 self.bStart:hide()
 self.bQuit:hide()
 self.bContinue:hide()
 self.bOptions:hide()
 self.bTitle:hide()

  scheduler.Remove(self.mnode, 'reset_coro')
  scheduler.ScheduleCoro(self.mnode, 
                         sen.clsCoro(reset_coro), 
                         'reset_coro',0.1) 
                         
--  self.mnode.rotate(0)
--  self:update_area()
  --self.bg:reset()
  --]]
end

local function key_down(a,b)
  local board = a.parent
  local code = sen.keyCode(b)
----[[  
  if code == 0x04 then
    board:playClick(4)
  end
  if code == 0x52 then
    board:playClick(3)
  end
  --]]
end

local scroll_accum_x = 0
local scroll_accum_y = 0
local last_time  = os.time()
local function input_scroll(a,b)
    if on_click or actionManager.is_running(nil,'camera') then return end
  local board = a.parent
  local scroll = sen.input_scroll(b)
  local vb = sen.vp_box()
  
  local diff=os.difftime(os.time(),last_time)
  scroll_accum_x = diff > 0 and 0 or ( scroll_accum_x +  scroll.x )
  scroll_accum_y = diff > 0 and 0 or ( scroll_accum_y +  scroll.y )
  local wf = (vb.r-vb.l)/32
  local hf = (vb.t-vb.b)/32
  if scroll_accum_x < -wf then
    board:playClick(3)
    scroll_accum_x = 0
    return 1
  end  
  if scroll_accum_x > wf then
    board:playClick(2)
    scroll_accum_x = 0
    return 1
  end  
  if scroll_accum_y < -hf then
    board:playClick(1)
    scroll_accum_y = 0
    return 1
  end  
  last_time  = os.time()
  return 0
end

function mboard:start()
  
  progress = settingsManager.get('progress',1)
  curr = lvlmanager.curr
  lvls_count = lvlmanager:count()
  
  self:reset()

  sen.connect("input", "touchesEnd", touches_end, self.node)
  sen.connect("input", "touchesBegin", touches_begin, self.node)
  sen.connect("input", "touchesMove", touches_move, self.node)
  sen.connect("input", "keyDown", key_down, self.node)
  sen.connect("input", "scroll", input_scroll, self.node)
    scroll_accum_x = 0
  scroll_accum_y = 0
end

function mboard:stop()
  sen.disconnect(self.node, "touchesBegin", "input")
  sen.disconnect(self.node, "touchesEnd", "input")
  sen.disconnect(self.node, "touchesMove", "input")
  sen.disconnect(self.node, "keyDown", "input")
  sen.disconnect(self.node, "scroll", "input")
end

function mboard:resize(b)
 self:_resize(b)
end

function mboard:runSideMatrix()
  for _,c in pairs(self.cells) do
    local cell = c[self.min_j+1]
    if cell and ( cell.col < -1 or cell.col > 1) then
      cell:runMatrixEffect(5)
    end 
    --c[self.max_j-2]:runMatrixEffect(2)
    --print(sen.inspect(c[0]))  
  end
  --self.cells[0][self.max_j-1]:runMatrixEffect(2)
  
  --print(self.min_i, self.min_j, self.max_i, self.max_j)
end

function mboard:runTopMatrix()
  local cells = self.cells
  cells[-1][self.max_j-1]:runMatrixEffect(2)
  cells[0][self.max_j-1]:runMatrixEffect(2)
  cells[1][self.max_j-1]:runMatrixEffect(2)
end

function mboard:runMatrix(dir, speed)
  
  if dir == 2 or dir == 5 then
    local mj = (dir==2 and self.max_j-1) or (dir==5 and self.min_j+1)
    for i,column in pairs(self.cells) do
      local cell  = column[mj]
      if cell then
        cell:runMatrixEffect(dir,speed)
      end
    end
  elseif dir == 4 or dir == 6 then
    local mi = (dir==4 and self.min_i+1) or (dir==6 and self.max_i-1)
    local col = self.cells[mi]
    if col then
      for j=self.max_j-1,self.min_j+1,-1 do
        local cell = col[j]
        if cell then
          cell:runMatrixEffect(dir,speed)
        end
      end
    end   
  end    
end

local function ResizeOnce(a,b)
  bbox = scene.getBBox()
  scene.board:resize(bbox)
  scene.board:reset()
  return 1
end  

local function onResize(a,b,c,d)
  scheduler.Schedule(scene, ResizeOnce, "menu_scene_resize", 0.1)
  return 0
end  

  

return 
function ()
  scene = sen.clsScene("mmenu")
  scene.board = mboard(scene)
  
  
  scene.onFG = function ()
    sen.connect("view", "view_change", onResize, scene.board.node)
    bbox = scene.getBBox()
    scene.board:_resize(bbox)
    scene.board:start()
  --  scene.connect("resize", onResize, scene)
  end
  
  scene.onBG = function ()
    sen.disconnect(scene.board.node, "view_change", "view")
    scene.board:stop()
  end
  
 
  
  scene.setColor( conf.smenu_bg_color )
  --scene.setColor( conf.cell_colors[2] )
  
  
  
  return scene
end

