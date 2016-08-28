-- This module reads a quest resource list file with the format of Solarus 1.5
-- (project_db.dat).

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

function converter.convert(quest_path, fonts)

  -- Read project_db.dat.
  local resources = load_quest_db(quest_path)

  -- No change, just return the info.
  return resources
end

return converter

