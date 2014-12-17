local sen           = require "sen.core"
local conf          = require "game.conf"
local scheduler     = sen.Scheduler()
local actionManager = sen.ActionManager
local conf          = require "game.conf"
local audioPlayer   = sen.AudioPlayer
local settingsManager = sen.SettingsManager

local rs    = (require "resources").str

local camera = sen.camera()
local bbox = {}


local rand = math.random
local pow = math.pow

class "optLine"

local CoroWait = sen.CoroWait

local optLineCoro = function (node, dt)
  node.clear()
  CoroWait(dt, rand()*0.5)
  
  local cfg = node.cfg
  local sv = settingsManager.get(cfg.settings_key,cfg.setting_def)
 
  local quad = sen.clsQuad()
  local tcol =  sv and {1,1,1, 0.04} or {1,1,1,0.02}
 
  local hsz = bbox.r - bbox.l
  local lh = cfg.Height or 100 
  quad.setColor(tcol)
  quad.setAnchor(0.5,0)
  quad.moveTo(bbox.r, 0 )
  
  node.quad = quad
  node.addChild(quad)

  local q1 = sen.clsQuad()
  q1.setColor({0.15,0.15,0.15,1})
  --q1.setColor({a=0.05})
  q1.moveTo(bbox.r, 0 )
  q1.scale(20,20)
  node.q1 = q1
  node.addChild(q1)

  local q2 = sen.clsQuad()
  q2.moveTo(bbox.r, 0 )
  q2.scale(16,16)
  q2.setColor(conf.lvl_colors[3])
  q2.setColor({a=sv and 1 or 0})
  node.q2 = q2
  node.addChild(q2)

 
  local text = (cfg.LabelTitle or "some option")
  local  lb = sen.clsLabel(nil, "mecha_sm", text)
  lb.setColor({a=0})
  local lba = sv and 0.75 or 0.4
  node.lb = lb
  node.addChild(lb)

  local  lb2 = sen.clsLabel(nil, "mecha_s", cfg.DescLabelTitle or '')
  lb2.setColor({a=0})
  
  node.lb2 = lb2
  node.addChild(lb2)
 
   actionManager.run(quad,
   {
      name = "interval",
      duration = 1.5,
      rate = rand()*2,
      amax = 0.5,
      period = 1,
        
      trigger = function(self, dt, cf)
        local dx = conf.btime(math.pow(dt,cf.rate))
        self.scale(dx*hsz, lh)
        self.setColor({a = cf.amax * pow(dt,cf.rate)})
        lb.moveTo(bbox.r-dx*hsz + 32,  0)
        lb2.moveTo(bbox.r-dx*hsz + 32, -9 )
        q1.moveTo(bbox.r-dx*hsz + 17, 0 )
        q2.moveTo(bbox.r-dx*hsz + 17, 0 )
        lb.setColor({a=pow(dt,cf.rate)*lba})
        lb2.setColor({a=pow(dt,cf.rate)*lba-0.2})
      end,
      start_trigger = function(self,cf)
        cf.amax = self.color().a
        self.setColor({a=0})
      end
   }, 'quad_show')
 
 
 
 actionManager.wait(quad, 'quad_show')
   
-- print("LOL")
 return 1
end

local optLineClickCoro = function (node, dt)

 local cfg = node.cfg
 --print(cfg.settings_key, settingsManager.get(cfg.settings_key,cfg.setting_def))
 local sv = not settingsManager.get(cfg.settings_key,cfg.setting_def)
 settingsManager.set(cfg.settings_key,sv)
 
 
 audioPlayer.playSound(sv and "button0.mp3" or "button1.mp3")
  --print(cfg.settings_key, settingsManager.get(cfg.settings_key,cfg.setting_def))
-- local text = (sv and '[x] ' or '[ ] ')..(cfg.LabelTitle or "some option")
 --node.lb.setText(text)
 if sv then
   actionManager.run(node.q2, conf.effect_fadeIn(2,1.2,0.98))
 else  
   actionManager.run(node.q2, conf.effect_fadeOut(2,1.2))
 end  
 --node.q2.setColor({a=sv and 1 or 0})
 
 node.lb.setColor({a=sv and 0.75 or 0.4})
 node.lb2.setColor({a=(sv and 0.75 or 0.4)-0.2})
 node.quad.setColor(  sv and {1,1,1, 0.04} or {1,1,1,0.02})
 local col = node.quad.color()
 local tcol = sv and {0.5,1,0.5,1} or {1,0.5,0.5,1}  
    
 actionManager.run(node.quad, 
 conf.effect_colorTransition(tcol, 5+rand(), 1/2)
 ,'color_transition')

 local ys =  node.quad.scaleY()
   actionManager.run(node.quad,
   {
      name = "interval",
      duration = 0.2,
      rate = rand()*2,
      amax = 0.5,
      period = 1,
      ys = 1,  
      trigger = function(self, dt, cf)
        local dx = math.pow(dt,cf.rate)-- conf.btime(dt)
--        local dx = conf.btime(dt)
        self.scale(self.scaleX(),ys +dx*12)
      end,
      start_trigger = function(self,cf)
      end
 }, 'quad_shake')
    
 actionManager.wait(node.quad)

 actionManager.run(node.quad, 
 conf.effect_colorTransition({col.r,col.g,col.b,col.a}, 5+rand(), 1/2)
 ,'color_transition')

   actionManager.run(node.quad,
   {
      name = "interval",
      duration = 0.2,
      rate = rand()*2,
      amax = 0.5,
      period = 1,
      ys = 1,  
      trigger = function(self, dt, cf)
        local dx = math.pow(dt,cf.rate)-- conf.btime(dt)
        self.scale(self.scaleX(),ys + 12*(1 - dx))
      end,
      start_trigger = function(self,cf)
      end
 }, 'quad_shake')
    
 actionManager.wait(node.quad)
 

 return 1
end

local optLineCloseCoro = function (node, dt)
 local cfg = node.cfg
 node.quad.setAnchor(0.5*(cfg.close_side or -1),0)
 local is_left = cfg.close_side < 0
 node.quad.moveTo(is_left and bbox.l or bbox.r,0)
 local hsz = bbox.r - bbox.l
 local lh = cfg.Height or 100 
 actionManager.run(node.quad,
 {
    name = "interval",
    duration = 0.5,
    rate = rand()*2,
    amax = 0.5,
    period = 1,
      
    trigger = function(self, dt, cf)
      local dx = conf.btime(math.pow(dt,cf.rate))
      self.scale((1-dx)*hsz, lh)
      if not is_left then
        node.lb.moveTo(bbox.r-(1-dx)*hsz + 32,  0)
        node.lb2.moveTo(bbox.r-(1-dx)*hsz + 32, -9 )
        node.q1.moveTo(bbox.r-(1-dx)*hsz + 16, 0 )
        node.q2.moveTo(bbox.r-(1-dx)*hsz + 16, 0 )
      end  
        --lb2.moveTo(bbox.r-dx*hsz + 32, -9 )
        --q1.moveTo(bbox.r-dx*hsz + 16, 0 )
        --q2.moveTo(bbox.r-dx*hsz + 16, 0 )
    end,
    start_trigger = function(self,cf)
    end
 }, 'quad_show')
 return 1
end

local optLineId = 0
function optLine:optLine(node, cfg)
  optLineId = optLineId + 1
  self.coro_name = 'optLineCoro'..tostring(optLineId)
  
  self.node = sen.clsNode()
  node.addChild(self.node)
    
  self.node.cfg = cfg or {}
  --self.lb = atLabel(node)
  self.coro = sen.clsCoro(optLineCoro)  

  self.click_coro_name = 'optLineClickCoro'..tostring(optLineId)
  self.click_coro = sen.clsCoro(optLineClickCoro)  

  self.close_coro_name = 'optLineCloseCoro'..tostring(optLineId)
  self.close_coro = sen.clsCoro(optLineCloseCoro)  
end

function optLine:moveTo(x, y)
  self.node.moveTo(x, y)
end

function optLine:doClick()
 

  scheduler.Remove(self.node, self.click_coro_name)
  scheduler.ScheduleCoro(self.node, 
                         self.click_coro, 
                         self.click_coro_name)
end
function optLine:doClose(side)
  scheduler.Remove(self.node, self.close_coro_name)
  self.node.cfg.close_side = side
  scheduler.ScheduleCoro(self.node, 
                         self.close_coro, 
                         self.close_coro_name)
end
function optLine:restart(cfg)
  bbox = sen.clsScene.getCurrent().getBBox()
  if cfg then
    for k,v in pairs(cfg) do self.node.cfg[k] = v end
  end  
  scheduler.Remove(self.node, self.coro_name)
  scheduler.ScheduleCoro(self.node, 
                         self.coro, 
                         self.coro_name)
  
end
-------------------------------------------------------------------------------------

class "buttonLine"

local buttonLineCoro = function (node, dt)
  node.clear()
  CoroWait(dt, rand()*0.2)
  
  local cfg = node.cfg
  --local sv = settingsManager.get(cfg.settings_key,cfg.setting_def)
 
  local quad = sen.clsQuad()
  local tcol =  cfg.lineColor or {1,1,1,0.05}
  local bw = (bbox.r - bbox.l)
  local hsz = bw / 100 * (cfg.width_p or 100)
  local hshift = bw /100*(cfg.shift_p or 0) 
  
  
  local lh = cfg.Height or 100 
  quad.setColor(tcol)
  quad.setColor({a=node.enabled and quad.color().a or 0.05})
  quad.setAnchor(0.5,0)
  quad.moveTo(bbox.r, 0 )
  
  --quad.blend(3)
  
  node.quad = quad
  node.addChild(quad)
  
----[[
  local q1 = sen.clsQuad()
  --q1.setColor(cfg.pquadColor or {1,1,1,1})
  --q1.setColor({a=node.enabled and q1.color().a or 0.1})
  q1.setColor(node.enabled and (cfg.pquadColor or {1,1,1,1}) or {1,1,1,0.1})
  q1.moveTo(bbox.r, 0 )
  q1.setAnchor(-0.5,0)
  q1.scale(12,lh-0.1)
  node.q1 = q1
  node.addChild(q1)
--]]

----[[
  local q2 =nil 
  if cfg.rico then 
    q2 = sen.clsSprite(nil, cfg.rico)
    --local qb = q2.getBBox()
    local f = math.min((lh - 8),20) / math.max(q2.height(), q2.width())
    q2.scale(f,f) 
    node.q2 = q2
    q2.moveTo(bbox.r-q2.width()*f, 0 )
    q2.setColor{a=0}
    node.addChild(q2)
 end   
--]]
 
  local text = (cfg.LabelTitle or "some option")
  local  lb = sen.clsLabel(nil, "mecha_sm", text)
  lb.setColor(cfg.titleColor or {1,1,1,0.5})
  lb.setColor({a=0})
  local lba1 = cfg.titleColor and cfg.titleColor[4] or 0.5
  node.lb = lb
  node.addChild(lb)

  local  lb2 = sen.clsLabel(nil, "mecha_s", cfg.DescLabelTitle or '')
  lb2.setColor(cfg.titleColor2 or {1,1,1,0.2})
  lb2.setColor({a=0})
  local lba2 = cfg.titleColor2 and cfg.titleColor2[4] or 0.2
  node.lb2 = lb2
  node.addChild(lb2)
 
   actionManager.run(quad,
   {
      name = "interval",
      duration = 0.5,
      rate = rand()*2,
      --amax = 0.5,
      period = 1,
        
      trigger = function(self, dt, cf)
        local dx = conf.btime(math.pow(dt,cf.rate))
        self.scale(dx*hsz, lh)
        self.setColor({a = cf.amax * pow(dt,cf.rate)})
        node.moveTo(dx*(-hshift), node.posY())
        --self.moveTo(dx*(bbox.r-hshift), 0)
        lb.moveTo(bbox.r-dx*hsz + (cfg.labelLshift or 20)    ,  cfg.labelTshift1 or -2)
        lb2.moveTo(bbox.r-dx*hsz + (cfg.labelLshift2 or 20) , cfg.labelTshift2 or -10 )
        q1.moveTo(bbox.r-dx*hsz, 0 )
       -- q2.moveTo(bbox.r-dx*hsz + 17, 0 )
        lb.setColor({a=pow(dt,cf.rate)*(node.enabled and lba1 or 0.1)})
        lb2.setColor({a=pow(dt,cf.rate)*(node.enabled and lba2 or 0.1)})
        if q2 then 
          q2.setColor({a=pow(dt,cf.rate)*(node.enabled and 0.15 or 0.05)})
        end   
      end,
      start_trigger = function(self,cf)
        cf.amax = self.color().a
        --print(cf.amax)
        self.setColor({a=0})
      end
   }, 'quad_show')
 
 
 
 actionManager.wait(quad, 'quad_show')
 --quad.setColor(tcol)
   
-- print("LOL")
 return 1
end

local buttonLineClickCoro = function (node, dt)

 local cfg = node.cfg
 --print(cfg.settings_key, settingsManager.get(cfg.settings_key,cfg.setting_def))
 --local sv = not settingsManager.get(cfg.settings_key,cfg.setting_def)
 --settingsManager.set(cfg.settings_key,sv)
 local snd = cfg.sound and cfg.sound or  "button1.mp3"
 audioPlayer.playSound(snd)
 --print("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
  --print(cfg.settings_key, settingsManager.get(cfg.settings_key,cfg.setting_def))
-- local text = (sv and '[x] ' or '[ ] ')..(cfg.LabelTitle or "some option")
 --node.lb.setText(text)
 --[[
 if sv then
   actionManager.run(node.q2, conf.effect_fadeIn(2,1.2,0.98))
 else  
   actionManager.run(node.q2, conf.effect_fadeOut(2,1.2))
 end 
 --]] 
 --node.q2.setColor({a=sv and 1 or 0})
 
 node.lb.setColor(cfg.titleColor or {1,1,1,0.5} )
 node.lb2.setColor(cfg.titleColor2 or {1,1,1,0.2})
 node.quad.setColor(cfg.lineColor or {1,1,1,0.05})
 node.q1.setColor(node.enabled and (cfg.pquadColor or {1,1,1,1}) or {1,1,1,0.1})
 if node.q2 then
   node.q2.setColor{a=node.enabled and 0.15 or 0.05}
 end

  if not node.enabled then
       node.lb.setColor({a=0.1})
       node.lb2.setColor({a=0.1})
       node.quad.setColor({a=0.05})
       --node.q1.setColor(node.enabled and (cfg.pquadColor or {1,1,1,1}) or {1,1,1,0.1})
  end

 
 local col = node.quad.color()
 local tcol = cfg.clickColor or cfg.pquadColor or conf.cell_colors[2]  
    
 actionManager.run(node.quad, 
 conf.effect_colorTransition(tcol, 5+rand(), 1/2)
 ,'color_transition')

 local ys =  node.quad.scaleY()
   actionManager.run(node.quad,
   {
      name = "interval",
      duration = 0.2,
      rate = rand()*2,
      amax = 0.5,
      period = 1,
      ys = 1,  
      trigger = function(self, dt, cf)
        local dx = math.pow(dt,cf.rate)-- conf.btime(dt)
--        local dx = conf.btime(dt)
        self.scale(self.scaleX(),ys +dx*6)
      end,
      start_trigger = function(self,cf)
      end
 }, 'quad_shake')
    
 actionManager.wait(node.quad)

 actionManager.run(node.quad, 
 conf.effect_colorTransition({col.r,col.g,col.b,col.a}, 5+rand(), 1/2)
 ,'color_transition')

   actionManager.run(node.quad,
   {
      name = "interval",
      duration = 0.2,
      rate = rand()*2,
      amax = 0.5,
      period = 1,
      ys = 1,  
      trigger = function(self, dt, cf)
        local dx = math.pow(dt,cf.rate)-- conf.btime(dt)
        self.scale(self.scaleX(),ys + 6*(1 - dx))
      end,
      start_trigger = function(self,cf)
      end
 }, 'quad_shake2')
    
 actionManager.wait(node.quad)
 

 return 1
end

local buttonLineCloseCoro = function (node, dt)
 --actionManager.stop(node.quad, 'quad_show')
 scheduler.Wait(node, node.coro_name)
 local cfg = node.cfg
 local choosen = cfg.choosen or false
 cfg.choosen = false
 
 node.quad.setAnchor(0.5*(cfg.close_side or -1),0)
 local is_left = cfg.close_side < 0
 if cfg.wait_click then
   scheduler.Wait(node, node.click_coro_name)
   is_left = false
 end  
 node.quad.moveTo(is_left and bbox.l or bbox.r,0)
 local hsz = bbox.r - bbox.l
 local lh = cfg.Height or 100 
 local hshift = node.posX()
 if node.q2 then node.q2.setColor{a=0} end 
 actionManager.run(node.quad,
 {
    name = "interval",
    duration = choosen and 1 or 0.5,
    rate = rand()*2,
    amax = 0.5,
    period = 1,
      
    trigger = function(self, dt, cf)
      local dx = conf.btime(math.pow(dt,cf.rate))
      self.scale((1-dx)*hsz, lh)
      if not is_left then
        node.moveTo((1-dx)*(-hshift), node.posY()) 
        node.lb.moveTo(bbox.r-(1-dx)*hsz +(cfg.labelLshift1 or 20),  cfg.labelTshift1 or -2)
        node.lb2.moveTo(bbox.r-(1-dx)*hsz + (cfg.labelLshift2 or 20), cfg.labelTshift1 or -10 )
        node.q1.moveTo(bbox.r-(1-dx)*hsz, 0 )
        --node.q2.moveTo(bbox.r-(1-dx)*hsz + 16, 0 )
      end  
        --lb2.moveTo(bbox.r-dx*hsz + 32, -9 )
        --q1.moveTo(bbox.r-dx*hsz + 16, 0 )
        --q2.moveTo(bbox.r-dx*hsz + 16, 0 )
    end,
    start_trigger = function(self,cf)
    end
 }, 'quad_show')
 --actionManager.wait(node.quad, 'quad_show')
 
 --scheduler.Wait(node, node.click_coro_name)
 return 1
end


local buttonLineId = 0
function buttonLine:buttonLine(node, cfg)
  buttonLineId = buttonLineId + 1
  self.coro_name = 'buttonLineCoro'..tostring(buttonLineId)
  
  self.node = sen.clsNode()
  self.node.ZOrder(0.1)
  self.node.coro_name = self.coro_name
  node.addChild(self.node)
    
  self.node.cfg = cfg or {}
  self.coro = sen.clsCoro(buttonLineCoro)  

  self.click_coro_name = 'buttonLineClickCoro'..tostring(buttonLineId)
  self.click_coro = sen.clsCoro(buttonLineClickCoro)
  self.node.click_coro_name = self.click_coro_name  

  self.close_coro_name = 'buttonLineCloseCoro'..tostring(buttonLineId)
  self.close_coro = sen.clsCoro(buttonLineCloseCoro)
  
  self.node.enabled = true  
end

function buttonLine:enable(flag)
  local f = flag==nil and true or flag
  local node = self.node
  local cfg = node.cfg
  if node.enabled == f then return end
  node.enabled = f
   if node.q2 then
     node.q2.setColor{a=node.enabled and 0.15 or 0.05}
   end
  if node.enabled then
     if node.lb then
       node.lb.setColor(cfg.titleColor or {1,1,1,0.5})
     end  
     if node.lb2 then
       node.lb2.setColor(cfg.titleColor2 or {1,1,1,0.2})
     end  
     if node.quad then
       node.quad.setColor(cfg.lineColor or {1,1,1,0.05})
     end  
     if node.q1 then  
       node.q1.setColor(cfg.pquadColor or {1,1,1,1})
     end  
  else
     if node.lb then
       node.lb.setColor({a=0.1})
     --else
       --print("AAAAAAAAAAAAAAAAAAAAAAAAAA")  
     end  
     if node.lb2 then
       node.lb2.setColor({a=0.1})
     end
     if node.quad then
       node.quad.setColor({a=0.05})
     end
     if node.q1 then  
       node.q1.setColor(node.enabled and (cfg.pquadColor or {1,1,1,1}) or {1,1,1,0.1})
     end  
  end
end

function buttonLine:checkPoint(x,y)
 local sx = self.width
 local sy = self.height
 local px = self.node.posX()
 local py = self.node.posY()
 bbox = sen.vp_box()-- clsScene.getCurrent().getBBox()
 camera = sen.camera()
 x = x+camera.posX()
 y = y+camera.posY()
 --local bw = (bbox.r-bbox.l)
 --local s = px / bw 
 --print(s)
 --local s = bw-sx
 return x >= (bbox.r+px-sx) and x <= bbox.r+px and y >=py-sy/2 and y <= py+sy/2
end

function buttonLine:moveTo(x, y)
  self.node.moveTo(x, y)
end

function buttonLine:doClick()
 
  scheduler.Remove(self.node, self.click_coro_name)
  if not  self.node.enabled then return end
  scheduler.ScheduleCoro(self.node, 
                         self.click_coro, 
                         self.click_coro_name)
                         
end
function buttonLine:doClose(side, choosen)
  
  --scheduler.Remove(self.node, self.coro_name)
  scheduler.Remove(self.node, self.close_coro_name)
  self.node.cfg.close_side = side
  self.node.cfg.choosen = choosen
  
  scheduler.ScheduleCoro(self.node, 
                         self.close_coro, 
                         self.close_coro_name)
end
function buttonLine:restart(cfg)
  bbox = sen.clsScene.getCurrent().getBBox()
  if cfg then
    for k,v in pairs(cfg) do self.node.cfg[k] = v end
  end  
  self.height = self.node.cfg.Height or 100
  local bw = (bbox.r - bbox.l)
  self.width = bw / 100 * (self.node.cfg.width_p or 100)
  scheduler.Remove(self.node, self.coro_name)
  scheduler.Remove(self.node, self.click_coro_name)
  scheduler.Remove(self.node, self.close_coro_name)
  scheduler.ScheduleCoro(self.node, 
                         self.coro, 
                         self.coro_name)
  
end
