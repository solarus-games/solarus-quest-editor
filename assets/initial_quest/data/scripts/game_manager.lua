local game_manager = {}

-- Sets initial values to a new savegame file.
local function initialize_new_savegame(game)

  -- You can modify this function to initialize life and equipement here.
  game:set_max_life(12)
  game:set_life(game:get_max_life())
  game:set_ability("lift", 1)
  game:set_ability("sword", 1)
end

-- Starts the game from savegame save1.dat,
-- initializing it if necessary.
function game_manager:start_game()

  local exists = sol.game.exists("save1.dat")
  local game = sol.game.load("save1.dat")
  if not exists then
    -- Initialize a new savegame.
    initialize_new_savegame(game)
  end
  game:start()

  -- Use Eldran's sprite for the hero.
  local hero = game:get_hero()
  hero:set_tunic_sprite_id("main_heroes/eldran")
end

return game_manager

