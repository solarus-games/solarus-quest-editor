-- This module updates all data files of a solarus 1.0 quest
-- into the format of solarus 1.1.
-- Usage:
--   local converter = require("converter_1_0_to_1_1")
--   converter.upgrade(quest_path)

local converter = {}

local function write_info(message)

  io.write(message, "\n")
  io.flush()
end

function converter.convert(quest_path)

  assert(quest_path)

  write_info("Updating your quest " .. quest_path .. " from Solarus 1.0 to Solarus 1.1.")
  write_info("It is recommended to backup your quest files before.")

  -- Remove the language list file languages/languages.dat.
  write_info("  Removing the obsolete language list file...")
  local language_list_converter = require("1_0_to_1_1/language_list_converter_1_0")
  local languages = language_list_converter.convert(quest_path)

  -- Convert the quest properties file quest.dat.
  write_info("  Converting the quest properties file...")
  local quest_properties_converter = require("1_0_to_1_1/quest_properties_converter_1_0")
  quest_properties_converter.convert(quest_path)

  -- Convert the resource list file project_db.dat.
  write_info("  Converting the resource list file...")
  local quest_db_converter = require("1_0_to_1_1/quest_db_converter_1_0")
  local resources = quest_db_converter.convert(quest_path, languages)

  -- Convert tilesets.
  write_info("  Converting tilesets...")
  local tileset_converter = require("1_0_to_1_1/tileset_converter_1_0")
  for _, resource in pairs(resources["tileset"]) do
    write_info("    Tileset " .. resource.id .. " (" .. resource.description .. ")")
    tileset_converter.convert(quest_path, resource.id)
  end
  write_info("  All tilesets were converted.")

  -- Convert sprites.
  write_info("  Converting sprites...")
  local sprite_converter = require("1_0_to_1_1/sprite_converter_1_0")
  for _, resource in pairs(resources["sprite"]) do
    write_info("    Sprite " .. resource.id .. " (" .. resource.description .. ")")
    sprite_converter.convert(quest_path, resource.id)
  end
  write_info("  All sprites were converted.")

  write_info("Update successful!")

end

return converter

