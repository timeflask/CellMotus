local ffi = require "ffi"

local sen = require "sen.core"
local base_cell = require "game.cell"
class "base_board"

local floor = math.floor
local ceil = math.ceil

local index_shift_map = 
{
 {-1,  0},
 { 0, -1},
 { 1,  0},
 { 1,  1},
 { 0,  1},
 {-1,  1},
}

local BORDER_ITEMS = 1

function base_board:touchToCell(td, start)
 local point = start and td.start or td.point
 local n = self.node
 local i,j = self:cell_index((point.x - n.posX())/n.scaleX(), 
                             (point.y  -n.posY())/n.scaleY() )
 return self.cells[i][j]                            
end

function base_board:visible(x,y)
  local sw =  BORDER_ITEMS*self.wcell 
  local sh =  BORDER_ITEMS*self.hcell
  local bbox = self.scene_bbox
  return   x > bbox.l - sw and 
           x < bbox.r + sw and 
           y > bbox.b - sh and 
           y < bbox.t + sh
                  
end

function base_board:cell_visible(c)
  return self:visible(c.x, c.y)
end

function base_board:cell_full_visible(c, eps)
  return self:xy_full_visible(c.x, c.y, eps)
end

function base_board:xy_full_visible(x, y, eps)
  local sw =  self.wcell/(2+(eps or 0.1)) 
  local sh =  self.hcell/(2+(eps or 0.1)) 
  local bbox = self.scene_bbox
  --print (bbox.b/self.scale, y, sh, y > bbox.b + sh )
  return   x > bbox.l + sw and 
           x < bbox.r - sw and 
           y > bbox.b + sh and 
           y < bbox.t - sh
end

function base_board:cell_coords(i,j)
  local sw =  self.wcell 
  local sh =  self.hcell
  local x, y = i*sw*self.xpad, (i%2)*sh/2 + j*sh*self.ypad
  return x, y, self:visible(x,y) 
end

function base_board:dist2(i,j,x,y)
  local cell = self.cells[i][j]
  if (cell) then return (x-cell.x)^2 + (y-cell.y)^2 else return 10000000000 end
end

function base_board:cell_index(x,y)
  local sw =  self.wcell 
  local sh =  self.hcell
  
  local i = floor(x/sw/self.xpad)   
  local j = floor((y - (i%2)*sh/2)/sh/self.ypad) 
  local dist2 =  self:dist2(i,j,x,y)
  local best_i,best_j = i,j
  local si,sj

  
  for k,v in ipairs(index_shift_map) do
    local cs = v
    si,sj = i+cs[1],j+cs[2]
    local ndist2 = self:dist2(si,sj,x,y)
    if (ndist2 < dist2) then
      dist2 = ndist2
      best_i,best_j = si,sj
    end   
  end   
  
  return best_i, best_j 
end

function base_board:fill_cells(i,j)
  local cells = self.cells
  local cell_class = self.cell_class
  
  
  if  cells[i][j] == nil then 
    local x,y, visible = self:cell_coords(i,j)
    if visible then  
      cells[i][j] = cell_class( x, y, i, j, self )
      self.ncells = self.ncells + 1
      if i < self.min_i then self.min_i = i elseif i > self.max_i then self.max_i = i end  
      if j < self.min_j then self.min_j = j elseif j > self.max_j then self.max_j = j end  
    end
      
  else
  end
  
  for k=1,6 do
    local ii, jj = base_cell.get_ij(k, i, j)
    if (cells[ii][jj] == nil) then 
      local x,y, visible = self:cell_coords(ii,jj)
      if (visible) then
        self:fill_cells(ii, jj, cell_class )
      end
    end     
  end
    
end

function base_board:del_cells(i,j)
  local cells = self.cells
  local cell_class = self.cell_class
  
  
  if  cells[i][j] ~= nil then 
    local x,y, visible = self:cell_coords(i,j)
    if not visible then
      cells[i][j]:clean()  
      cells[i][j] = nil
      self.ncells = self.ncells - 1
    end  
  end
  
  for k=1,#index_shift_map do
    local coords = index_shift_map[k]
    local ii = i + coords[1]
    local jj = j + coords[2]
    if (cells[ii][jj] ~= nil) then 
      local x,y, visible = self:cell_coords(ii,jj)
      if (not visible) then
        self:del_cells(ii, jj, cell_class )
      end
    end     
  end
end


function base_board:base_board(scene, name, cell_class, cell_image, xpad, ypad, scale_cell)
  self.node = sen.clsNode(name)
  --self.node.rotate(20)
  self.node.board = self
  self.xpad = xpad or 0.833
  self.ypad = ypad or 1.05
  self.scale = scale_cell or 1
  --self.node.scale(self.scale,self.scale)
  self.cell_image = cell_image
  self.cell_class = cell_class
  
  local sprite_bbox = sen.clsSprite.imageBBox(cell_image)
  self.wcell = (sprite_bbox.right - sprite_bbox.left) * self.scale
  self.hcell = (sprite_bbox.top - sprite_bbox.bottom) * self.scale

  self.cells = sen.auto_table(2)
  local cells = self.cells 
  
  self.scene_bbox = scene.getBBox()
  
  local sw =  self.wcell 
  local sh =  self.hcell

  self.min_i = 0
  self.min_j = 0
  self.max_i = 0
  self.max_j = 0
  self.ncells = 0
  self:fill_cells(0,0)  
  
  scene.addChild(self.node)
end


function base_board:trim_invis(v)
  local sw =  BORDER_ITEMS*self.wcell 
  local sh =  BORDER_ITEMS*self.hcell
  local bbox = self.scene_bbox
  
  if v.x < bbox.l - sw and v.col >= self.min_i then
    self.min_i = v.col+1
   -- print(self.min_i)
  elseif v.x > bbox.r  + sw  and v.col <= self.max_i then
    self.max_i = v.col-1
  elseif v.y < bbox.b - sh and v.row >= self.min_j then
    self.min_j = v.row + 1
  elseif v.y > bbox.t  + sh  and v.row <= self.max_j then
    self.max_j = v.row - 1
  end
end

function base_board:trim_indexes()
  local cells = self.cells
  for i,vi in pairs(cells) do
    for j, v in pairs(cells[i]) do
      local visible = self:visible(v.x, v.y)
      if visible then
        if v.col > self.max_i then self.max_i = v.col elseif v.col < self.min_i then self.min_i = v.col end
        if v.row > self.max_j then self.max_j = v.row elseif v.row < self.min_j then self.min_j = v.row end
      else
        self:trim_invis(v)
      end              
    end
  end  
end

function base_board:trim_cells()
 for i,col in pairs(self.cells) do
   for j,v in pairs(col) do
      if  v ~= nil then 
        local x,y, visible = self:cell_coords(i,j)
        if not visible then
          self.cells[i][j]:clean()  
          self.cells[i][j] = nil
          self.ncells = self.ncells - 1
        end  
      end
   end
 end     
end

function base_board:refill_cells()
  for i=self.min_i,self.max_i do
     self:fill_cells(i,self.min_j-BORDER_ITEMS)
     self:fill_cells(i,self.max_j+BORDER_ITEMS)
  end
  for j=self.min_j,self.max_j do
     self:fill_cells(self.min_i-BORDER_ITEMS,j)
     self:fill_cells(self.max_i+BORDER_ITEMS,j)
  end
end

function base_board:_resize(b)
  self.scene_bbox = b
  self:trim_indexes()
 -- self:refill_cells()
end

function base_board:resize(b)
  self:_resize(b)
  
end


local dir_angle_ranges = {
  {300,360},
  {240,300},
  {180,240},
  {120,180},
  {60 ,120},
  {0  ,60},
}

local function in_range(a, l, r)
  return (a>l and a <r)
end

local function swipe2angle(td)
  local r = math.atan2(td.point.y-td.start.y,td.point.x-td.start.x)
  --local r = math.atan2(td.point.y-td.prev.y,td.point.x-td.prev.x)
  return 180*(1-r/math.pi)
end

function  base_board:swipe2dir(td)
  local a = swipe2angle(td)
  if in_range(a,0,60) then
    return 6
  elseif in_range(a,60,120) then
    return 5
  elseif in_range(a,120,180) then
    return 4
  elseif in_range(a,180,240) then
    return 3
  elseif in_range(a,240,300) then
    return 2
  elseif in_range(a,300,360) then
    return 1
  else  
    return 2
  end  
end

function  base_board:swipe_range(td, l, ri)
  return in_range(swipe2angle(td),l,ri)
end

function  base_board:swipe_range_dir(td, dir, e)
  local epsilon = e or 20
  local a = swipe2angle(td)
  local r = dir_angle_ranges[dir]
  return in_range(a, r[1]-epsilon, r[2]+epsilon )
end

function  base_board:is_swipe(td, dt, dx)
--[[
  return  math.sqrt ( (td.point.x - td.start.x)^2+ 
                      (td.point.y - td.start.y)^2 ) >= (dx or self.wcell / 3) and
          td.point.z <= (dt or 1)
          --]]
  if dx == nil then          
    local bbox = sen.vp_box()
    dx = math.min( bbox.r-bbox.l, bbox.t-bbox.b) / 15  
  end   
                    
  return  td.point.w >= (dx ) and
          td.point.z <= (dt or 2)
            
end

function  base_board:is_tap(td,dt,dx)
--[[
  return  math.sqrt ( (td.point.x - td.start.x)^2+ 
                      (td.point.y - td.start.y)^2 ) <= (dx or self.wcell / 8) and
          td.point.z <= (dt or 0.4)
          --]]
  if dx == nil then          
    local bbox = sen.vp_box()
    dx = math.min( bbox.r-bbox.l, bbox.t-bbox.b) / 20  
  end   
          
  return  td.point.w <= dx and
          td.point.z <= (dt or 1)
            
end

function base_board:is_swipe_down(td)
  return self:swipe_range(td,210,330)
end

function base_board:is_swipe_right(td)
  return self:swipe_range(td,150,210)
end

function base_board:is_swipe_up(td)
  return self:swipe_range(td,30,150)
end

function base_board:is_swipe_left(td)
  return self:swipe_range(td,0,30) or
         self:swipe_range(td,330,360)   
end

return base_board

