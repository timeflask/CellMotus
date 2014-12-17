
local ffi = require "ffi"
local C = ffi.C

local clsObject = require "sen.object"
local clsCoro = require "sen.rcoro"

ffi.cdef[[

  typedef int (*scheduler_update_callback)(void*, double, const char*);
  
  typedef struct scheduler_t {
    object_t super;
    float    scale;
    void     *nodes;
  }scheduler_t;
  
  void
  sen_scheduler_add(scheduler_t*              self,
                    object_t*                 obj,
                    scheduler_update_callback callback,
                    const char*               key,
                    double                    interval,
                    int                       repeat,
                    double                    delay,
                    int                       pause);
  
  void
  sen_scheduler_remove(scheduler_t*              self,
                       object_t*                 obj,
                       const char*               key);
  
  extern
  scheduler_t* sen_scheduler();
  
  void 
  sen_scheduler_pause(scheduler_t* self,
                      object_t* obj,
                      const char* key);
  
  void 
  sen_scheduler_resume(scheduler_t* self,
                       object_t* obj,
                       const char* key);  
                       
  int sen_scheduler_is_running(scheduler_t* self,
                              object_t* obj,
                              const char* key);                       

]]
 
 
local function SchedulerClosure()
  -- private
  
  local m_scheduler = C.sen_scheduler()
  
  local function pack_node(node, func)
    return {node = node, func=func}
  end

  local function pack_node_coro(node, coro)
    return {node = node, coro=coro}
  end
  
  local m_procs = {}
  local m_coros = {} 
  
  local sfunc_wrapper = ffi.cast("scheduler_update_callback", function(ref, dt, key)
    local key = ffi.string(key)
    
    local data = m_procs[key]
    local ret = 1
    if (data ~= nil) then
      ret = data.func(data.node, dt)
      if ret > 0 then
        m_procs[key] = nil
      end
    end  
    return ret
  end)
  
   
  
  local scoro_wrapper = ffi.cast("scheduler_update_callback", function(ref, dt, key)
    local key = ffi.string(key)
    local data = m_coros[key]
    local ret = 1
    if (data ~= nil) then
      ret =  data.coro.Update(data.node, dt)
      if ret and ret > 0 then
        m_coros[key] = nil
      end
    end  
    return ret==nil and 0 or ret
  end)

  -- public
  local _ = {}
  
  function _.Schedule(node, func, key, interval, repeatTimes, delay, pause)
    if (m_procs[key] ) then
      _.Remove(node,key)
    end
   
    m_procs[key] = pack_node(node, func)                         
    C.sen_scheduler_add(m_scheduler,
                        clsObject.cast( node.ref ),
                        sfunc_wrapper,
                        key or nil,
                        interval or -1,
                        repeatTimes or -1 ,
                        delay or -1,
                        pause or 0)
                          
  end
  
  function _.ScheduleCoro(node, coro, key, interval, repeatTimes, delay, pause)
    if (m_coros[key]) then
      _.Remove(node,key)
    end
    m_coros[key] = pack_node_coro(node, coro)                         
    C.sen_scheduler_add(m_scheduler,
                        clsObject.cast( node.ref ),
                        scoro_wrapper,
                        key or nil,
                        interval or -1,
                        repeatTimes or -1 ,
                        delay or -1,
                        pause or 0)
                          
  end

  function _.Remove(node, key)
    if m_coros[key] == nil and m_procs[key] == nil then
      return
    end
      
    C.sen_scheduler_remove(m_scheduler,
                           clsObject.cast(node.ref),
                           key or nil)
                           
    if (m_coros[key] ~= nil) then
      
      m_coros[key] = nil                       
    end
    if (m_procs[key] ~= nil) then
                             
      m_procs[key] = nil                       
    end
  end
                    
  function _.Pause(node, key)
    C.sen_scheduler_pause(m_scheduler,
                          clsObject.cast(node.ref),
                          key or nil)
  end
  
  function _.Resume(node, key)
    C.sen_scheduler_resume(m_scheduler,
                          clsObject.cast(node.ref),
                          key or nil)
  end

  function _.is_running(target, key)
    return C.sen_scheduler_is_running (m_scheduler, clsObject.cast(target.ref), key ) > 0
  end
  
  function _.Wait(node, key)
    while _.is_running(node, key) do
      coroutine.yield()
    end
  end
  
  return _;
end
 
return SchedulerClosure  