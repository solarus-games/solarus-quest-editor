-- This script allows to register multiple functions as Solarus events.
--
-- Usage:
--
-- Just require() this script and then all Solarus types
-- will have a register_event() method that adds an event callback.
--
-- Example:
--
-- local multi_events = require("scripts/multi_events")
--
-- -- Register two callbacks for the game:on_started() event:
-- game:register_event("on_started", my_function)
-- game:register_event("on_started", another_function)
--
-- -- It even works on metatables!
-- game_meta:register_event("on_started", my_function)
-- game_meta:register_event("on_started", another_function)
--
-- The good old way of defining an event still works
-- (but you cannot mix both approaches on the same object):
-- function game:on_started()
--   -- Some code.
-- end
--
-- Limitations:
--
-- Menus are regular Lua tables and not a proper Solarus type.
-- They can also support multiple events, but to do so you first have
-- to enable the feature explicitly like this:
-- multi_events:enable(my_menu)
-- Note that sol.main does not have this constraint even if it is also
-- a regular Lua table.

local multi_events = {}

local function register_event(object, event_name, callback)

  local previous_callbacks = object[event_name] or function() end
  object[event_name] = function(...)
    return previous_callbacks(...) or callback(...)
  end
end

-- Adds the multi event register_event() feature to an object
-- (userdata, userdata metatable or table).
function multi_events:enable(object)
  object.register_event = register_event
end

local types = {
  "game",
  "map",
  "item",
  "surface",
  "text_surface",
  "sprite",
  "timer",
  "movement",
  "straight_movement",
  "target_movement",
  "random_movement",
  "path_movement",
  "random_path_movement",
  "path_finding_movement",
  "circle_movement",
  "jump_movement",
  "pixel_movement",
  "hero",
  "dynamic_tile",
  "teletransporter",
  "destination",
  "pickable",
  "destructible",
  "carried_object",
  "chest",
  "shop_treasure",
  "enemy",
  "npc",
  "block",
  "jumper",
  "switch",
  "sensor",
  "separator",
  "wall",
  "crystal",
  "crystal_block",
  "stream",
  "door",
  "stairs",
  "bomb",
  "explosion",
  "fire",
  "arrow",
  "hookshot",
  "boomerang",
  "camera",
  "custom_entity"
}

-- Add the register_event function to all userdata types.
for _, type in ipairs(types) do

  local meta = sol.main.get_metatable(type)
  assert(meta ~= nil)
  multi_events:enable(meta)
end

-- Also add it to sol.main (which is a regular table).
multi_events:enable(sol.main)

return multi_events
