-- This script updates all data files of a solarus 1.2 quest
-- into the format of solarus 1.3.
--   local converter = require("converter_1_2_to_1_3")
--   converter.convert(quest_path)

local converter = {}

local function write_info(message)

  io.write(message, "\n")
  io.flush()
end

function converter.convert(quest_path)

  assert(quest_path)

  write_info("Updating your quest " .. quest_path .. " from Solarus 1.2 to Solarus 1.3.")
  write_info("It is recommended to backup your quest files before.")

  -- Convert the quest properties file quest.dat.
  write_info("  Converting the quest properties file...")
  local quest_properties_converter = require("1_2_to_1_3/quest_properties_converter_1_2")
  quest_properties_converter.convert(quest_path)

  -- Convert the resource list file project_db.dat.
  write_info("  Converting the list of resources...")
  local quest_db_converter = require("1_2_to_1_3/quest_db_converter_1_2")
  local resources = quest_db_converter.convert(quest_path)

  -- Convert maps.
  write_info("  Converting maps...")
  local map_converter = require("1_2_to_1_3/map_converter_1_2")
  for _, resource in pairs(resources["map"]) do
    write_info("    Map " .. resource.id .. " (" .. resource.description .. ")")
    map_converter.convert(quest_path, resource.id)
  end
  write_info("  All maps were converted.")

  write_info("Update successful!")

end

return converter

