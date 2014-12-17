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

class "animBG"

local CoroWait = sen.CoroWait

local bgid = 0

local bg_coro = function (node, dt)
 local board = node.board

 local _act = nil
 local cols = {
  conf.cell_colors[2],
  conf.cell_colors[3],
  --{0xFF,0x62,0x4D,1},
  {0x57,0x2D,0x8F,1},
  {0xaa,0x75,0xe0,1},
  {0x8a,0x4a,0xc9,1}
 }
 local col =cols[rand(3)]
 local action1 = function(tar, d) return {
      name = "seq",
      actions = { 
      {
        name = "interval",
        duration = 2+rand(),
        rate = 1/2,
        trigger = function(self, dt, conf)
          local dx = pow(dt, conf.rate)
          --self.moveTo(conf.startX+conf.deltaX*pow(dt, conf.rate), y)
          tar.sprite.setColor({a=0.03*(dx)})
        end,
        start_trigger = function(self, conf)
          tar.sprite.setColor(col)
          tar.sprite.setColor({a=0.0})
          tar.sprite.cr = true

-- --[[

          ---]]
            --local c = tar:get_neighbor(rand(6))
            --if c and not c.sprite.cr then
              --actionManager.run(c.sprite, _act(c), 'abg')
            --end
          
        end,
        end_trigger = function(self,conf)
        end
      },
      {
        name = "interval",
        duration = 2+rand(3),
        rate = 2,
        trigger = function(self, dt, conf)
          local dx = pow(dt, conf.rate)
          tar.sprite.setColor({a=0.03*(1-dx)})
        end,
        start_trigger = function(self, conf)
          for i=1,6 do
            local c = tar:get_neighbor(i)
            if c and not c.sprite.cr and d < 20 then
              
              actionManager.run(c.sprite, _act(c,d+1), 'abg')
              --break
            end
            
          end
  
        end,
        end_trigger = function(self,conf)
          --if conf.on_end then
            --conf.on_end(self)
          --end
          tar.sprite.cr = false  
        end
      }
    }
 }end
 
 _act = action1
 local ii = rand(board.min_i+1,board.max_i-1)
 local jj = rand(board.min_j+1,board.max_j-1)
 local start = board.cells[ii] and board.cells[ii][jj] or board.cells[0][0] 
 if start and not start.sprite.cr then
   actionManager.run(start.sprite, action1(start,1), 'abg')
 end  

 return 0
end


function animBG:animBG(board, cfg)
  bgid = bgid + 1
  self.coro_name = 'animBGCoro'..tostring(bgid)
  
  self.board = board
   
  self.node = board.node
  self.node.abgcfg = cfg or {}

  self.coro = sen.clsCoro(bg_coro)  
end

function animBG:reset(cfg)
  if cfg then
    for k,v in pairs(cfg) do self.node.abgcfg[k] = v end
  end  
  self:stop()
  scheduler.ScheduleCoro(self.node, 
                         self.coro, 
                         self.coro_name, 0.5)
end
function animBG:stop()
  
  scheduler.Remove(self.node, self.coro_name)
  actionManager.stop(nil, 'abg')
  local cells = self.board.cells
  for _,c in pairs(cells) do for __,v in pairs(c) do
    v.sprite.cr = false
    v.sprite.setColor({a=0})
  end end
end
