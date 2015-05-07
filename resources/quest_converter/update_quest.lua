-- This script updates all data files of a solarus quest
-- into the most recent format.
-- Usage: lua update_quest.lua path/to/your_quest

local solarus_formats = {
  "1.0",
  "1.1",
  "1.2",
  "1.3",
  "1.4",
  "1.5",
}

local function write_info(message)

  message = message or ""
  io.write(message, "\n")
  io.flush()
end

local quest_path = ...
if quest_path == nil then
  error("Usage: lua update_quest.lua path/to/your_quest")
end

-- Determines and returns the format of a quest.
local function get_quest_format(quest_path)

  local quest_format = nil

  -- Read the quest properties file.
  function quest(properties)

    quest_format = properties.solarus_version
  end

  local quest_file = loadfile(quest_path .. "/data/quest.dat")
  if quest_file == nil then
    error("No quest was found in '" .. quest_path .. "'")
  end
  quest_file()

  if quest_format ~= nil then
    quest_format = quest_format:match("^([0-9]+%.[0-9]+)%.?")
  end
  return quest_format
end

-- Makes an upgrade step from a current format to the next one.
local function update_step(quest_path, old_format, new_format)

  old_format = old_format:gsub("%.", "_")
  new_format = new_format:gsub("%.", "_")
  local converter_name = "converter_" .. old_format .. "_to_" .. new_format
  local converter = require(converter_name)
  converter.convert(quest_path)
end

-- Main function.
local function update_quest(quest_path)
  local quest_format = get_quest_format(quest_path)

  if quest_format == nil then
    error("Failed to determine the format of this quest")
  end

  local solarus_format = solarus_formats[#solarus_formats]
  write_info("Format of the quest: " .. quest_format)
  write_info("Latest Solarus format: " .. solarus_format)

  if quest_format == solarus_format then
    -- Nothing to do.
    write_info("This quest is already up-to-date.")
  else
    -- Update to do.
    local started = false
    for _, format in ipairs(solarus_formats) do
      if started then
        write_info("====== Step " .. quest_format .. " to " .. format .. " ======")
        update_step(quest_path, quest_format, format)
        write_info("====== Step done ======")
        write_info()
        quest_format = format
      elseif format == quest_format then
        started = true
      end
    end
    write_info("All updates were successful!")
  end
end

local success, error_message = pcall(update_quest, quest_path)
if not success then
  write_info(error_message)
  error(error_message, 0)  -- Passing level 0 avoids an extra addition of file/line info.
end

