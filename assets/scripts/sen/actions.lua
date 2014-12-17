
local ffi = require "ffi"
local clsObject = require "sen.object"
local Scheduler = require "sen.scheduler"
local inspect = require "sen.inspect"
local C = ffi.C
local utils = require "sen.utils"
local ripairs = utils.ripairs
local max = math.max
local min = math.min

local EPSILON = 0.00000001
local M_PI = 3.14159265358
local M_PI_2 = 1.57079632679
local M_PI_X_2 = M_PI * 2.0

----------------------------------------------------------------------------
local actions_factory = {}
function actions_factory.__create ( target, config ) 
  local action = actions_factory[ config.name ].new(target, config)
 -- action.onStart()
  return action
end
----------------------------------------------------------------------------
local base_action = {}
function base_action.inherit ( target, config )
   local self = {}
   self.target = target
   self.config = config
   self.trigger = config.trigger
   self.start_trigger = config.start_trigger 
   self.end_trigger = config.end_trigger
   self.first = true
   self.tag = config.tag or '*'
   
  function self.isDone()
    return true 
  end
  
  function self.onStart()
    if self.start_trigger ~= nil then
      self.start_trigger(self.target, self.config)
    end
  end

  function self.onEnd()
    if self.end_trigger ~= nil then
      self.end_trigger(self.target, self.config)
    end
  end
    
  function self.reset()
  end  
    
  return self
end
 
----------------------------------------------------------------------------
local interval_action = {}
function interval_action.new ( target, config )
  local self = base_action.inherit(target, config)
  local duration = config.duration or 1
  local forever = config.forever or false
  
  if duration < EPSILON then
    duration = EPSILON
  end
  
  local first = true
  local elapsed = 0
  
  function self.reset()
    first = true
    elapsed = 0
  end    
  
  function self.isDone()
    local _done = elapsed >= duration
    if  _done and forever then
      first = true
      _done = false
    end  
    return _done
  end

  function self.step(dt)
    if first then
      first = false
      elapsed = 0
    else
      elapsed = elapsed + dt  
    end
    
    self.update(max(0, min(1, elapsed / duration) ))
    --self.trigger(target, , self.config )
  end
  
  function self.update(dt)
    self.trigger(target, dt, self.config )
  end
  
  return self
end

actions_factory.interval = interval_action 
----------------------------------------------------------------------------
local speed_action = {}
function speed_action.new ( target, config )
  local self = base_action.inherit(target, config)
  local speed = config.speed or 1
  local action = actions_factory.__create( target, config.action ) 
  
  function self.isDone()
    return action.isDone() 
  end
  
  function self.step(dt)
    action.step (dt * speed)
  end
  
  function self.onStart()
    if self.start_trigger ~= nil then
      self.start_trigger(self.target, self.config)
    end
    action.onStart()
  end

  function self.onEnd()
    if self.end_trigger ~= nil then
      self.end_trigger(self.target, self.config)
    end
    action.onEnd()
  end
  
  function self.reset()
    action.reset()
  end
     
  return self
end
actions_factory.speed = speed_action 
----------------------------------------------------------------------------
local seq_action = {}
function seq_action.new ( target, config )
  local self = base_action.inherit(target, config)
  local conf_actions = config.actions
  local actions = {}
  for i, v in ipairs(conf_actions) do 
    table.insert(actions, actions_factory.__create( target, v ))
  end
  local num = table.getn(actions)
  local current_action = 1 
  local action = actions[current_action]
  action.onStart()
  local forever = config.forever or false
  
  --action.onStart()
  
  function self.isDone()
    local _done = current_action > num
    if  _done and forever then
      _done = false
      for i, v in ipairs(actions) do 
        v.reset()
      end

       current_action = 1 
       action = actions[current_action]
       action.onStart()
    end  
    return _done
  end
  
  function self.step(dt)
    action.step (dt)
    if action.isDone() then
      action.onEnd()
      current_action = current_action + 1
      if (current_action <= num) then
        action = actions[current_action]
        action.onStart()
      end  
    end  
  end
  
  return self
end

actions_factory.seq = seq_action  
----------------------------------------------------------------------------
----
local table_insert = table.insert
local table_size = table.getn
 
local function ActionManager(name)
  -- private
  local m_name = "LuaActionManager-"..name
  local scheduler = Scheduler()   
  
  local ref = ffi.new(clsObject.ct)
  C.sen_object_init(ref, name, nil)  
  -- public
  local _ = { ref = ref }
  
  local actions = {}
  local running = false
  
  function _.run(target, config, tag)
    local action = actions_factory.__create(target, config )
    if action then
      if tag then action.tag = tag end
      table_insert( actions, action )
      if not running then 
        scheduler.Resume(_)
        running = true
      end
    end     
  end 
  
  --local remove_actions = {} 


    
  function _.update(self, dt, key)
    local i = 1
    local len = table.getn(actions) 
    while i <= len do
      local v = actions[i]
      if v.removed then  
       --  print('---------------------', table.getn(actions))
         table.remove(actions, i)
        -- print('---------------------', table.getn(actions))
        -- _.dump()
      else
        if (v.first) then
          v.onStart()
          v.first = false
        end
        v.step(dt)
        if v.removed or v.isDone()  then
          if (v.removed == nil) then 
            v.onEnd() 
          else
           -- print('222222222222222222222222222222222222222222222222')  
          end
          table.remove(actions, i)
        else
          i = i+1  
        end
      end        
      len = table.getn(actions)
    end
    
    if (table_size(actions) == 0 and running ) then 
      scheduler.Pause(_) 
      running = false
    end     
    return 0
  end

  function _.stop(target, tag)
    for i, v in ipairs(actions) do
      if (target==nil or v.target == target)  and (tag==nil or tag==v.tag) then
        v.removed = true
      end      
    end
  end
  
--[[
  function _.dump()
    for i, v in ipairs(actions) do
       print(inspect(v))
    end
  end
  --]]
  
  function _.is_running(target, action_tag)
    local found = false
    for i, v in ipairs(actions) do
      if v.removed==nil and 
         (target==nil or v.target == target) and 
         (action_tag==nil or v.tag == action_tag)
      then
        found = true
        break
      end      
    end
    return found
  end
  
  function _.wait(target, action_tag)
    while _.is_running(target,action_tag) do
      coroutine.yield()
    end
  end
    
   scheduler.Schedule(_, _.update, m_name, -1, -1, -1, 1)
                    
  return _;
end


return ActionManager("main")