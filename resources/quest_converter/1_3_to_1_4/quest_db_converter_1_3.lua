-- This module reads a quest resource list file with the format of Solarus 1.3
-- (project_db.dat) and adds fonts to it.
-- Changes:
-- - Fonts become a resource.
-- - Ensure that the required sprite "entities/crystal"
--   (this is better at least for the editor).

local converter = {}

local resource_type_names = {
  "map",
  "tileset",
  "music",
  "sprite",
  "sound",
  "item",
  "enemy",
  "language",
  "entity",
  "font",
}

local function load_quest_db(quest_path)

  local resources = {}

  -- Create a table for each type of resource.
  for _, resource_type_name in ipairs(resource_type_names) do
    local resource = {}
    resources[resource_type_name] = resource
    _G[resource_type_name] = function(resource_element)
      resource[#resource + 1] = resource_element
    end
  end

  dofile(quest_path .. "/data/project_db.dat")

  return resources
end

-- Returns whether the specified element is declared in project_db.dat.
local function is_resource_declared(resources, resource_type, element_id)

  for _, element in ipairs(resources[resource_type]) do
    if element.id == element_id then
      return true
    end
  end

  return false
end

-- Adds an element to project_db.dat if it is not already there.
local function ensure_resource_declared(output_file, resources, resource_type, element_id, element_description)

  if not is_resource_declared(resources, resource_type, element_id) then
    output_file:write(resource_type .. '{ id = "' .. element_id .. '", description = "' .. element_description .. '" }\n')
  end
end

function converter.convert(quest_path, fonts)

  -- Read project_db.dat.
  local resources = load_quest_db(quest_path)

  local output_file, error_message = io.open(quest_path .. "/data/project_db.dat", "a")
  if output_file == nil then
    error("Cannot open quest resource list file for writing: " .. error_message)
  end

  -- Make sure the crystal block sprite is declared.
  ensure_resource_declared(output_file, resources, "sprite", "entities/crystal_block", "Crystal block")
 
  -- Add fonts to the list.
  for _, id in ipairs(fonts) do
    ensure_resource_declared(output_file, resources, "font", id, id)
  end

  output_file:close()

  return resources
end

return converter

