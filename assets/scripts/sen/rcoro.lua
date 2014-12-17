local yield = coroutine.yield
local coro_create = coroutine.create
local resume = coroutine.resume
local running = coroutine.running

local CoFunc = function(func)
  local res = nil
  while true do 
    res = func(yield(res)) 
  end
end 
      
local Running = {}
local Wrappers = {}
local u_Reset = Running

local CoCreate = function (func, on_reset)
  local on_reset = on_reset or nil
  local coro, in_reset
  local function Resume(success, res_or_error, ...)
    Running[coro] = nil
    if res_or_error == u_Reset then
      in_reset = true
      coro = nil
      success, res_or_error = pcall(on_reset, ...)
      in_reset = false
    end
    if not success then
      error(res_or_error, 3)
    elseif coro then
      return res_or_error, ...
    end
  end
  local function wrapper(arg_or_reset, ...)
    if coro == nil or arg_or_reset == u_Reset then
      coro = coro_create(CoFunc)
      resume(coro, func)
      if arg_or_reset == u_Reset then
        return Resume(true, u_Reset, ...)
      end
    end
    
    return Resume( resume(coro, arg_or_reset, ...) )
  end
  Wrappers[wrapper] = true
  return wrapper
end  
     
local function coReset (coro, ...)
  local running_coro = Running[running() or 0]
  if coro and coro ~= running_coro then
    coro(u_Reset, ...)
  else
    yield(u_Reset, ...)
  end
end     

local r_coro = {}

function r_coro.new(mainFunc, onResetFunc)
  local coro = CoCreate(mainFunc, onResetFunc)
  local _ = {}
  function _.Reset(...)
    coReset(coro, ...)
  end
  function _.Update(node, dt)
    return coro(node, dt)
  end
  return _
end

setmetatable(r_coro, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})
     
return r_coro