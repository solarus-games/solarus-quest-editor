/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus Quest Editor is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus Quest Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "entities/npc.h"
#include "map_model.h"

namespace SolarusEditor {

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
Npc::Npc(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::NPC) {

  set_origin(QPoint(8, 13));

  set_num_directions(4);
  set_no_direction_allowed(true);
  set_no_direction_text(MapModel::tr("Any"));

  SubtypeList subtypes = {
    { "0", MapModel::tr("Generalized NPC (something)") },
    { "1", MapModel::tr("Usual NPC (somebody)") }
  };
  set_existing_subtypes(subtypes);
  set_traversable(false);
}

}
