local sen = require "sen.core"
local actionManager = sen.ActionManager
class "base_cell"

local index_shift_map = 
{
 {-1,  0, -1},
 { 0, -1, -1},
 { 1,  0, -1},
 { 1,  1,  0},
 { 0,  1,  1},
 {-1,  1,  0},
}

function base_cell:base_cell(x, y, col, row, board)
  self.x  = x
  self.y  = y
  self.board = board
  self.col   = col
  self.row   = row
  
  
  self.sprite = sen.clsSprite(nil, board.cell_image)
  self.sprite.moveTo( x , y )
  if (self.board.scale ~= 1) then
   self.sprite.scale(self.board.scale, self.board.scale)
  end  
  --self.sprite.rotate(90)
  self.sprite.ZOrder(-0.5)
  --self.sprite.blend(3)
  board.node.addChild(self.sprite)
  
end

function base_cell:clean()
  self:on_clean()
  actionManager.stop(self.sprite)
  self.board.node.delChild(self.sprite)
  self.board = nil
end


function base_cell:box_min_dist()
  local bbox = self.board.scene_bbox
  local bminB = math.max ( bbox.l, bbox.b)
  local bminT = math.min ( bbox.r, bbox.t)
--  local xmin =  math.min ( bbox.r - self.x, self.x - bbox.l)
--  local ymin =  math.min ( bbox.t - self.y, self.y - bbox.b)
  local xmin =  math.min ( bminT - self.x, self.x - bminB)
  local ymin =  math.min ( bminT - self.y, self.y - bminB)
  return xmin,ymin
end

function base_cell.get_ij(index, col, row)
 
  if index == nil or index <= 0 then
    index = math.random(6)
  elseif index > 6 then 
    index = 1  
  end
  
  local shift = index_shift_map[index]

  local i = col + shift[1]
  local j = row + ((col%2)==1 and shift[2] or shift[3]) 
  
  return i, j
end

function base_cell:get_neighbor(index)
  
  local board = self.board
  if not board then return nil end
  local cells = board.cells

  local i, j = base_cell.get_ij(index, self.col, self.row)


  return cells[i][j]
end

function base_cell:get_dist(c)
  local dx = self.x - c.x
  local dy = self.y - c.y
  return math.sqrt( dy*dy + dx*dx  )  
end

function string.starts(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function base_cell:removeSprites(target)
  local node = self.board.node
  if self.sprites then
    if target~=nil then
     local c = self.sprites[target]
     if (c ~= nil) then 
       actionManager.stop(c)
       node.delChild(c)
       self.sprites[target]=nil
     end
    else    
      for i,v in pairs(self.sprites) do
        if not string.starts(i,'@')  then
          actionManager.stop(v)
          node.delChild(v)
          self.sprites[i]=nil
        end  
      end
    end
  end  
end

function base_cell:removeNodes(target)
 -- if not self.board then return end
  local node = self.board.node
  if self.nodes then
    if target~=nil then
     local c = self.nodes[target]
     if (c ~= nil) then 
       actionManager.stop(c)
       node.delChild(c)
      
       self.nodes[target]=nil
     end
    else    
      for i,v in pairs(self.nodes) do
        if not string.starts(i,'@')  then
         -- print('-------------------------- ', i)
          actionManager.stop(v)
          node.delChild(v)
          self.nodes[i]=nil
        end  
      end
    end
  end  
end
return base_cell