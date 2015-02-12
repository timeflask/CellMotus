local sen           = require "sen.core"
local base_cell     = require "game.cell"
local base_board    = require "game.board"
local conf          = require "game.conf"
--local scheduler     = sen.Scheduler()
--local actionManager = sen.ActionManager
local settingsManager = sen.SettingsManager

class "level"

function level.is_empty(state)
  return state == nil
end

function level.is_item(state)
  return state and state.type and state.type == "item"
end

function level.is_arrow(state)
  return state and state.type and state.type == "arrow"
end

function level.is_pin(state)
  return state and state.type and state.type == "pin"
end

function level.is_pinned(state)
  return state and state.type and state.type == "item" and
         state.pin and state.pin.color == state.color
end

function level.copy_state(state)
  if state == nil then return nil end
  local r = {}
  for k,v in pairs(state) do
    r[k]=v
  end
  return r
end

function level.cmp_colors(s1,s2)
  if s1.color == nil and s2.color then return false end
  if s2.color == nil and s1.color then return false end
  if s2.color == nil and s1.color==nil then return true end

  return s1.color[1]  == s2.color[1] and
         s1.color[2]  == s2.color[2] and
         s1.color[3]  == s2.color[3]

end



function level.cmp_state(s1,s2)
  return sen.rcmp(s1,s2)
end

function level:make_item(color, direction, internal)
 self.nitems = self.nitems + 1
 local ret = { type="item", color=color, dir = direction,  }

 if level.is_pin(internal)  then
  -- self.npins = self.npins + 1
   ret.pin = internal
 elseif (level.is_arrow(internal))   then
  -- self.narrows = self.narrows + 1
   ret.arrow = internal
 end

 return ret
end

function level:make_pin(color)
  self.npins = self.npins + 1
 return { type="pin", color=color,  }
end


function level:make_arrow(dir)
  self.narrows = self.narrows + 1
 return { type="arrow", dir=dir,  }
end

function table.shallow_copy(t)
  local t2 = {}
  for k,v in pairs(t) do
    t2[k] = v
  end
  return t2
end

local colors = table.shallow_copy(conf.cell_colors)

local function swap(array, index1, index2)
    array[index1], array[index2] = array[index2], array[index1]
end

local function shuffle(array)
    local counter = #array
    while counter > 1 do
        local index = math.random(counter)
        swap(array, index, counter)
        counter = counter - 1
    end
end

function level:fillData(data)


  local content = nil
  if data.t == "item" then
    local internal = data.a and self:make_arrow(data.a) or data.p and self:make_pin(colors[data.p])
    content = self:make_item(colors[data.c], data.d, internal)
    content.color_index = data.c
  elseif data.t == "pin" then
    content = self:make_pin(colors[data.c])
    content.color_index = data.c
  elseif data.t == "arrow" then
    content = self:make_arrow(data.d)
  end
  if content then
    self.map[data.i][data.j] = content
    self.ntotal = self.ntotal  + 1
  end
end

local tag = 1

function level:level(cfg)
  self.map = sen.auto_table(2)

  self.nitems = 0
  self.npins = 0
  self.narrows = 0
  self.ntotal = 0
  self.tag = tag
  tag = tag + 1

  local map = self.map

  if settingsManager.get("random_colors", true) then
    shuffle(colors)
  else
    colors =table.shallow_copy(conf.cell_colors)
  end

  for i,v in ipairs(cfg) do
    self:fillData(v)
  end
  self.scale = 7 / (cfg.maxn or 5)
  self.maxn =  (cfg.maxn or 5)
  self.sx = cfg.sx or 0
  self.sy = cfg.sy or 0
end

function level:max_items()
  return self.items
end
--[[
function level:is_done()
  local pinned = 0
  for j,c in pairs(self.board.cells) do
    for i,v in pairs(c) do
      if level.is_pinned(v.state) then
        --print(sen.inspect(v.state))
        pinned  = pinned  + 1
      end
    end
  end
  return pinned == self.nitems
end
--]]

return level
