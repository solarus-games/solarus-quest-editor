-- Initialize hero behavior specific to this quest.

require("scripts/multi_events")

local hero_meta = sol.main.get_metatable("hero")

local function initialize_hero_features(game)

  local hero = game:get_hero()
  hero:set_tunic_sprite_id("main_heroes/eldran")
end

-- Set up Eldran hero sprite on any game that starts.
local game_meta = sol.main.get_metatable("game")
game_meta:register_event("on_started", initialize_hero_features)
return true
