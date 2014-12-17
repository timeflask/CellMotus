local sen           = require "sen.core"
local conf          = require "game.conf"
local scheduler     = sen.Scheduler()
local actionManager = sen.ActionManager

class "atLabel"

local yield  = coroutine.yield
local rand = math.random
local cid = 1

local coro_wait = function(dt,secs)
  local m_elapsed = 0
  while (m_elapsed < secs) do
    yield()
    m_elapsed = m_elapsed + dt
  end
end

local coro = function (ref, dt)
 local node = ref
 if node.quad then
   actionManager.stop(node.quad)
   node.quad = nil
 end  

 if node.lb then
   actionManager.stop(node.lb)
   node.lb = nil
 end  

 if node.q2 then
   actionManager.stop(node.q2)
   node.q2 = nil
 end  
 
 node.clear()

 local cfg = node.cfg
 
 local quad = sen.clsQuad()
 --local tcol = conf.lvl_colors[node.type]
 local tcol = conf.atLabel_colors[cfg.pointerColorIndex or 1]
 
 local is_small = false -- conf.scr_size == 'small'
 
 local qw =  cfg.qw and cfg.qw or ( is_small and 12 or 20)
 
 quad.setColor(tcol)
 quad.setAnchor(0,0.5)
 
 node.quad = quad
 node.addChild(quad)
 
 actionManager.run(quad, conf.effect_blink(3,1,true), 'quad_blink')
 
 actionManager.run(quad,
 {
    name = "interval",
    duration = 0.2,
    rate = 2,
    amax = 0.5,
    period = 1,
      
    trigger = function(self, dt, conf)
      quad.scale(4,dt*qw)
    end,
 }, 'quad_show')
 
 
 
 while actionManager.is_running(quad, 'quad_show') do
   yield()
 end
   
 actionManager.stop(quad, 'quad_blink')  
 quad.setColor(tcol)
 
 local pt = cfg.pointerText or '22'-- tostring(board.lvls.curr)
 --if #pt == 1 then pt='  '..pt end
 local text = cfg.Text or 'No Title    '
 
 
 local  lb = sen.clsLabel(nil, is_small and "mecha_s" or "mecha_m", pt)
 local lbb = lb.getBBox()
 --lb.setColor(1,1,1,1)
-- lb.ZOrder(1)
 lb.move(is_small and 6 or 8,lbb.b-lbb.t-4)
 local qts = lbb.r-lbb.l + 10
 lb.setText('')
 --lb.ZOrder(0.7)
 node.lb = lb
 node.addChild(lb)

 local q2 = sen.clsQuad()
 q2.setColor(tcol)
 q2.scale(qts, is_small and 12 or 22)
 q2.setAnchor(0.5,0)
 q2.moveTo(0,lbb.b-lbb.t + (is_small and -1 or 2))
 node.q2 = q2
 node.addChild(q2)

 
 local len = #text
 local q2s =(lbb.r-lbb.l)/len*0.96
 local i = 1
 while i <= len do
   lb.setText(string.sub(text,1,i))
   local lbb = lb.getBBox()
   q2.moveTo(lbb.r+12,q2.posY())
   i = i + 1
  -- coro_wait(dt, rand()*0.05)
  yield()
 end
 
 
 actionManager.run(lb, 
 conf.effect_colorTransition(cfg.labelColorT or {1,1,1,0.2}, 0.3+rand(), 1/2)
 ,'color_transition')
 actionManager.run(q2, 
 conf.effect_colorTransition(cfg.quadColorT or{0.5,0.5,0.5,0.2}, 0.6+rand(), 1/2)
 ,'color_transition')
 actionManager.run(quad, 
 conf.effect_colorTransition(cfg.pointerColorT or{0.5,0.5,0.5,0.2}, 0.4+rand(), 1/2)
 ,'color_transition')
  
 return 1
end

function atLabel:moveTo(x, y)
  self.node.moveTo(x, y)
end

function atLabel:restart(cfg)
  if cfg then
    for k,v in pairs(cfg) do self.node.cfg[k] = v end
  end  
  scheduler.Remove(self.node, self.coro_name)
  scheduler.ScheduleCoro(self.node, 
                         self.coro, 
                         self.coro_name,0.05)   
end

function atLabel:atLabel(node, cfg)
  cid = cid + 1
  self.coro_name = 'atLabelCoro'..tostring(cid)
  
  self.node = sen.clsNode()
  self.node.ZOrder(0.8)
  node.addChild(self.node)
    
  self.node.cfg = cfg or {}
  self.coro = sen.clsCoro(coro)
end

return atLabel