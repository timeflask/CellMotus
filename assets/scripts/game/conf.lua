local sen           = require "sen.core"
local audioPlayer   = sen.AudioPlayer

local rand = math.random
local pow = math.pow

local size_table = {
  ["small"] = 48,
  ["normal"] = 72,
  ["large"] = 96,
  ["xlarge"] = 128,
}

local screen_size = sen.screenSizeName() 
local mat_total = 0

local function bounce_time(dt)
  local T    
  if (dt < 1 / 2.75) then
      T = 7.5625 * dt * dt;
  elseif (dt < 2 / 2.75) then
      dt = dt - 1.5 / 2.75
      T=  7.5625 * dt * dt + 0.75
  elseif(dt < 2.5 / 2.75) then
      dt = dt - 2.25 / 2.75
      T = 7.5625 * dt * dt + 0.9375
  else
    dt = dt - 2.625 / 2.75
    T = 7.5625 * dt * dt + 0.984375
  end
  return T
end
    
local _config = {}
local config = {
 scr_size = screen_size,
 
 image_size = function()
   return (size_table[screen_size] or 48 )
 end,


 image = function(img)
   --return img .. (size_table[screen_size] or 48 )
   return img .. tostring(128)
 end,
 
 cell_color_bg = {1,1,1,0.05},
  
 cell_colors  = { 
   {0x9b,0x59,0xb6,1},
   {0x34,0x98,0xdb,1},
   {0x2e,0xcc,0x71,1},
   {0xe6,0x7e,0x22,1},
   {0xF1,0xC4,0x0F,1},
   {0xC0,0x39,0x2B,1},
--   {0xA7,0x9A,0x92,1},
   {0x62,0x48,0x57,1},
   {0x40,0x34,0x96,1},
   {0xEC,0xF0,0xF1,1},
 },
 
 scroll_block_color = {0xDD,0x68,0x5F,1},
 
 swapper_color = {1,1,1,0.7},
 
 slevels_bg_color = { 0x15, 0x11, 0x17, 0x00 },
 sgame_bg_color = { 0x15, 0x11, 0x17, 0x00 },
 smenu_bg_color = { 0x15, 0x11, 0x17, 0x00 },
 
 matrix_colors  = {
  --{0xA1,0xD3,0xEF,1},
  {1,1,1,0},
  {0xD1,0x5E,0x61,0},
  {0x48,0xB6,0x6D,0},
 },
 
-- lvl_finished_bg = {0x2C,0xA0,0xB5, 0.3},
 lvl_finished_bg = {0x34,0x98,0xdb,0.4},
 lvl_curr_bg = {0x9b,0x59,0xb6,0.8},
-- lvl_curr_bg = {0x6e,0x1c,0x86,0.5},
-- lvl_progress_bg = {0xDD,0x68,0x5F,0.5},

 lvl_progress_bg = {0x2e,0xcc,0x71,0.8},
 
 lvl_locked_bg = {1,1,1,0.012},
 
 lvl_colors = {
   {0xFE,0x33,0x1F,1},
   --{0x45,0xB7,0x48,1},
   {0x33,0x9B,0xF3,1},
   {0x20,0xAA,0x00,1},
   {0x36,0x9F,0x59,1},
 },

 btime = bounce_time,

 atLabel_colors = {
   {0xFE,0x33,0x1F,1},
   --{0x45,0xB7,0x48,1},
   {0x33,0x9B,0xF3,1},
   {0x20,0xAA,0x00,1},
   {0x36,0x9F,0x59,1},
 },

 
  effect_fadeIn = function(speed, rate, alpha_max, on_end) 
  return {
    name = "speed",
    speed = speed or 1, 
    tag = 'fadeIn',
    --on_end = on_end,
    action = {
      name = "interval",
      on_end = on_end,
      duration = 1,
      rate = rate or (1/rand(3)),
      amax = alpha_max or 1,
      col = nil,
      trigger = function(self, dt, conf)
        local f = conf.amax*pow(dt, conf.rate)
        if self.getBlend() == 4 then
          self.setColor( conf.col.r*f,conf.col.g*f,conf.col.b*f,conf.col.a*f)
        else
          self.setColor( {a=f})
        end
      end,
      end_trigger=function(self, conf)
        if conf.on_end then
          conf.on_end(self)
        end
        self.bcol = conf.col
      end,
      start_trigger = function(self, conf)
        conf.col = self.bcol or self.color()
        if conf.amax > 0.999 then
          conf.amax = conf.col.a
        end  

        if self.getBlend() == 4 then
          self.setColor( 0,0,0,0)
        else
          self.setColor({a=0})
        end
      end      
    },

  }
  end,

  effect_fadeOut = function(speed, rate, alpha_start, on_end) 
  return {
    name = "speed",
    speed = speed or 1, 
    tag = 'fadeOut',
    action = {
      name = "interval",
      duration = 1,
      on_end = on_end,
      rate = rate or (1/rand(3)),
      astart = alpha_start or 1, 
      col = nil,
      trigger = function(self, dt, conf)
        local f = conf.astart*(1-pow(dt, conf.rate))
        if self.getBlend() == 4 then
          self.setColor( conf.col.r*f,conf.col.g*f,conf.col.b*f,conf.col.a*f)
        else
          self.setColor( {a=f})
        end
      end,
      start_trigger = function(self, conf)
        conf.col = self.bcol or self.color()
        self.setColor({a=conf.astart})
      end,
      end_trigger = function(self, conf)
        if conf.on_end then
          conf.on_end(self)  
        end  
        self.bcol = conf.col
      end
    },
  }
  end,
  
  effect_fadeTransition = function(speed, rate, toAlpha, on_end) 
  return {
    name = "speed",
    speed = speed or 1, 
    tag = 'fadeTransition',
    action = {
      name = "interval",
      duration = 1,
      on_end = on_end,
      toA = toAlpha or 0,
      deltaA = 0, 
      startA = 0,
      rate = rate or (1/rand(3)),
      trigger = function(self, dt, conf)
        self.setColor( {
          a = conf.startA + conf.deltaA*(pow(dt, conf.rate)) 
          })
      end,
      start_trigger = function(self, conf)
        conf.startA = self.color().a
        conf.deltaA = conf.toA - conf.startA
      end,
      end_trigger = function(self, conf)
        if conf.on_end then
          conf.on_end(self)  
        end  
      end
    },
  }
  end,  
  
 effect_colorTransition = function (col, speed, rate, on_end)
  return 
  {
    name = "speed",
    speed = speed or (rand()+0.5), 
    action = {
      name = "interval",
      duration = 1,
      cd = {},
      cs = {},
      ci = col or {1,1,1,1},
      deltaA = 0, 
      startA = 0,
      rate =  rate or (1/rand(2)),
      trigger = function(self, dt, conf)
        local pdt = pow(dt, conf.rate)
        self.setColor(
          conf.cs.r - pdt*conf.cd.r, 
          conf.cs.g - pdt*conf.cd.g, 
          conf.cs.b - pdt*conf.cd.b, 
          conf.cs.a - pdt*conf.cd.a 
        )
      end,
      start_trigger = function(self, conf)
        local cs = self.color() 
        conf.cd.r = cs.r - conf.ci[1] 
        conf.cd.g = cs.g - conf.ci[2] 
        conf.cd.b = cs.b - conf.ci[3] 
        conf.cd.a = cs.a - conf.ci[4]
        conf.cs = cs
      end,
      end_trigger = function(self, conf)
        if on_end then
          on_end(self)
        end
      end
    },
  }    
 end,

 effect_Matrix = function(speed, dir, update_state, type)
 return {
       name = "speed",
       speed = speed or 4,
       
       end_trigger = function (self, conf)
         local cell = self.parent
         if (cell) then
           cell:removeSprites("@matrix")
         end 
       end,           
        
       action = {
         name = "seq",
         actions = {
           {       
             name = "interval",
             duration = rand(),
             rate = 2,
             amax = 0.1,
             color_type = type or 1,
             update = update_state or false,
             dir = dir or 2,
             trigger = function(self, dt, conf)
                self.setColor({a=pow(dt, conf.rate)*conf.amax})
             end,
             
             start_trigger = function (self, conf)
               self.setColor(_config.matrix_colors[conf.color_type])
               local cell = self.parent
               if cell and cell.state then
                 conf.amax = 0.8
               end 
             end,

             end_trigger = function (self, conf)
               local this = self.parent
               if this ~= nil then
                 local dir = (conf.dir == 4 or conf.dir == 3) and conf.dir-(this.col)%2 or
                             (conf.dir == 6 and  conf.dir+(this.col)%2) or
                             conf.dir               
                 local cell = this:get_neighbor(dir)
                 if (conf.update) then
                   this:setEmpty()
                   this:updateGameState()
                 end  
                 if (cell ~= nil and conf.dir < 7) then
                   cell:runMatrixEffect(conf.dir, conf.update, conf.color_type)
                 end
               end  
             end
           },
           {       
              name = "interval",
              duration = 1.5*rand(),
              rate = 1/(2*rand()),
              amax = 0.1,
              trigger = function(self, dt, conf)
                self.setColor({a=(1-pow(dt, conf.rate))*conf.amax})
              end,
              start_trigger = function(self, conf)
               local cell = self.parent
               if cell and cell.state then
                 conf.amax = 0.8
               end 
              end
           },
         }
       }
  }
  end,

 effect_blink = function(speed, amax, forever, on_end)
     return {
       name = "speed",
       tag = "blink",
       speed = speed or 4, 
       action = {
         name = "seq",
         forever = (forever==nil) or forever,
         actions = {
         _config.effect_fadeIn(speed, 1/3, amax),
         _config.effect_fadeOut(speed, 2, amax, on_end),
         }
       }
     }
 end,
 
 effect_slide_to = function(from, to, speed, rate) 
     return {
      name = "speed",
      speed = speed or 1, 
      action = {
        name = "interval",
        duration = 1,
        startX = from.x,
        startY = from.y,
        deltaX = to.x - from.x,
        deltaY = to.y - from.y,
        rate = rate or 2,
        trigger = function(self, dt, conf)
         --local dx = pow(dt, conf.rate)
          local dx = bounce_time(pow(dt, 1/2)) 
          self.moveTo(conf.startX+conf.deltaX*dx, 
                      conf.startY+conf.deltaY*dx)
                        
        end,
        start_trigger = function(self, conf)
          if self then
            self.moveTo( conf.startX , conf.startY )
            self.ZOrder(self.posZ()-0.001)
          end  
        end,
        end_trigger = function(self, conf)
          if self then
            self.ZOrder(self.posZ()+0.001)
          end  
        end        
        
     }
     }
 end,
 
 effect_moveTo = function(x,y, speed, rate) 
     return {
      name = "speed",
      speed = speed or 1, 
      action = {
        name = "interval",
        duration = 1,
        --startX = 0,
        --startY = 0,
        --deltaX = to.x - from.x,
        --deltaY = to.y - from.y,
        rate = rate or 2,
        trigger = function(self, dt, conf)
          local dx = pow(dt, conf.rate) 
          self.moveTo(conf.startX+conf.deltaX*dx, 
                      conf.startY+conf.deltaY*dx)
        end,
        start_trigger = function(self, conf)
          conf.startX = self.posX()
          conf.startY = self.posY()
          conf.deltaX = x - conf.startX
          conf.deltaY = y - conf.startY
        end
        
     }
     }
 end, 
 
 effect_scaleTo = function(x,y, speed, rate, on_end) 
     return {
      name = "speed",
      speed = speed or 1, 
      action = {
        name = "interval",
        duration = 1,
        --startX = 0,
        --startY = 0,
        --deltaX = to.x - from.x,
        --deltaY = to.y - from.y,
        rate = rate or 2,
        trigger = function(self, dt, conf)
          local dx = pow(dt, conf.rate) 
          self.scale(conf.startX+conf.deltaX*dx, 
                     conf.startY+conf.deltaY*dx)
        end,
        start_trigger = function(self, conf)
          conf.startX = self.scaleX()
          conf.startY = self.scaleY()
          conf.deltaX = x - conf.startX
          conf.deltaY = y - conf.startY
        end,
        end_trigger = function(self,conf)
          if on_end then
            on_end(self)
          end  
        end
        
     }
     }
 end,  
 
 effect_dir_bounce = function(cell, dir, dist)
  -- print("DIST "..(dist or 'nil'))
   local dir_cell = cell:get_neighbor(dir)
     return {
       name = "speed",
       start_trigger=function()
         audioPlayer.playSound("impact0.wav")
       end,
       speed = 4, 
       action = {
         name = "seq",
         actions = {
           {       
              name = "interval",
              duration = 1,
              startX = cell.x,
              startY = cell.y,
              deltaX = (dir_cell.x - cell.x)*(dist and 1/6*dist or 1/3),
              deltaY = (dir_cell.y - cell.y)*(dist and 1/6*dist or 1/3),
              rate = 1/7,
              trigger = function(self, dt, conf)
                --math.pow(dt, conf.rate)
                local dx = bounce_time(dt)
                self.moveTo(conf.startX+conf.deltaX*dx, 
                            conf.startY+conf.deltaY*dx)
              end,
           },
           {       
              name = "interval",
              duration = 1,
              startX = cell.x,
              startY = cell.y,
              deltaX = (dir_cell.x - cell.x)*(dist and 1/6*dist or 1/3),
              deltaY = (dir_cell.y - cell.y)*(dist and 1/6*dist or 1/3),
              rate =  2,
              trigger = function(self, dt, conf)
               --math.pow((1-dt), conf.rate)
                local dx = math.pow((1-dt), conf.rate) --bounce_time(1-dt)
                self.moveTo(conf.startX+conf.deltaX*dx, 
                            conf.startY+conf.deltaY*dx)
              end,
           },
         }
       }
     }
 end, 
 
 effect_pinned = function()
     return {
       name = "speed",
       speed = 1, 
       action = {
         name = "seq",
         forever = true,
         actions = {
           {       
              name = "interval",
              duration = 1,
              rate = 1/3,
              trigger = function(self, dt, conf)
                self.setColor({a=math.pow(dt, conf.rate) /5})
              end,
           },
           {       
              name = "interval",
              duration = 1,
              rate = 3,
              trigger = function(self, dt, conf)
                self.setColor({a=(1-math.pow(dt, conf.rate))/5})
              end,
           },
         }
       }
     }
 end, 
 
 effect_fadeIn1 = function(a)
 return {
       name = "speed",
       speed = 1, 
       action = {
          name = "interval",
          duration = 1,
          rate = 1/math.random(4),
          topAlpha = a,
          trigger = function(self, dt, conf)
             self.setColor({a= conf.topAlpha*math.pow(dt, conf.rate)})
          end
       }
     }
 end,

  effect_fade_in = function(speed, rate) 
  return {
    name = "speed",
    speed = speed or 1.5, 
    action = {
      name = "interval",
      duration = 1,
      rate = rate or 1/5,
      trigger = function(self, dt, conf)
        self.setColor( {a= math.pow(dt,conf.rate)} )
      end,
      start_trigger = function(self)
        self.setColor({a=0})
      end
  }}
  end,
 
 effect_matrix = function(state)
 return {
       name = "speed",
       speed = 4,
       
        end_trigger = function (self, conf)
          local cell = self.cell 
          cell.board.node.delChild(self)  
        end,           
        
       action = {
         name = "seq",
         actions = {
           {       
              name = "interval",
              duration = 0.3+math.random(),
              rate = 2,
              maxAlpha = state and 1 or 1/5,
              trigger = function(self, dt, conf)
                self.setColor({a=math.pow(dt, conf.rate)*conf.maxAlpha})
              end,
              end_trigger = function (self, conf)
                local this = self.cell
                if this ~= nil then
                  local cell = this:get_neighbor(2)
                  this:setEmpty()
                  this:updateGameState()
                  if (cell ~= nil) then
                    cell:runMatrixEffect()
                  end
                end  
                
              end
           },
           {       
              name = "interval",
              duration = 1.5+math.random(),
              rate = 1/4,
              maxAlpha = state and 1 or 1/5,
              trigger = function(self, dt, conf)
                self.setColor({a=(1-math.pow(dt, conf.rate))*conf.maxAlpha})
              end,
              start_trigger = function(self, conf)
                if self.cell.state ~= nil then
                  conf.maxAlpha = 1
                end
              end,
              end_trigger = function(self, conf)
                if self.cell ~= nil then
                  self.cell.locked = false
                end
              end,
              
           },
         }
         
       }
  }
  end,
   
 
}


_config = config


return config