local sen=require "sen.core"
local actionManager = sen.ActionManager
local src = sen.screen()
require "game.atlabel"
require "game.cell"
local conf  = require "game.conf"
local conf_ = conf

local tnode = nil
local tnode_root = nil
local board = nil
local px = 0
local py = 0

local t1_update = function (board)
end

local tutorial_titles = {
  "basics",
  "multitude",
  "practice 1/5",
  "pushing",
  "practice 2/5",
  "drag-and-drop",
  "practice 3/5",
  "arrow",
  "practice 4/5",
  "arrows",
  "practice 5/5"
}

local function findItem(state)
  local items = board.items
  for _,item in ipairs(items) do
      local eq = true
      local st = item.state
      for k,v in pairs(state) do
        if not st[k] or st[k] ~= v then
          eq = false
          break
        end
      end
      if eq then
        return item
      end
  end
  return nil
end

local function get_xy(sx,sy,state)
  local item = findItem(state)
  if item then
    local xx = board._PosX or board.node.posX()
    local yy = board._PosY or board.node.posY()
    local sxx = board._ScaleX or board.node.scaleX()
    local syy = board._ScaleY or board.node.scaleY()
    return (item.x+xx+board.wcell*sx)*sxx,
           (item.y+yy+board.hcell*sy)*syy,
           item.state.color
  end
  return nil
end

local function get_xy2(sx,sy,item)
  if item then
    local xx = board._PosX or board.node.posX()
    local yy = board._PosY or board.node.posY()
    local sxx = board._ScaleX or board.node.scaleX()
    local syy = board._ScaleY or board.node.scaleY()
    return (item.x+xx+board.wcell*sx)*sxx,
           (item.y+yy+board.hcell*sy)*syy,
           (item.state and item.state.color or {1,1,1,0.5})
  end
  return nil
end

local function get_item_xy(item)
  if item then
    local xx = board._PosX or board.node.posX()
    local yy = board._PosY or board.node.posY()
    local sxx = board._ScaleX or board.node.scaleX()
    local syy = board._ScaleY or board.node.scaleY()
    return (item.x)*sxx+xx,
           (item.y)*syy+yy,
           item.state and item.state.color or {1,1,1,0.5}
  end
  return nil
end

local function show_caption(sx,sy,state,text,qcol,qqcol)

  local x,y,col
  if state then
    x,y,col = get_xy(sx,sy,state)
  else
    local b = sen.vp_box()
    y = (b.t-b.b)/2*sy
    x = (b.r-b.l)/2*sx

    --qcol = col
  end


  if not x then return end

  local lb = atLabel(tnode)
  table.insert( tnode.caps, lb )
  lb:moveTo(x,y)
  lb:restart {
    pointerColorIndex=2,
    pointerText = '11',
    Text=text..'   ', --.tutorial_titles[lvlmanager.curr]
    small = true,
    quadColorT = qqcol or {1,1,1,0},
    pointerColorT = qcol or col or {1,1,1,0.8},
    labelColorT = {1,1,1,0.8},
  }

  return x-2, y-lb:sheight()
end


local function show_caption_item(sx,sy,item,text,qcol,qqcol)

  local x,y,col
  
  if item then
    x,y,col = get_xy2(sx,sy,item)
  else
    x = px + sx
    y = py + sy
    col = {1,1,1,0.5}
  end  

  if not x then return end

  local lb = atLabel(tnode)
  table.insert( tnode.caps, lb )
  lb:moveTo(x,y)
  lb:restart {
    pointerColorIndex=2,
    pointerText = '11',
    Text=text..'   ', --.tutorial_titles[lvlmanager.curr]
    small = true,
    quadColorT = qqcol or {1,1,1,0},
    pointerColorT = qcol or col or {1,1,1,0.8},
    labelColorT = {1,1,1,0.8},
  }

  return x-2, y-lb:sheight()
end

local slide_to = function(x1,y1, x2,y2, speed, rate)
   return {
    name = "speed",
    speed = speed or 1,
    action = {
      name = "interval",
      duration = 1,
      deltaX = x2 - x1,
      deltaY = y2 - y1,
      rate = rate or 1/2,
      trigger = function(self, dt, conf)
        local dx = conf_.btime(math.pow(dt, 2))
        self.moveTo(x1+conf.deltaX*dx,
                    y1+conf.deltaY*dx)

      end,
      start_trigger = function(self, conf)
        if self then
          self.moveTo( x1 , y1 )
        end
      end
     }
   }
end


local function show_text(sx,sy,state,text,anim, col)
  local x,y
  if state then
    x,y = get_xy(sx,sy,state)
  else
    x,y = sx,sy
  end
  if not x then return end

  local color = col or {1,1,1,0.5}
  local vb = sen.vp_box()
  for line in text:gmatch("[^\r\n]+") do
    local lb = sen.clsLabel(nil, "mecha_ss", line)
    lb.setColor(color)
    lb.ZOrder(0.52)
    if anim then
      actionManager.run(lb,slide_to(vb.r+20,y,x,y,math.random()*2+1),'animLines')
    else
      lb.moveTo(x,y)
    end
    tnode.addChild(lb)
    local b = lb.getBBox()
    y=y-(b.t-b.b)*1.8
  end

  return x,y
end


local function show_ico(item,dir,text,blink,col)

  local node = tnode.icos
  if node == nil then
    node = sen.clsNode()
    tnode.icos = node
    tnode.addChild(node)
  end

  local x,y,c = get_item_xy(item)
  local color = col or {0,0,0,1}
  local sw = board.wcell
  local sh = board.hcell
  local d = dir or 6
  --local sx = (d==1 or d==6) and -sw*0.7 or sw*0.7
  --local sy = (d==1 or d==3) and -sh*0.55 or sh*0.55
  local sx = 0

  local sy = 0
  if level.is_item(item.state) and item.state.dir == 2 and
     level.is_item(item:get_neighbor(2).state)
  then
    sy = sh*0.505
  else
    sy = -sh*0.505
  end

  local s = sen.clsQuad()
  local lb = sen.clsLabel(nil, "mecha_s", text or "TAP ME")

  s.scale( lb.width()*1.2, lb.height()*2)
  s.setColor(color)
  s.ZOrder(0.45)
  --s.moveTo(x+sx, y+sy)
  node.addChild(s)
  if blink then
    actionManager.run(s, conf.effect_blink(1,0.4,true), 'ico_blink')
  else
    actionManager.run(s,conf.effect_fadeIn1(1),'ico_blink')
  end
  actionManager.run(s,slide_to(x+sx*1.2,y+sy,x+sx,y+sy,math.random()*2+1),'animLines')

  --local b = lb.
  lb.setColor(1,1,1,1)
  lb.moveTo(x-lb.width()/2+sx*1.2, y-lb.height()/2+sy)
  lb.ZOrder(0.46)
  node.addChild(lb)
  actionManager.run(lb,conf.effect_fadeIn1(1),'animLines')

end

local function clear_stage()
  actionManager.stop(nil,'animLines')
  actionManager.stop(nil,'ico_blink')
  for _,c in ipairs(tnode.caps) do
    c:clear()
    c = nil
  end
  tnode.caps = {}
  tnode.clear()
  tnode.icos = nil
end


local DO_NOTHING = 1
local DO_FAIL = 2
local DO_GJ = 3
local bad_items = {}
local function mark_bad_items()
  --clear_stage()
  if tnode.icos then
    tnode.icos.clear()
  end
  for _,v in ipairs(bad_items) do
    show_ico (v,0,"LOST",false,{1,0,0,1})
  end
end

local function tap_mark(i1,i2,i3,i4)
   if i1 then
     if level.is_pinned(i1.state) then
       show_ico(i1,6,"GOOD", false, {0,0.5,0,1})
     else
       show_ico(i1,6,"TAP", true)
     end     
   end  

   if i2 then
     if level.is_pinned(i2.state) then
       show_ico(i2,6,"GOOD", false, {0,0.5,0,1})
     else
       show_ico(i2,6,"TAP", true)
     end     
   end  
   
   if i3 then
     if level.is_pinned(i3.state) then
       show_ico(i3,6,"GOOD", false, {0,0.5,0,1})
     else
       show_ico(i3,6,"TAP", true)
     end     
  end   

  if i4 then
     if level.is_pinned(i4.state) then
       show_ico(i4,6,"GOOD", false, {0,0.5,0,1})
     else
       show_ico(i4,6,"TAP", true)
     end     
  end 
end

  
local tutorial_data = {
  function ()
    -- DO NOTHING
    --print("NOTHING")
  end,

  function ()
    -- FAIL
    local vb = sen.vp_box()
    local sf = (vb.r-vb.l) > 620
    mark_bad_items()
    local x,y,y_bg
    x,y = show_caption(-0.39,sf and -0.35 or -0,nil,"LEVEL FAILED", {1,0,0,1}, {1,0,0,1})
    y_bg = y+g_screen.fonts.height.mecha_b*2
    x,y = show_text(x,y,nil, "Some cells cannot be placed\nonto appropriate circles.",true)

    local ys = y
    x,y = show_text(x,y-10,nil, "board is a total mess ?!\n",true, {1,1,1,1})
    x,y = show_text(x,y,nil, "In that case, you may want to\nRESET the current level.\nTAP [RESET] OR SWIPE DOWN ANYWHERE",true)

    if (vb.r-vb.l) > 620 then
      x = x + (vb.r-vb.l) * 0.35
      y = ys
    end

    x,y = show_text(x,y-10,nil, "I've nade a huge mistake...\n",true, {1,1,1,1})
    x,y = show_text(x,y,nil, "In that case, you may want to\nundo the most recent turns.\nTAP [UNDO] OR SWIPE UP ANYWHERE",true)

    local bg = sen.clsQuad()
    local ybg_end = y
    bg.moveTo(vb.l, ybg_end)
    bg.setAnchor(-0.5,-0.5)
    bg.scale(vb.r-vb.l, y_bg - ybg_end)
    bg.ZOrder(0.49)
    bg.setColor({ 0x15, 0x11, 0x17, 0xDD })
    tnode.addChild(bg)
  end,

  function ()
    local vb = sen.vp_box()
    local sf = (vb.r-vb.l) > 620
    local x,y
    x,y = show_caption_item(0,-32,nil,"WELL DONE", {0,1,0,0}, {0,1,0,0})
  end,

  -- LVL 1 - 1
  [11] = function()
     local x,y
     local item = findItem({type="item"})
     show_ico(item,6,"TAP ME", true)

     x,y=show_caption(0,-0.55,{type="item"},"CELL")
     show_text(x,y,nil,"Tap cell to move it\ntowards the target circle.",true)

     x,y=show_caption_item(0,-32,nil,"DIRECTION",{1,1,1,0.3})
     x,y=show_text(x,y,nil, "Cell moves in one direction\nalong a straight. The direction\nis indicated by an arrow\npointing from the cell.",true)
     --x,y=show_text(x,y,nil, "Cell moves in one direction\nalong a straight.",true)
     --x,y=show_text(x,y,nil, "The direction is indicated by\nan arrow pointing from the cell.",true)


     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460

     x,y=show_caption(sf and -0.55 or 0,sf and 0.75 or -0.17,{type="pin"},"TARGET")
     show_text(x,y,nil, "Place cell on the circle\nof the appropriate color.",true)
  end,
  -- LVL 1 - 2
  [12] = function()
     local x,y
     local item = findItem({type="item"})
     show_ico(item,6,"TAP", true)

     x,y=show_caption(0.0,-0.55,{type="item"},"TAP AGAIN",{1,1,1,0.3})
     x,y=show_text(x,y,nil, "Cells cannot be stopped",true)

     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460

     x,y=show_caption(sf and -0.9 or 0,sf and 0.75 or -0.17,{type="pin"},"TARGET")

     x,y=show_text(x,y,nil, "Target is not moving anywhere",true)
  end,

  -- LVL 1 - 2
  [21] = function(i1,i2)
     local x,y
     show_ico(i1,6,"TAP", true)
     show_ico(i2,6,"TAP", true)

     x,y=show_caption_item(-0.75,-0.6,i2,"number of cells",{1,1,1,0.3})
     x,y=show_text(x,y,nil, "usually, there are certainly\nmultiple cells (2, 3, 4 or\neven more). In order to succeed\nyou must put all of them to\ncorresponding circles. It doesn't\nmatter how you approach this,\nuse any turn sequence.",true)

  end,

  [22] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     if level.is_pinned(i1.state) then
       show_ico(i1,6,"GOOD", false, {0,0.5,0,1})
       x,y=show_caption_item(-0.6,sf and 1.4 or 0.95,i1,"OK. IS IT LOCKED ?",i1.state.color)
       x,y=show_text(x,y,nil, "Answer is NO! Cells are\nnever locked. however, it's\na bad idea to touch this one.",true)
       show_ico(i2,6,"TAP", true)
       x,y=show_caption_item(sf and -1 or -0.4,sf and -0.6 or -0.55,i2,"on the verge of victory",i2.state.color)
       x,y=show_text(x,y,nil, "This one is Definitely the right choice!",true)
     else
       show_ico(i1,6,"TAP", true)
       x,y=show_caption_item(sf and -1 or -0.4,sf and 1.05 or 0.9,i1,"on the verge of victory",i1.state.color)
       x,y=show_text(x,y,nil, "This one is Definitely the right choice!",true)

       show_ico(i2,6,"GOOD", false, {0,0.5,0,1})
       x,y=show_caption_item(sf and -1 or -0.5,sf and -0.6 or -0.55,i2,"OK. IS IT LOCKED ?",i2.state.color)
       x,y=show_text(x,y,nil, "Answer is NO! Cells are\nnever locked. however, it's\na bad idea to touch this one.",true)
     end
  end,
  
  [30] = function(i1,i2,i3)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     show_ico(i3,6,"TAP", true)

     x,y=show_caption_item(0.4,-0.35,i1,"PUSHER",i1.state.color)
     x,y=show_text(x,y,nil, "cell can push the others" ,true)

     x,y=show_caption_item(0.4,-0.35,i2,"PUSH ME",i2.state.color)

     x,y=show_caption_item(0,-32,nil,"About positioning", {1,1,1,0.4})
     x,y=show_text(x,y,nil, 'more often than not,\ncell can not reach\nit\'s target, otherwise\nthe game would be trivial.\nYou have to find a way ...' ,true)
  end,

  [31] = function(i1,i2,i3)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(0.4,-0.35,i1,"PUSHER",i1.state.color)
     x,y=show_text(x,y,nil, "Let's start pushing" ,true)
     x,y=show_caption_item(-0.3,-0.55,i3,"ACCEPTABLE",i3.state.color)
     x,y=show_text(x,y,nil, "This one can wait\nno problems" ,true)
  end,

  [32] = function(i1,i2,i3)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i3,6,"TAP", true)
     x,y=show_caption_item(0.4,-0.35,i1,"PUSHER",i1.state.color)
     x,y=show_text(x,y,nil, "waiting..." ,true)

     x,y=show_caption_item(sf and 0 or 0.4, sf and 0.9 or 0.5,i3,"just one step",i3.state.color)
     x,y=show_text(x,y,nil, "move it to pusher's path" ,true)
  end,

  [33] = function(i1,i2,i3)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(0.4,-0.35,i1,"PUSHER",i1.state.color)
     x,y=show_text(x,y,nil, "Push them all !" ,true)

     x,y=show_caption_item(0,-32,nil,"STACK SIZE",{1,1,1,0.5})
     x,y=show_text(x,y,nil, "pusher can handle\nany number of cells" ,true)

     x,y=show_caption_item(-0.55,1,i3,"victory is close",{1,1,1,0.5})
  end,

  [40] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i2,6,"HOLD", true)
     x,y=show_caption_item(0,-32,nil,"First of all",{0,1,0,0.8},{0,1,0,0.8})
     x,y=show_text(x,y,nil, "you can complete any\nlevel without dragging.\nit just takes longer." ,true)
     x,y=show_caption(0.2,0.1,{type="pin",color_index=1},"TARGET",i2.state.color)
     x,y=show_text(x,y,nil, "drag the right cell here" ,true)
  end,

  [41] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i2,6,"TAP", true)
     x,y=show_caption(-0.2,sf and 0.5 or 0.4,{type="pin",color_index=1},"TARGET",i2.state.color)
     x,y=show_text(x,y,nil, "place the right cell here" ,true)
     x,y=show_caption_item(-0.3,-0.55,i2,"TAP DETECTED !",{1,1,1,0.3})
     x,y=show_text(x,y,nil, "oh well..." ,true)
  end,
  [42] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i2,6,"HOLD", true)
     x,y=show_caption(0.2,0.1,{type="pin",color_index=1},"TARGET",i2.state.color)
     x,y=show_text(x,y,nil, "drag-and-drop the right cell here" ,true)
     x,y=show_caption_item(-0.3,-0.55,i1,"HMMMM",{1,1,1,0.3})
     x,y=show_text(x,y,nil, "we are not looking\nfor easy ways !" ,true)
  end,
  [43] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i2,6,"TAP", true)
     x,y=show_caption(-0.2,sf and 0.5 or 0.4,{type="pin",color_index=1},"TARGET",i2.state.color)
     x,y=show_text(x,y,nil, "place the right cell here" ,true)
     x,y=show_caption_item(0.1,-0.55,i1,"ACHIEVEMENT UNLOCKED",{1,1,1,0.3})
     x,y=show_text(x,y,nil, "< Master of Tapping >" ,true)
  end,
  [44] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(0.5,0.5,i2,"TARGET",i2.state.color)
     x,y=show_text(x,y,nil, "push it here" ,true)
     x,y=show_caption_item(0.4,-0.35,i1,"PUSHER",i1.state.color)

  end,
  [45] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i2,6,"TAP", true)
     x,y=show_caption(0,sf and 0.4 or 0.3,{type="pin", color_index=4},"FINISH IT",i2.state.color)

  end,
  [46] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     x,y=show_caption_item(0,-32,nil,"Jumping",{1,1,1,0.5})
     x,y=show_text(x,y,nil, "cell can't jump\nover other cells" ,true)
     show_ico(i1,6,"HOLD", true)
     x,y=show_caption_item(0.4,-0.35,i1,"PUSHER",i1.state.color)
     x,y=show_caption_item(-0.3,-0.55,i2,"DRAG AND PUSH",i1.state.color)
     x,y=show_text(x,y,nil, "drag the left cell ontop" ,true)
     x,y=show_caption_item(0.5,0.5,i2,"TARGET",i2.state.color)
     x,y=show_text(x,y,nil, "push it here" ,true)
  end,

  [50] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(0,sf and 0.8 or 0.6,i2,"arrow on the board !",{1,1,1,0.4})
     x,y=show_text(x,y,nil, "arrows allows you to\nchange cell's direction." ,true)
  end,

  [51] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(0,sf and 0.8 or 0.6,i2,"just like that",{1,1,1,0.4})
     x,y=show_text(x,y,nil, "Let's finish this\nand let's move on" ,true)
  end,

  [60] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(0.4,-0.4,i1,"START",i1.state.color)
     x,y=show_text(x,y,nil, "let's talk about arrows" ,true)
  end,

  [61] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(sf and -0.2 or -0.0,-0.4,i2,"FACT (1/6)",{1,1,1,0.1})
     x,y=show_text(x,y,nil, "there may be several arrows" ,true)
  end,

  [62] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(-0.0,-0.4,i2,"FACT (2/6)",{1,1,1,0.1})
     x,y=show_text(x,y,nil, "arrow permanently\nchanges a cell\ndirection" ,true)
  end,

  [63] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(-0.0,sf and 0.7 or 0.6,i2,"FACT (3/6)",{1,1,1,0.1})
     x,y=show_text(x,y,nil, "arrow never change\nits direction" ,true)
  end,

  [64] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(-0.0,sf and 0.6 or 0.5,i2,"FACT (4/6)",{1,1,1,0.1})
     x,y=show_text(x,y,nil, "arrow never moves" ,true)
  end,

  [65] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     x,y=show_caption_item(1.4,sf and 2 or 1,i2,"FACT (5/6)",{1,1,1,0.1})
     x,y=show_text(x,y,nil, "usually, there are no useless\narrows. you'll need to use all\nof them somehow." ,true)
  end,

  [66] = function(i1,i2)
     local vb = sen.vp_box()
     local sf =  (vb.r-vb.l) < 460
     local x,y
     show_ico(i1,6,"TAP", true)
     local pin = i1:get_neighbor(i1.state.dir)
     show_caption_item(-0.2,-0.2,pin,"FINISH",i1.state.color)
     x,y=show_caption_item(-0,-0.3,i2,"FACT (6/6)",i1.state.color)
     x,y=show_text(x,y,nil, "cell can't jump\nover an arrow" ,true)
  end,

  [67] = function(i1,i2)
    tap_mark(i1)
  end,
  
  [100] = function(i1,i2)
    local x,y
    local item1 = findItem({type="item", color_index=2})
    local item2 = findItem({type="item", color_index=1})
    if item1.col==-2 and item1.row==0 and
       item2.col==1 and item2.row==-1 then
       x,y=show_caption_item(0,-32,nil,"FINAL LESSON",{1,0,0,0.5})
       x,y=show_text(x,y,nil, "By the end of the lesson,\nthe game mechanics will no\nlonger be a mystery to you.\nGood Luck." ,true)
     end
    tap_mark(i1,i2)
  end,
  
  
}

local finish6 = false
local TRACE_MAX = 6
local function iscr(item, col, row)
  if not item then return -1 end
  local c = item.col
  local r = item.row
  if c == col and r == row then
    return 0
  end
  local d = item.state.dir or 6
  for i=1,TRACE_MAX do
    c,r = base_cell.get_ij(d, c, r)
    if c == col and r == row then
      return 1
    end
  end
  return -1
end

local tutorial_state =
{
-- BASICS 1
function ()
  local item = findItem({type="item"})
  local ret = tutorial_data[DO_NOTHING]
  if item then
    if  level.is_pinned(item.state) then ret =tutorial_data[DO_GJ]
    elseif iscr(item, -1, -1)==0 then ret=tutorial_data[11]
    elseif iscr(item, 0, 0)==0 then ret=tutorial_data[12]
    else
      bad_items = {}
      table.insert(bad_items, item)
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
   end
  end
  return ret
end,

-- MULTITUDE
function ()
  local item1 = findItem({type="item",color_index=2})
  local item2 = findItem({type="item",color_index=4})
  local ret = tutorial_data[DO_NOTHING]
  if item1 and item2 then
    local r1 = iscr(item1, -1, 0)
    local r2 = iscr(item2, 1, 0)
    local fail1 = r1 < 0
    local fail2 = r2 < 0
    if fail1 or fail2 then
      bad_items = {}
    end
    if fail1 then
      table.insert(bad_items, item1)
    end
    if fail2 then
      table.insert(bad_items, item2)
    end
    if fail1 or fail2 then
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
    elseif r1>0 and r2>0 then
      ret=tutorial_data[21]
    elseif r1 > 0 or r2 > 0 then
      ret=tutorial_data[22]
    else
      ret =tutorial_data[DO_GJ]
    end
  end
  return ret,item1,item2
end,

-- MULTITUDE p1/4
function ()
  local item1 = findItem({type="item",color_index=1})
  local item2 = findItem({type="item",color_index=2})
  local item3 = findItem({type="item",color_index=3})
  local item4 = findItem({type="item",color_index=4})
  local ret = tutorial_data[DO_NOTHING]
  if item1 and item2 then
    
    local r1 = iscr(item1, -2, 0)
    local fail = false
    if r1 < 0 then
      fail = true
      bad_items = {}
      table.insert(bad_items, item1)
    end
    local r2 = iscr(item2, 2, 0)
    if iscr(item2, 2, 0) < 0 then
      if not fail then
        bad_items = {}
        fail = true
      end  
      table.insert(bad_items, item2)
    end
    local r3 = iscr(item3, 0, -2)
    if iscr(item3, 0, -2) < 0 then
      if not fail then
        bad_items = {}
        fail = true
      end  
      table.insert(bad_items, item3)
    end
    local r4 = iscr(item4, 0, 1)
    if iscr(item4, 0, 1) < 0 then
      if not fail then
        bad_items = {}
        fail = true
      end  
      table.insert(bad_items, item4)
    end

    if fail then
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
    elseif r1==0 and r2==0 and r3==0 and r4==0 then
      ret =tutorial_data[DO_GJ]
    else
      ret=tap_mark
    end
    
  end
  return ret,item1,item2,item3,item4
end,

-- SHIFT
function ()
  local item1 = findItem({type="item",color_index=1})
  local item2 = findItem({type="item",color_index=3})
  local item3 = findItem({type="item",color_index=4})
  local ret = tutorial_data[DO_NOTHING]
  ----[[
  if item1 and item2 and item3 then

    local fail1 = false
    local fail2 = false
    local fail3 = false

    if iscr(item2, -1, -1) ~= 0 and
       iscr(item2, 0, 0) ~= 0 and
       iscr(item2, 1, 0) ~= 0 then
      fail2 = true
    end

    if iscr(item1, 0, 0) < 0 then
      fail1 = true
    end

    if iscr(item3, 1, 0) < 0 then
      fail3 = iscr(item3, 2, 1) ~= 0
    end
    if not fail3 and iscr(item3, 1, 0) == 0 then
      fail3 = iscr(item1, 1, 0) <= 0
    end
    if not fail3 and iscr(item3, 0, 1) == 0 then
      fail3 =  iscr(item2, 1, 0) == 0 and
               iscr(item1, 0, 0) == 0
    end

    if fail1 or fail2 or fail3 then
      bad_items = {}
    end

    if fail1 then
      table.insert(bad_items, item1)
    end
    if fail2 then
      table.insert(bad_items, item2)
    end
    if fail3 then
      table.insert(bad_items, item3)
    end

    if fail1 or fail2 or fail3 then
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
    else
      if iscr(item1, -2, -1) == 0 and
        iscr(item3, 0, 1) == 0
       then
        ret=tutorial_data[30]
      elseif iscr(item1, -2, -1) == 0 and
             iscr(item3, 1,0) == 0 then
        ret=tutorial_data[31]
      elseif iscr(item1, -1, -1) == 0 and
             iscr(item2, 0,0) == 0 and
             iscr(item3, 0,1) == 0 then
        ret=tutorial_data[32]
      elseif iscr(item1, -1, -1) == 0 and
             iscr(item2, 0,0) == 0 and
             iscr(item3, 1,0) == 0 then
        ret=tutorial_data[33]
      else
        ret=tutorial_data[DO_GJ]
      end

    end

  end
  --]]
  return ret,item1,item2,item3
end,

--- PUSHING  2/5
function ()
  local item1 = findItem({type="item",color_index=1})
  local item2 = findItem({type="item",color_index=2})
  local item3 = findItem({type="item",color_index=3})
  local ret = tutorial_data[DO_NOTHING]

  if item1 and item2 and item3 then
    local r1 = iscr(item1, 0, 0)
    local r2 = iscr(item1, 1, -1)
    
    local fail = false
    if iscr(item1, -2, -1) ~= 0 and
       iscr(item1, -1, -1) ~= 0 then
       fail = true
       bad_items = {}
       table.insert(bad_items, item1)
    end
    
    if (iscr(item3, 0, -1) ~= 0 and
       iscr(item3, -1, -1) ~= 0  and
       iscr(item3, 0, 0) ~= 0  and
       iscr(item3, 1, -1) ~= 0 and
         not ( iscr(item2, 0, 0) > 0 and
               item2:get_neighbor(item2.state.dir) == item3
             )
       )
       or
       (iscr(item1, -2, -1) ~= 0 and
        iscr(item3, 0, -1) == 0)
       or
        (
          iscr(item2, 0, 0) <= 0 and
          iscr(item3, 1, -1) ~= 0
        )       
    then    
       if not fail then
         fail = true
         bad_items = {}
       end  
       table.insert(bad_items, item3)
    end
    
    if iscr(item2, 0, 0) < 0 then
       if not fail then
         fail = true
         bad_items = {}
       end  
       table.insert(bad_items, item2)
    end
    

    if fail then
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
    elseif level.is_pinned(item1.state) and
            level.is_pinned(item2.state) and
            level.is_pinned(item3.state) 
    then
      ret = tutorial_data[DO_GJ]
    else
      local i1 = item1
      local i2 = item2
      local i3 = item3
      if iscr(i3, 0, -1) ~= 0 and
         not level.is_pinned(i3.state) 
      then
        item3 = nil
      end
    
      if iscr(i3, -1, -1) ~= 0 and
         not level.is_pinned(i1.state) 
      then
        
        item1 = nil
      end

      if i2:get_neighbor(i2.state.dir) ~= i3
      then
        item2 = nil
      end

      ret = tap_mark
    end  
    
  end
  return ret,item1,item2,item3
end,

-- DRAG AND SHIFT
function ()
  local item1 = findItem({type="item",color_index=1})
  local item2 = findItem({type="item",color_index=4})
  local ret = tutorial_data[DO_NOTHING]
  if item1 and item2 then

    local r1 = iscr(item1, 0, 0)
    local r2 = iscr(item2, 0, 0)
    local r3 = iscr(item2, 0, 1)
    local fail1 = r1 < 0
    local fail2 = r2 < 0 and r3 < 0

    if not fail1 and not fail2 then
      if r2 > 0 and r1 <= 0 then
        fail2 = true
      end
    end

    if fail1 or fail2 then
      bad_items = {}
    end
    if fail1 then
      table.insert(bad_items, item1)
    end
    if fail2 then
      table.insert(bad_items, item2)
    end

    if fail1 or fail2 then
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
    else

      if iscr(item1, -2, -1) == 0 and
         iscr(item2,  2, -1) == 0 then
        ret = tutorial_data[40]
      elseif iscr(item1, -2, -1) == 0 and
             iscr(item2,  1, -1) == 0 then
        ret = tutorial_data[41]
      elseif iscr(item1, -1, -1) == 0 and
             iscr(item2,  2, -1) == 0 then
        ret = tutorial_data[42]
      elseif iscr(item1, -1, -1) == 0 and
             iscr(item2,  1, -1) == 0 then
        ret = tutorial_data[43]
      elseif iscr(item1, -1, -1) == 0 and
             iscr(item2,  0, 0) == 0 then
        ret = tutorial_data[44]
      elseif iscr(item1, 0, 0) == 0 and
             iscr(item2,  1, 0) == 0 then
        ret = tutorial_data[45]

      elseif iscr(item2, 0, 0) == 0 and
             iscr(item1,  -2, -1) == 0 then
        ret = tutorial_data[46]
      else
        ret = tutorial_data[DO_GJ]
      end
    end

  end
  return ret,item1,item2
end,


--- PUSHING EXERS 3/5
function ()
  local item1 = findItem({type="item",color_index=2})
  local item2 = findItem({type="item",color_index=4})
  local item3 = findItem({type="item",color_index=3})
  local ret = tutorial_data[DO_NOTHING]

  if item1 and item2 and item3 then
    ret = tap_mark
    
    local fail =false
    if iscr(item2,0, 1) < 0 and
       item2:get_neighbor(5)~=item1 then
       fail = true
       bad_items = {}
       table.insert(bad_items, item2)
    end

    if (iscr(item3,1, -1) ~= 0 and
       iscr(item3,-1, 1) < 0 and (
       item3.col==0 and (item2.col~=0 or item2.row>item3.row)  ))
       or
       (item3.row>1)
    then   
       if not fail then
         fail = true
         bad_items = {}
       end  
       table.insert(bad_items, item3)
    end

    if (item3.col<=0 and item1.col~=-1) or
       (item1.col==-1 and iscr(item1,-1, 0) < 0)
    
    then   
       if not fail then
         fail = true
         bad_items = {}
       end  
       table.insert(bad_items, item1)
    end
    
    if fail then
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
    elseif level.is_pinned(item1.state) and 
            level.is_pinned(item2.state) and
            level.is_pinned(item3.state) 
    then
      ret = tutorial_data[DO_GJ]
    else
      
      if item3.col == 1 and item2.col == 0 and item2.row>-2 or
         item3.col == 0 and item2.row==0
      then
        item2 = nil
      end

      local n3 = item3:get_neighbor(6)
      --print(n3~=item1)
      if not ( n3.col==-1 and n3.row==1 ) and
         ( n3~=item1 or (n3.col==-1) ) and
         not level.is_pinned(item3.state)
      then
        item3 = nil
      end

      if item1.col == 0 and item1.row <= 0  then
        item1 = nil
      end
      ret = tap_mark
    end  

  end
  return ret,item1,item2,item3,item4
end,


--- ARROW
function ()
  local item1 = findItem({type="item"})
  local ret = tutorial_data[DO_NOTHING]

  if item1 then
    item2 = item1:get_neighbor(item1.state.dir)
    --print(item1.col, item1.row)
    ----[[
    local r1 = iscr(item1, 0, 0)
    local r2 = iscr(item1, 1, -1)

    local fail1 = r1<0 and r2<0

    if fail1 then
      bad_items = {}
      table.insert(bad_items, item1)
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
    elseif r2==0 then
      ret = tutorial_data[DO_GJ]
    elseif r1 > 0 then
      ret = tutorial_data[50]
    else
      ret = tutorial_data[51]
    end
    --]]
  end
  return ret,item1,item2
end,

--- ARROW EXERC4/5
function ()
  local item1 = findItem({type="item", color_index=2})
  local item2 = findItem({type="item", color_index=4})
  local ret = tutorial_data[DO_NOTHING]
  
  if item1 and item2 then
    
    local fail = false
    if iscr(item2, -1, 0) == 0 and
       iscr(item1, 0, 1) ~= 0 then
       fail = true
       bad_items = {}
       table.insert(bad_items, item2)
    end
 
    if (item2.state.dir == 4) and
       (
        iscr(item2, 0, 0) ~= 0 and
        iscr(item2, 1, -1) < 0
       )
    then   
     
       if not fail then
         fail = true
         bad_items = {}
       end  
       table.insert(bad_items, item2)
    end

    if  iscr(item2, 0, 0) == 0 and
        iscr(item1, 0, 1) ~= 0
       
    then   
     
       if not fail then
         fail = true
         bad_items = {}
       end  
       table.insert(bad_items, item2)
    end
    
    if item1.state.dir == 4 and
        iscr(item1, 1, 0) < 0 
    then   
     
       if not fail then
         fail = true
         bad_items = {}
       end  
       table.insert(bad_items, item1)
    end

    
    
    if fail then
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
    elseif level.is_pinned(item1.state) and level.is_pinned(item2.state) then
      ret = tutorial_data[DO_GJ]
    else
    
      if iscr(item2, -1, 0) == 0 then
        item1 = nil
      end

      if iscr(item2, 0, 0) == 0 then
        item2 = nil
      end
    
      ret = tap_mark
    end  
    
  end
  return ret,item1,item2
end,


--- ARROWS
function ()
  local item1 = findItem({type="item"})
  local item2 = nil
  local ret = tutorial_data[DO_NOTHING]
  local idx = 0
  if item1 then
    item2 = item1:get_neighbor(item1.state.dir)

    local a = iscr(item1, -1, -1)
    if (a > 0) then
      ret = tutorial_data[60]
    else
      a = iscr(item1, 0, -1)
      if (a > 0) then
        ret = tutorial_data[61]
        finish6 = false
      else
        a = iscr(item1, 1, -1)
        if (a > 0 and item1.state.dir == 4) then
          ret = tutorial_data[62]
        --  finish6 = false
        else
          a = iscr(item1, 1, 0)
          if (a > 0 and item1.state.dir == 5) then
            idx = finish6 and 67 or 63
            ret = tutorial_data[idx]
          --  finish6 = false
          else
            a = iscr(item1, 0, 1)
            if (a > 0) then
              idx = finish6 and 67 or 64
              ret = tutorial_data[idx]
            else
              a = iscr(item1, -1, 0)
              if (a > 0) then
                idx = finish6 and 67 or 65
                ret = tutorial_data[idx]
              elseif item1.col==-1 and item1.row==0 then
                if not finish6 then
                  item2 = item2:get_neighbor(item1.state.dir)
                  ret = tutorial_data[66]
                  finish6 = true
                else
                  ret = tutorial_data[67]
                end
              else
                ret = tutorial_data[DO_GJ]
              end
            end
          end
        end
      end
    end
  end
  return ret,item1,item2
end,

--- ARROW EXERC5/5
function ()
  local item1 = findItem({type="item", color_index=2})
  local item2 = findItem({type="item", color_index=1})
  local ret = tutorial_data[DO_NOTHING]
  
  if item1 and item2 then
    
    local fail = false
    if iscr(item2, 1, -1) ~= 0 and
       iscr(item2, 1, 0) < 0 then
       fail = true
       bad_items = {}
       table.insert(bad_items, item2)
    end
 
    if (item1.state.dir == 6) and
        iscr(item1, 0, 0) < 0 
    then   
     
       if not fail then
         fail = true
         bad_items = {}
       end  
       table.insert(bad_items, item1)
    end

    
    if fail then
      mark_bad_items()
      ret = tutorial_data[DO_FAIL]
    elseif level.is_pinned(item1.state) and level.is_pinned(item2.state) then
      ret = tutorial_data[DO_GJ]
    else
    
      if iscr(item2,2, 0) ~= 0 and
         iscr(item2,1, 0) ~= 0 
      then
        item2 = nil
      end
    
      ret = tutorial_data[100]
    end  
    
  end
  return ret,item1,item2
end,

}

local g_stage = nil
local lastZoomX = 1
local lastZoomY = 1
local showZoom = false
local showZoomDone = false
local znode = nil

local function doShowZoom()
  if showZoomDone then return end
  local x,y
  x,y = show_caption_item(-0,-32,nil,"Zooming the Board", {0.4,0.4,0.4,0.4}, {0,0,1,0})
  x,y=show_text(x,y,nil, "The board can be automatically\nzoomed in and out. This is the\nnormal behavior in Cell Motus.",true)

  showZoomDone = true
end


local update_tutorial = function(_board, reset)
  board = _board

  tnode_root = board.tutorial_node
  if tnode_root == nil then
    tnode_root = sen.clsNode()
    tnode = sen.clsNode()
    znode = sen.clsNode()
    tnode.caps = {}

    tnode_root.title = atLabel(tnode_root)
    tnode_root.title.index = 0
    board.tutorial_node=tnode_root
    tnode_root.addChild(tnode)
    tnode_root.addChild(znode)
    tnode_root.node = tnode
    board.scene.addChild(tnode_root)
  end
  tnode = tnode_root.node

  local sb = sen.vp_box()
  local w=sb.r-sb.l
  w = w<=0 and 1 or w
  local h=sb.t-sb.b
  h = h<=0 and 1 or h
  local titl = tnode_root.title
  local tt = tutorial_titles[board.lvls.curr]
  
  px = sb.l+10
  py = sb.t-15 - g_screen.fonts.height.mecha_b*2

  if tt~=nil then
    titl:moveTo(px, py)
  else
    actionManager.stop(titl.node)
    titl:moveTo(-w*1000,-h*1000)
  end


  local statef = tutorial_state[board.lvls.curr]

  if reset then
    clear_stage()
    g_stage = nil
    
  elseif (titl.index ~= board.lvls.curr) then
    titl.index =board.lvls.curr
    if tt ~= nil then
      titl:restart {
        pointerColorIndex=4,
        pointerText = '11',
        Text='TUTORIAL: '..tt..'   ', --.tutorial_titles[lvlmanager.curr]
        small = true,
        quadColorT = {0,1,0,0},
        pointerColorT =  {0,1,0,1},
        labelColorT = {1,1,1,0.8}
      }

      actionManager.run(titl.node,slide_to(px,sb.b-10,px,py,1))
    --  znode.clear()
      showZoom = false
      showZoomDone = false
    end
  end

  if board._ScaleX and (
     lastZoomX > board._ScaleX or
     lastZoomY > board._ScaleY) then
       showZoom = true
  else
    --showZoomDone = false
  end
  lastZoomX = board._ScaleX or 1
  lastZoomY = board._ScaleY or 1
  
----[[
  if not statef then
    clear_stage()
    g_stage=nil
    return
  end
  local showStatef,i1,i2,i3,i4 = statef()


  if (g_stage == showStatef)and(g_stage==tutorial_data[DO_FAIL]) then return end
  clear_stage()
  if showZoom then
    doShowZoom()
  end


  g_stage = showStatef
  if not showStatef then  return end
  showStatef(i1,i2,i3,i4)
  --]]
end

return update_tutorial
