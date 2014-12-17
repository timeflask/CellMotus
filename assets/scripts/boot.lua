package.path = 'assets/scripts/?.lua;' .. package.path

math.randomseed( os.time() )

local game = require "game.main"


return game

