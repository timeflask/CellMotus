local sen           = require "sen.core"
local base_cell     = require "game.cell"
local base_board    = require "game.board"
local level         = require "game.level"

local rand = math.random
local ripairs = sen.ripairs

local function get_random_ij(level, imin, imax, jmin, jmax)
  local found = false
  local i, j
  while not found do
    i = rand(imin, imax)
    j = rand(jmin, jmax)
    found = true
    for k,v in ipairs(level) do
      if v.i == i and v.j == j then
        found = false 
        break
      end
    end
  end
  return i,j
end

local function in_range(n, nmin, nmax)
  return (n<=nmax and n>= nmin)
end

local function in_conf_range(conf, i, j)
  return in_range(i, conf.imin, conf.imax) and
         in_range(j + (i%2), conf.jmin, conf.jmax)
end

local function get_random_pos(conf)
  local i, j
  repeat
    i, j = rand(conf.imin, conf.imax), rand(conf.jmin, conf.jmax)
  until in_conf_range(conf,i,j)
  return i,j  
end

local function dir_reflect(dir)
  local ret = (dir+3)%6
  return ret > 0 and ret or 6
end

local function create_starter_item(i, j, index)
  return {
    i=i, j=j, t="item", c=index, d=1,
  }
end

local function create_starter_items(level, conf)
  local items_num = rand(2, conf and conf.max_items or 4 )
  for i=1,items_num do
    local c,r = get_random_ij(level, 
                      conf and conf.imin or -1, 
                      conf and conf.imax or 1, 
                      conf and conf.jmin or -1, 
                      conf and conf.jmax or 1 )
    table.insert(level, create_starter_item(c,r,i));
  end
end

local board_map = 
{
 {-1,  0, -1},
 { 0, -1, -1},
 { 1,  0, -1},
 { 1,  1,  0},
 { 0,  1,  1},
 {-1,  1,  0},
}

local path = {}
local meta = {}

local function get_ij(index, col, row)
 
  if index == nil or index <= 0 then
    index = math.random(6)
  elseif index > 6 then 
    index = 6  
  end
  
  local shift = board_map[index]

  local i = col + shift[1]
  local j = row + ((col%2)==1 and shift[2] or shift[3]) 
  
  return i, j
end


local function get_step_map(conf, i, j)
  local ret = {}
  for _=1,6 do
    local ii,jj= get_ij(_, i, j)
    if in_conf_range(conf, ii, jj) then
      table.insert(ret, {d=_,i=ii,j=jj})
    end
  end
  return ret
end

local function delete_edge_steps(conf, item)
  for _,v,remove in ripairs(item.steps) do
    if v.d ~= item.d then
       
       local ii, jj = get_ij(dir_reflect(v.d), item.i, item.j)
       if not in_conf_range(conf, ii, jj) then
         print('[* move] DELETE   ',v.d, ii, jj)
         remove()
       end
    end
  end
end

local WEIGHT_SLIDE = 1
local WEIGHT_MOVE  = 2

local function build_moves_map(conf, item)
  if item.i == nil then
    item.i, item.j = get_random_pos(conf)
  end
  
  item.steps = {}
  for d=1,6 do
    local ii,jj= get_ij(d, item.i, item.j)
    if in_conf_range(conf, ii, jj) then
      table.insert(item.steps, {i=ii,j=jj})
    end
  end
  
  if (item.d == nil) then
    local bad_directions = {}
    for _,s in ipairs(item.steps) do
      for d=1,6 do
        if bad_directions[d] == nil then
          local ii,jj = get_ij(d, s.i, s.j)
          if not( in_conf_range(conf, ii, jj)) then
            bad_directions[d] = d
          else 
            local i,j = get_ij(d, item.i, item.j)
            if not( in_conf_range(conf, i, j)) then
              bad_directions[d] = d
            end
          end
        end  
      end
    end
    repeat
      item.d = rand(6)
    until bad_directions[item.d] == nil  
  end
  
  
end

local function build_step_map(conf, item)
  delete_edge_steps(conf, item)
  
  for _,v in ipairs(item.steps) do
    v.w = v.d ~= item.d and WEIGHT_MOVE or WEIGHT_SLIDE
    for k=1,6 do
      local aitem = {a=k, d=k, i=v.i, j=v.j}
      table.insert(v, aitem)
    end 
    if v.d ~= item.d then
    end
  end
  
end

local function build_steps(conf, item)
end

local function init_item(conf)
--[[
 local c,r = get_random_ij(level, conf.imin, conf.imax, 
                                  conf.jmin, conf.jmax )
                                  
 local item = {
    i=c, j=r, t="item", c=rand(4),
  }
  
  item.steps = get_step_map(conf, c, r)
  
  item.d = item.steps[rand(#item.steps)].d
  
  build_step_map(conf, item)
  ]]
 local item = {
    t="item", c=rand(4),
  }
  
  build_moves_map(conf, item)
  return item
end

local function create_level(conf)
  local level = {}
  local conf = conf or {}
  
  meta = {}
  
  conf.imin = conf.imin or -3 
  conf.imax = conf.imax or 3 
  conf.jmin = conf.jmin or -2 
  conf.jmax = conf.jmax or 2 
  
  
  local item = init_item(conf)
  
  level[1] = item
  
  print (sen.inspect(level))
  return level  
end

return create_level