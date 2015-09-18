-- Solarus 1.4 to 1.5.
-- Changes in the map syntax:
-- - New mandatory property num_layers.

local converter = {}

function converter.convert(quest_path, map_id, default_font_id)

  local input_file_name = quest_path .. "/data/maps/" .. map_id .. ".dat"
  local input_file, error_message = io.open(input_file_name)
  if input_file == nil then
    error("Cannot open old map file for reading: " .. error_message)
  end

  local text = input_file:read("*a")  -- Read the whole file.

  -- Add the num_layers property with value 3.
  text = text:gsub(
      "\n  tileset = \"",
      "\n  num_layers = 3,\n  tileset = \""
  )

  input_file:close()

  local output_file_name = input_file_name
  local output_file, error_message = io.open(output_file_name, "w")
  if output_file == nil then
    error("Cannot open new map file for writing: " .. error_message)
  end

  output_file:write(text)
  output_file:close()
end

return converter

