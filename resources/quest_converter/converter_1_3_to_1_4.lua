-- This script updates all data files of a solarus 1.3 quest
-- into the format of solarus 1.4.
-- Usage: lua update_quest.lua path/to/your_quest
--   local converter = require("converter_1_3_to_1_4")
--   converter.upgrade(quest_path)


local converter = {}

local function write_info(message)

  io.write(message, "\n")
  io.flush()
end

function converter.convert(quest_path)

  assert(quest_path)

  write_info("Updating your quest " .. quest_path .. " from Solarus 1.3 to Solarus 1.4.")
  write_info("It is recommended to backup your quest files before.")

  -- Convert the quest properties file quest.dat.
  write_info("  Converting the quest properties file...")
  local quest_properties_converter = require("1_3_to_1_4/quest_properties_converter_1_3")
  quest_properties_converter.convert(quest_path)

  -- Remove the font list file text/fonts.dat.
  write_info("  Removing the obsolete font list file...")
  local font_list_converter = require("1_3_to_1_4/font_list_converter_1_3")
  local fonts = font_list_converter.convert(quest_path)

  -- Convert the resource list file project_db.dat.
  write_info("  Converting the resource list file...")
  local quest_db_converter = require("1_3_to_1_4/quest_db_converter_1_3")
  quest_db_converter.convert(quest_path, fonts)

  write_info("Update successful!")

end

return converter

