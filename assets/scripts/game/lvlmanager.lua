local sen           = require "sen.core"
local base_cell     = require "game.cell"
local base_board    = require "game.board"
local level         = require "game.level"
local scheduler     = sen.Scheduler()
local actionManager = sen.ActionManager
local ai            = require "game.ai"
local settingsManager = sen.SettingsManager

class "lvlmanager"

    
local levels = require "levels"

function lvlmanager:lvlmanager()
  local progress = settingsManager.get('progress',1)
  self.curr = settingsManager.get('curr',1)
  if self.curr > progress then
    self.curr = progress
    settingsManager.set('curr',self.curr)
  end
end

function lvlmanager:set(lvl)
  local progress = settingsManager.get('progress',1)
  if lvl and  lvl <= progress and lvl > 0 then
    self.curr = lvl
  else
    self.curr = progress       
  end
  settingsManager.set('curr',self.curr)
  return level(levels[self.curr])
end

function lvlmanager:count()
  return #levels
end

function lvlmanager:dev()
  return level ( dofile('../CellMotus/assets/scripts/dev.lua') )
end

function lvlmanager:next(step)
  if step == nil then return self:set() end
  
  self.curr = self.curr + step
  
  local progress = settingsManager.get('progress',1)
  
  if (self.curr > #levels) then
    self.curr = 1
  elseif   (self.curr < 0) then
    self.curr = progress
  elseif self.curr > progress then
    settingsManager.set('progress',self.curr)
  end
  
  settingsManager.set('curr',self.curr)
  
  --local lvl = level(self.board, ai())
  
  local lvl = level(levels[self.curr])
  return lvl
end

function lvlmanager:is_top_progress()
  return self.curr == settingsManager.get('progress',1)
end

return lvlmanager()
