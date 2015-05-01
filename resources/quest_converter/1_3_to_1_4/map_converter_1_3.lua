-- Solarus 1.3 to 1.4.
-- Changes in the map syntax:
-- - Shop treasures have a font field.

local converter = {}

function converter.convert(quest_path, map_id, default_font_id)

  if default_font_id == nil then
    -- Nothing to do: there is no default font value to take care of.
    return
  end

  local input_file_name = quest_path .. "/data/maps/" .. map_id .. ".dat"
  local input_file, error_message = io.open(input_file_name)
  if input_file == nil then
    error("Cannot open old map file for reading: " .. error_message)
  end

  local text = input_file:read("*a")  -- Read the whole file.

  -- Set the old default font to shop treasures.
  local shop_treasure_replacement = "shop_treasure{\n  font = \"" .. default_font_id .. "\",\n"
  text = text:gsub("shop_treasure{\n", shop_treasure_replacement)

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

